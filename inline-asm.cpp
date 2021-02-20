#include "native-asm.h"
#include <cstddef>

using namespace llvm;

static LLVMContext _ctx {};


class NativeLlvmTarget {
public:
    static std::shared_ptr<NativeLlvmTarget> instance;
    static std::shared_ptr<NativeLlvmTarget> Create() {
        if (instance == nullptr) return instance = std::make_shared<NativeLlvmTarget>();
        return instance;
    }
    NativeLlvmTarget() {
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
    }
};

std::shared_ptr<NativeLlvmTarget> NativeLlvmTarget::instance {nullptr};

namespace llvm {
namespace orc {

class KaleidoscopeJIT {
private:
  std::string errorStr;
  ExecutionSession ES;
  std::shared_ptr<NativeLlvmTarget> nativeTarget;
  std::shared_ptr<SymbolResolver> Resolver;
  std::unique_ptr<TargetMachine> TM;
  const DataLayout DL;
#if LLVM_VERSION_MAJOR >= 8
  LegacyRTDyldObjectLinkingLayer ObjectLayer;
  LegacyIRCompileLayer<decltype(ObjectLayer), SimpleCompiler> CompileLayer;
#else
  RTDyldObjectLinkingLayer ObjectLayer;
  IRCompileLayer<decltype(ObjectLayer), SimpleCompiler> CompileLayer;
#endif
public:
  KaleidoscopeJIT()
      : nativeTarget(NativeLlvmTarget::Create()),
        Resolver(createLegacyLookupResolver(
            ES,
            [this](const llvm::StringRef &Name) -> JITSymbol {
              if (auto Sym = CompileLayer.findSymbol(Name.str(), false))
                return Sym;
              else if (auto Err = Sym.takeError())
                return std::move(Err);
              if (auto SymAddr =
                      RTDyldMemoryManager::getSymbolAddressInProcess(Name.str()))
                return JITSymbol(SymAddr, JITSymbolFlags::Exported);
              return nullptr;
            },
            [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); })),
        TM(EngineBuilder().setErrorStr(&errorStr).selectTarget()?:(puts(errorStr.c_str()), nullptr)), DL(TM->createDataLayout()),
        ObjectLayer(ES,
                    [this](VModuleKey) {
#if LLVM_VERSION_MAJOR >= 8
                      return LegacyRTDyldObjectLinkingLayer::Resources{
#else
                      return RTDyldObjectLinkingLayer::Resources{
#endif
                          std::make_shared<SectionMemoryManager>(), Resolver};
                    }),
        CompileLayer(ObjectLayer, SimpleCompiler(*TM)) {
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  }

  TargetMachine &getTargetMachine() { return *TM; }

  VModuleKey addModule(std::unique_ptr<Module> M) {
    // Add the module to the JIT with a new VModuleKey.
    auto K = ES.allocateVModule();
    cantFail(CompileLayer.addModule(K, std::move(M)));
    return K;
  }

  JITSymbol findSymbol(const std::string Name) {
    std::string MangledName;
    raw_string_ostream MangledNameStream(MangledName);
    Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
    return CompileLayer.findSymbol(MangledNameStream.str(), true);
  }

  JITTargetAddress getSymbolAddress(const std::string Name) {
    return cantFail(findSymbol(Name).getAddress());
  }

  void removeModule(VModuleKey K) {
    cantFail(CompileLayer.removeModule(K));
  }
};

} // end namespace orc
} // end namespace llvm

