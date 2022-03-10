#include "native-asm.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/Types.h>

static int cid = 0;
static LLVMModuleRef module = NULL;
static LLVMOrcThreadSafeContextRef tscontext = NULL;
static LLVMContextRef context = NULL;

struct __avstruct {
    void* a;
    void* b;
};
off_t const addr_a = offsetof(struct __avstruct, a);
off_t const addr_b = offsetof(struct __avstruct, b);
static char const* generate_entry_function(char* asm_dec, char* constraints, LLVMInlineAsmDialect dialect, ptrdiff_t offset, int count, asm_arg_info_t* arg_info)
{
    LLVMTypeRef fn_arg_types[] = {
        LLVMPointerType(LLVMInt8TypeInContext(context), 0),
        LLVMPointerType(LLVMInt8TypeInContext(context), 0),
    };
    LLVMTypeRef function_type = LLVMFunctionType(LLVMPointerType(LLVMInt8TypeInContext(context), 0), fn_arg_types, 2, 0);
    char buf[1024];
    sprintf(buf, "__entry_inline_%d", cid++);
    LLVMValueRef func = LLVMAddFunction(module, buf, function_type);
    LLVMSetFunctionCallConv(func, LLVMCCallConv);
    LLVMBasicBlockRef blk = LLVMCreateBasicBlockInContext(context, "");
    LLVMAppendExistingBasicBlock(func, blk);
    LLVMValueRef* call_args = (LLVMValueRef*)malloc(sizeof(LLVMValueRef) * count);
    LLVMTypeRef* asm_types = (LLVMTypeRef*)malloc(sizeof(LLVMTypeRef) * count);
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
    LLVMPositionBuilderAtEnd(builder, blk);
    LLVMTypeRef dblty = LLVMDoubleTypeInContext(context);
    LLVMTypeRef i8ty = LLVMInt8TypeInContext(context);
    LLVMTypeRef i64ty = LLVMInt64TypeInContext(context);
    LLVMTypeRef i8pty = LLVMPointerType(i8ty, 0);
    LLVMTypeRef i8ppty = LLVMPointerType(i8pty, 0);
    LLVMTypeRef dblpty = LLVMPointerType(dblty, 0);
    if (count > 0) {
        LLVMValueRef args = LLVMGetParam(func, 1);
        for (int i = 0; i < count; ++i) {
            LLVMValueRef arg = args;
            if (addr_a > 0) {
                LLVMValueRef offset = LLVMConstInt(i8ty, addr_a, 0);
                arg = LLVMBuildGEP(builder, arg, &offset, 1, "");
            }
            arg = LLVMBuildBitCast(builder, arg, i8ppty, "");
            arg = LLVMBuildLoad(builder, arg, "");
            LLVMValueRef offset_v = LLVMConstInt(i8ty, offset, 0);
            arg = LLVMBuildGEP(builder, arg, &offset_v, 1, "");
            arg = LLVMBuildBitCast(builder, arg, dblpty, "");
            arg = LLVMBuildLoad(builder, arg, "");
            enum AsmArgType ty = arg_info[i].ty;
            LLVMTypeRef argty = dblty;
            if (ty == ASM_ARG_TY_INT)
                arg = LLVMBuildFPToSI(builder, arg, (argty = i64ty), "");
            else if (ty == ASM_ARG_TY_PTR || ty == ASM_ARG_TY_STR) {
                arg = LLVMBuildFPToSI(builder, arg, i64ty, "");
                arg = LLVMBuildIntToPtr(builder, arg, i8pty, "");
                argty = i8pty;
            }
            if (i != count - 1) {
                LLVMValueRef index = LLVMConstInt(i8ty, addr_b, 0);
                args = LLVMBuildGEP(builder, args, &index, 1, "");
                args = LLVMBuildBitCast(builder, args, i8ppty, "");
                args = LLVMBuildLoad(builder, args, "");
            }
            // first
            call_args[i] = arg;
            asm_types[i] = argty;
        }
    }
    LLVMTypeRef asm_func_type = LLVMFunctionType(i64ty, asm_types, count, 0);
    LLVMValueRef x = LLVMGetInlineAsm(asm_func_type, asm_dec, strlen(asm_dec), constraints, strlen(constraints), 1, 0, dialect, 0);
    LLVMValueRef retval = LLVMBuildCall(builder, x, call_args, count, "");
    retval = LLVMBuildSIToFP(builder, retval, dblty, "");
    LLVMValueRef fn = LLVMGetNamedFunction(module, "ctr_build_number_from_float");
    assert(LLVMTypeOf(LLVMGetParam(fn, 0)) == dblty);
    retval = LLVMBuildCall(builder, fn, &retval, 1, "");
    LLVMBuildRet(builder, retval);
    LLVMDisposeBuilder(builder);
    // LLVMDumpModule(module);
    LLVMVerifyModule(module, LLVMAbortProcessAction, NULL);
    return strdup(buf);
}

