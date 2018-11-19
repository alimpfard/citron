#include "native-asm.h"

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
  RTDyldObjectLinkingLayer ObjectLayer;
  IRCompileLayer<decltype(ObjectLayer), SimpleCompiler> CompileLayer;

public:
  KaleidoscopeJIT()
      : nativeTarget(NativeLlvmTarget::Create()),
        Resolver(createLegacyLookupResolver(
            ES,
            [this](const std::string &Name) -> JITSymbol {
              if (auto Sym = CompileLayer.findSymbol(Name, false))
                return Sym;
              else if (auto Err = Sym.takeError())
                return std::move(Err);
              if (auto SymAddr =
                      RTDyldMemoryManager::getSymbolAddressInProcess(Name))
                return JITSymbol(SymAddr, JITSymbolFlags::Exported);
              return nullptr;
            },
            [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); })),
        TM(EngineBuilder().setErrorStr(&errorStr).selectTarget()?:(puts(errorStr.c_str()), nullptr)), DL(TM->createDataLayout()),
        ObjectLayer(ES,
                    [this](VModuleKey) {
                      return RTDyldObjectLinkingLayer::Resources{
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
        void *run(std::string,std::string,InlineAsm::AsmDialect);
        BlockT context() { return *blocks.top(); }
        Function* function() const { return tempFunction; }
        void currentFunction(Function* f) { tempFunction = f; }
        void pop() {
            auto* top = blocks.top();
            blocks.pop();
            delete top;
        }
    };

    Value* generateInlineAsm(std::string asmdec, std::string constraint, InlineAsm::AsmDialect dialect) {
        std::vector<llvm::Type *> AsmArgTypes;
        FunctionType *AsmFTy =
        FunctionType::get(Type::getInt64Ty(_ctx), AsmArgTypes, false);
        auto* value = InlineAsm::get(AsmFTy, asmdec, StringRef(constraint), true, false, dialect);
        return value;
    }

    static int cid = 0;
    std::string generateEntryFunction(std::string asmdec, std::string constr, InlineAsm::AsmDialect dialect, GenContext& context) {
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
        Value* retval = CallInst::Create(generateInlineAsm(asmdec, constr, dialect), std::vector<Value*>{}, "", blk);
        retval = new SIToFPInst(retval, Type::getDoubleTy(_ctx), "", blk);
        retval = CallInst::Create(context.module->getFunction("ctr_build_number_from_float"), std::vector<Value*>{retval}, "", blk);
        ReturnInst::Create(_ctx, retval, blk);
        // if(verifyFunction(*func, &llvm::errs())) {
            // puts("What the actual fuck?");
        // }
        return name;
    }

    static orc::KaleidoscopeJIT jit{};

    static void llvmDebugPrint(BasicBlock* value) {
        errs() << *value;
    }
    static void llvmDebugPrint(Value* value) {
        errs() << *value;
    }
    static void llvmDebugPrint(Type* value) {
        errs() << *value;
    }
    static void llvmDebugPrint(Module* value) {
        errs() << *value;
    }

    void* GenContext::run(std::string dec, std::string ct, InlineAsm::AsmDialect dialect) {
        std::vector<Type*> srtsArgs {
            Type::getDoubleTy(_ctx)
        };
        FunctionType* malloc_ty = FunctionType::get(Type::getInt8PtrTy(_ctx), srtsArgs, false);
        Function *ctr_build_number_from_float_f = Function::Create(malloc_ty, Function::ExternalLinkage, Twine("ctr_build_number_from_float"), module);
        ctr_build_number_from_float_f->setCallingConv(CallingConv::C);
        auto name = generateEntryFunction(dec, ct, dialect, *this);
        // llvmDebugPrint(module);
        // llvm::verifyModule(*module);
        jit.addModule(std::unique_ptr<Module>(module));
        return (void*)jit.getSymbolAddress(name);
    }

    extern "C" void *ctr_cparse_intern_asm_block(char* asm_begin, char* asm_end, char* constraints, int att) {
        int len = asm_end-asm_begin;
        char* s = (char*)malloc(len+1);
        memcpy(s, asm_begin, len);
        s[len] = 0;
        GenContext context;
        void* fn = NULL; /* get pointer */
        if ((fn = context.run(s, constraints, att?InlineAsm::AsmDialect::AD_ATT:InlineAsm::AsmDialect::AD_Intel)) == nullptr) {
            return NULL;
        }
        free(s);
        /* relocate */
        // printf("Generated function at %p\n", fn);
        return fn;
    }