void llvmDebugPrint(BasicBlock* value) {
    errs() << *value;
}
void llvmDebugPrint(Value* value) {
    errs() << *value;
}
void llvmDebugPrint(Type* value) {
    errs() << *value;
}
void llvmDebugPrint(Module* value) {
    errs() << *value;
}

    using BlockT = BasicBlock*;

    class GenContext {
    public:
        std::stack<BlockT*> blocks;
        Module *module;
        Function* mFunction;
        Function* tempFunction;
    public:
        GenContext() {
            module = new Module("citron-inline", _ctx);
        }
        void *run(std::string,std::string,InlineAsm::AsmDialect,int,int,asm_arg_info_t*);
        BlockT context() { return *blocks.top(); }
        Function* function() const { return tempFunction; }
        void currentFunction(Function* f) { tempFunction = f; }
        void pop() {
            auto* top = blocks.top();
            blocks.pop();
            delete top;
        }
    };

    Value* generateInlineAsm(std::string asmdec, std::string constraint, InlineAsm::AsmDialect dialect, FunctionType* AsmFTy) {
        auto* value = InlineAsm::get(AsmFTy, asmdec, StringRef(constraint), true, false, dialect);
        return value;
    }

    static int cid = 0;
    struct __avstruct { void* a; void* b; };
    constexpr auto addr_a = offsetof(__avstruct, a);
    constexpr auto addr_b = offsetof(__avstruct, b);
    std::string generateEntryFunction(std::string asmdec, std::string constr, InlineAsm::AsmDialect dialect, int off, int cnt, asm_arg_info_t* arginfo, GenContext& context) {
        std::vector<llvm::Type *> FnArgTypes {
            PointerType::get(Type::getInt8Ty(_ctx), 0),
            PointerType::get(Type::getInt8Ty(_ctx), 0)
        };
        FunctionType *FTy =
            FunctionType::get(PointerType::get(Type::getInt8Ty(_ctx), 0), FnArgTypes, false);
        std::string name = "__entry_inline_" + std::to_string(cid++);
        Function* func = Function::Create(
          FTy,
          GlobalValue::ExternalLinkage,
          (name).c_str(),
          context.module
        );
        func->setCallingConv(CallingConv::C);
        BasicBlock *blk = BasicBlock::Create(_ctx, "", func, 0);
        std::vector<Value*> callargs{};
        std::vector<llvm::Type *> AsmArgTypes;
        if (cnt>0) {
            Value* args = func->arg_end()-1;
            Type* dblty = Type::getDoubleTy(_ctx);
            Type* i8ty = Type::getInt8Ty(_ctx);
            Type* i64ty = Type::getInt64Ty(_ctx);
            Type* i8pty = PointerType::get(i8ty, 0);
            Type* i8ppty= PointerType::get(i8pty, 0);
            Type* dblpty = PointerType::get(dblty, 0);
            for (int i=0; i<cnt; i++) {
                // Value* mem = new AllocaInst(dblty, 0, "", blk);
                // mem = new StoreInst();
                auto *arg = args;
                if (addr_a>0)
                    arg = GetElementPtrInst::CreateInBounds(arg, ConstantInt::get(i8ty, addr_a, false), "", blk);
                arg = new BitCastInst(arg, i8ppty, "", blk);
                arg = new LoadInst(i8ppty, arg, "", blk);
                arg = GetElementPtrInst::CreateInBounds(arg, ConstantInt::get(i8ty, off, false), "", blk);
                arg = new BitCastInst(arg, dblpty, "", blk);
                arg = new LoadInst(dblpty, arg, "", blk);
                auto dty = arginfo[i].ty;
                auto argty = dblty;
                if (dty == ASM_ARG_TY_INT)
                    arg = new FPToSIInst(arg, (argty=i64ty), "", blk);
                else if (dty == ASM_ARG_TY_STR || dty == ASM_ARG_TY_PTR) {
                    arg = new IntToPtrInst(new FPToSIInst(arg, i64ty, "", blk), i8pty, "", blk);
                    argty = i8pty;
                }
                if (i!=cnt-1) {
                    args = GetElementPtrInst::CreateInBounds(args, ConstantInt::get(i8ty, addr_b, false), "", blk);
                    args = new BitCastInst(args, i8ppty, "", blk);
                    args = new LoadInst(i8ppty, args, "", blk);
                }
                // first
                callargs.push_back(arg);
                AsmArgTypes.push_back(argty);
            }
        }
        FunctionType *AsmFTy = FunctionType::get(Type::getInt64Ty(_ctx), AsmArgTypes, false);
        auto* x = generateInlineAsm(asmdec, constr, dialect, AsmFTy);
        Value* retval = CallInst::Create(AsmFTy, x, callargs, "", blk);
        retval = new SIToFPInst(retval, Type::getDoubleTy(_ctx), "", blk);
        retval = CallInst::Create(context.module->getFunction("ctr_build_number_from_float"), std::vector<Value*>{retval}, "", blk);
        ReturnInst::Create(_ctx, retval, blk);
        // llvmDebugPrint(func);
        if(verifyFunction(*func, &llvm::errs())) {
            puts("What the actual fuck? [Invalid program is as follows]");
            llvmDebugPrint(func);
            return "__NEVER_EXIST__";
        }
        return name;
    }

    static orc::KaleidoscopeJIT jit{};

    void* GenContext::run(std::string dec, std::string ct, InlineAsm::AsmDialect dialect, int offset, int count, asm_arg_info_t* arginfo) {
        std::vector<Type*> srtsArgs {
            Type::getDoubleTy(_ctx)
        };
        FunctionType* malloc_ty = FunctionType::get(Type::getInt8PtrTy(_ctx), srtsArgs, false);
        Function *ctr_build_number_from_float_f = Function::Create(malloc_ty, Function::ExternalLinkage, Twine("ctr_build_number_from_float"), module);
        ctr_build_number_from_float_f->setCallingConv(CallingConv::C);
        auto name = generateEntryFunction(dec, ct, dialect, offset, count, arginfo, *this);
        // llvmDebugPrint(module);
        if (llvm::verifyModule(*module))
            return nullptr;
        jit.addModule(std::unique_ptr<Module>(module));
        return (void*)jit.getSymbolAddress(name);
    }

    extern "C" void *ctr_cparse_intern_asm_block(char* asm_begin, char* asm_end, char* constraints, int offset, int argc, asm_arg_info_t* arginfo, int att) {
        int len = asm_end-asm_begin;
        char* s = (char*)malloc(len+1);
        memcpy(s, asm_begin, len);
        s[len] = 0;
        GenContext context;
        void* fn = NULL; /* get pointer */
        if ((fn = context.run(s, constraints, att?InlineAsm::AsmDialect::AD_ATT:InlineAsm::AsmDialect::AD_Intel, offset, argc+1, arginfo)) == nullptr) {
            return NULL;
        }
        free(s);
        /* relocate */
        // printf("Generated function at %p\n", fn);
        return fn;
    }