static LLVMOrcLLJITRef jit = NULL;
static LLVMOrcJITDylibRef dylib = NULL;
void* ctr_cparse_intern_asm_block(char const* asm_begin, char const* asm_end, char const* constraints, int offset, int argc, asm_arg_info_t* arginfo, int att)
{
    LLVMInitializeNativeAsmParser();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeTarget();

    LLVMErrorRef error = NULL;

    if (!tscontext) {
        tscontext = LLVMOrcCreateNewThreadSafeContext();
        context = LLVMOrcThreadSafeContextGetContext(tscontext);
        error = LLVMOrcCreateLLJIT(&jit, NULL);
        if (error) {
            fprintf(stderr, "Failed to create JIT: %s\n", LLVMGetErrorMessage(error));
            return NULL;
        }
        LLVMOrcDefinitionGeneratorRef generator;
        error = LLVMOrcCreateDynamicLibrarySearchGeneratorForProcess(&generator, 0, NULL, NULL);
        if (error) {
            fprintf(stderr, "Failed to create JIT lookup fn: %s\n", LLVMGetErrorMessage(error));
            return NULL;
        }

        dylib = LLVMOrcLLJITGetMainJITDylib(jit);
        LLVMOrcJITDylibAddGenerator(dylib, generator);
    }

    module = LLVMModuleCreateWithNameInContext("citron-inline", context);
    LLVMTypeRef dbl = LLVMDoubleTypeInContext(context);
    LLVMTypeRef fnty = LLVMFunctionType(LLVMPointerType(LLVMInt8TypeInContext(context), 0), &dbl, 1, 0);
    LLVMValueRef ff = LLVMAddFunction(module, "ctr_build_number_from_float", fnty);
    LLVMSetFunctionCallConv(ff, LLVMCCallConv);
    LLVMSetLinkage(ff, LLVMExternalLinkage);

    ptrdiff_t len = asm_end - asm_begin;
    char* s = (char*)malloc(len + 1);
    memcpy(s, asm_begin, len);
    s[len] = 0;

    char const* fn = generate_entry_function(s, (char*)constraints, att ? LLVMInlineAsmDialectATT : LLVMInlineAsmDialectIntel, offset, argc + 1, arginfo);
    free(s);

    error = LLVMOrcLLJITAddLLVMIRModule(jit, dylib, LLVMOrcCreateNewThreadSafeModule(module, tscontext));
    if (error) {
        fprintf(stderr, "Failed to compile asm: %s\n", LLVMGetErrorMessage(error));
        return NULL;
    }

    LLVMOrcExecutorAddress address;
    error = LLVMOrcLLJITLookup(jit, &address, fn);
    if (error) {
        fprintf(stderr, "Failed to insert asm: %s\n", LLVMGetErrorMessage(error));
        return NULL;
    }

    module = NULL;

    return (void*)(intptr_t)address;
}
