#ifndef CTR_LLVM_PASS_AST_H
#define CTR_LLVM_PASS_AST_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <sstream>
#include "../citron.h"
#include "ops.hpp"
#include "citron_types.hpp"

template < class T >
std::ostream& operator << (std::ostream& os, const std::vector<T>& v) 
{
    os << "[";
    for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii)
    {
        os << *ii << (ii!=v.end()-1?" ":"");
    }
    os << "]";
    return os;
}

std::ostream& operator << (std::ostream& os, const std::pair<llvm::Function*, std::vector<llvm::Value*>>& v) 
{
    os << "<" << v.first << ", " << v.second << ">";
    return os;
}
namespace Citron {

typedef llvm::Value* valProv_f(std::vector<llvm::Value*>, std::vector<Citron::TypeC>);
using valProv_ = std::function<valProv_f>;

llvm::Value* emptyllvmValueProvider_(std::vector<llvm::Value*> arg, std::vector<Citron::TypeC> argtypes) {
    return nullptr;
}
const static valProv_ emptyllvmValueProvider = &emptyllvmValueProvider_;
using mCache_t = std::pair<llvm::Function*, valProv_>;

namespace llvmCoreValues {
    static llvm::LLVMContext TheContext;
    static llvm::IRBuilder<> Builder(TheContext);
    static std::unique_ptr<llvm::Module> TheModule (new llvm::Module("citron", TheContext));
    static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM (new llvm::legacy::FunctionPassManager(TheModule.get()));
    static std::map<std::string, std::map<std::vector<Citron::TypeC>, std::pair<Citron::TypeC, mCache_t>>> MethodCache;
    static int id = 0;
    static std::ostringstream _temp_os;
    static std::string fresh() {
        _temp_os.str("");
        _temp_os << "_fresh_block$" << id++;
        return _temp_os.str();
    }
    static std::string errors;
    static void populateCache();
    static std::map<std::string, std::pair<Citron::TypeC, llvm::Value*>> NamedValues;
}

namespace llvmTypes {
    static llvm::Type* getLLVMTy(Citron::TypeC t);
    static llvm::Type* DoubleTy = llvm::Type::getDoubleTy(llvmCoreValues::TheContext);
    static llvm::Type* CharTy = llvm::Type::getInt8Ty(llvmCoreValues::TheContext);
    static llvm::Type* BoolTy = llvm::Type::getInt1Ty(llvmCoreValues::TheContext);
    static llvm::Type* VoidTy = llvm::Type::getVoidTy(llvmCoreValues::TheContext);
    static std::function<llvm::ArrayType*(uint64_t)> StringTy_gen = [](uint64_t length){ return llvm::ArrayType::get(llvmTypes::CharTy, length+1); };
    static llvm::Type* FnTy_gen(std::vector<llvm::Type*> vs) {
        auto v = vs.back();
        vs.pop_back();
        return llvm::FunctionType::get(v, vs, false);
    }
};

namespace llvmCoreValues {
    static llvm::Type* getLLVMTy(Citron::TypeC t) {
        switch(t) {
        case Citron::Type::NumberTy: return llvmTypes::DoubleTy;
        case Citron::Type::StringTy: return llvmTypes::StringTy_gen(t.data());
        case Citron::Type::BoolTy  : return llvmTypes::BoolTy;
        case Citron::Type::DummyTy : return llvmTypes::VoidTy;
        case Citron::Type::FunTy: 
            std::vector<llvm::Type*> ms;
            auto vs = Citron::Type::incomplete_types[t.data()-1];
            ms.resize(vs.size());
            std::transform(vs.begin(), vs.end(), ms.begin(), getLLVMTy);
            return llvmTypes::FnTy_gen(ms);
        }
    }
};

class ExprAST;

class vPtrC {
public:
    llvm::Value* v;
    bool callable;
    bool literal;
    vPtrC(llvm::Value* v) : v(v), callable(true), literal(true) {}
    vPtrC(llvm::Value* v, bool c) : v(v), callable(c), literal(true) {}
    vPtrC(llvm::Value* v, bool c, bool l) : v(v), callable(c), literal(l) {}
    bool is_call() const { return callable; }
    bool is_lit() const { return literal; }
    explicit operator llvm::Value* () const { return v; }
    bool operator! () { return !v; }
    explicit operator bool () { return !!v; }
};

using argT = std::shared_ptr<ExprAST>;
using argVec = std::vector<argT>;
using pipeline_t = std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, std::vector<Citron::TypeC>, int>>;
using codegen_t = std::pair<vPtrC, std::tuple<argT, pipeline_t, Citron::TypeC>>;



class ExprAST {
public:
    virtual ~ExprAST() = default;
    static std::shared_ptr<ExprAST> transform_expr(ctr_tnode* node);
    virtual std::ostream& print (std::ostream& os) const = 0;
    friend std::ostream& operator<< (std::ostream& os, const ExprAST& v) {
        return v.print(os);
    } 
    virtual codegen_t codegen(bool intern=false) = 0;
    virtual Citron::TypeC ty() const = 0;
};

static const std::shared_ptr<ExprAST> EOP = {};

class NilExprAST : public ExprAST {
public:
    NilExprAST() {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << "(void)";
    }
    virtual codegen_t codegen(bool intern=false) ;
    virtual Citron::TypeC ty() const { return Citron::Type::DummyTy; }
};

namespace Constants {
    static auto Nil = std::make_shared<NilExprAST>(NilExprAST());
}

class NumberExprAST : public ExprAST {
    double val;
public:
    NumberExprAST(double val) : val(val) {}
    NumberExprAST(char const* val) : val(atof(val)) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << val;
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::TypeC ty() const { return Citron::Type::NumberTy; }
};

class BoolExprAST : public ExprAST {
    bool val;
public:
    BoolExprAST(bool val) : val(val) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << (val ? "True" : "False");
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::TypeC ty() const { return Citron::Type::BoolTy; }
};

class StringExprAST : public ExprAST {
    std::string val;
public:
    StringExprAST(char* s, size_t l) : val{s, l} {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << '\'' << val << '\'';
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::TypeC ty() const { return Citron::Type::StringTy.transform(val.length()); }
};

class VariableExprAST : public ExprAST {
    std::string name_;
public:
    VariableExprAST(char* s, size_t l) : name_(s, l) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << "var " << name_;
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::TypeC ty() const {
        auto x = llvmCoreValues::NamedValues.find(name_);
        if (x == llvmCoreValues::NamedValues.end())
            return Citron::Type::DummyTy;
        else
            return (*x).second.first; //type
    }
    std::string name() { return name_; }
};

class AssignmentExprAST : public ExprAST {
    argT target, assignee;
public:
    AssignmentExprAST(argT v, argT assignee) : target(v), assignee(assignee) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << "let " << *target << " = " << *assignee << " in ";
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::TypeC ty() const {
        VariableExprAST* var = dynamic_cast<VariableExprAST*>(target.get());
        auto type = assignee->ty();
        auto varty = target->ty();
        if (varty == Citron::Type::DummyTy)
            llvmCoreValues::NamedValues[var->name()] = {type, nullptr};
        else if (varty != type)
            std::__throw_invalid_argument(("Invalid rebinding for variable " + var->name()).c_str());
        return type;
    }
};

class Message {
public:
    std::string name;
    argVec arguments;
    friend std::ostream& operator<< (std::ostream& os, const Message& m) {
        std::vector<std::string> strs;
        boost::split(strs, m.name, boost::is_any_of(":"));
        if (strs.size() == 1) {
            os << strs[0];
            if (m.arguments.size() == 1) 
                os << " " << *(m.arguments[0]);
            return os;
        }
        for (int i =0; i<strs.size()-1; i++) {
            os << strs[i] << " " << *(m.arguments[i]) << " ";
        }
        return os;
    }
};

class UnaMessage : public Message {
public:
    UnaMessage(std::string name) : Message({name, {}}) {}
    UnaMessage(char const * name, int length) : Message({std::string(name, length), {}}) {}
};

class BinMessage : public Message {
public:
    BinMessage(std::string name, argT&& argument) 
        : Message({name, argVec{}}) { arguments.push_back(std::move(argument)); }
    BinMessage(char const * name, int length, argT&& argument) 
        : Message({std::string(name, length), argVec{}}) { arguments.push_back(std::move(argument)); }
};

class KWMessage : public Message {
public:
    KWMessage(std::string name, argVec &arguments) : Message({name, std::move(arguments)}) {}
    KWMessage(char const * name, int length, argVec &arguments) : Message({std::string(name, length), std::move(arguments)}) {}
};

class MessagesExpr : public ExprAST {
    argT receiver;
    std::vector<Message> message_line;
public:
    MessagesExpr(argT&& rec, std::vector<Message> msgs) : receiver(std::move(rec)), message_line(msgs) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << "(" << message_line << " to " << *(receiver) << ")";
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::TypeC ty() const { 
        if(message_line.size() == 0) 
            return receiver->ty();
        Citron::TypeC last_type = receiver->ty();
        for(int i=0; i<message_line.size(); i++) {
            auto msg = message_line[i];
            auto&& protos = llvmCoreValues::MethodCache[msg.name];
            auto arg_v = std::vector<Citron::TypeC>{last_type};
            for (auto arg : msg.arguments)
                arg_v.push_back(arg->ty());
            bool foundty = false;
            for(auto it = protos.cbegin(), ite = protos.cend(); it != ite; it++) {
                auto proto_kv = *it;
                if (proto_kv.first.size() != arg_v.size()) continue;
                if (!std::equal(proto_kv.first.begin(), proto_kv.first.end(), arg_v.begin(), citron_type_equals)) continue;
                foundty = true;
                last_type = proto_kv.second.first.resolve(arg_v);
                break;
            }
            if(!foundty) {
                    std::vector<std::string> tys;
                    tys.resize(arg_v.size());
                    std::transform(arg_v.begin(), arg_v.end(), tys.begin(), ty_as_string);
                    std::cout << "no type information exists for " << msg.name << " (" << boost::algorithm::join(tys, " -> ") << " -> ?)\nThese alternatives exist:\n";
                    for (auto x : protos) {
                        tys.resize(x.first.size());
                        std::transform(x.first.begin(), x.first.end(), tys.begin(), ty_as_string);
                        std::cout << msg.name << " with type " << boost::algorithm::join(tys, " -> ") << "\n";
                    }
                    std::__throw_invalid_argument(("Unknown method call " + msg.name).c_str());
                }
        }
        return last_type;
    }
};

class SequenceExprAST : public ExprAST {
    std::vector<argT> sequence;
    bool main;
public:
    SequenceExprAST(ctr_tlistitem* list, bool main) : sequence(), main(main) {
        while (list) {
            auto t_expr = ExprAST::transform_expr(list->node);
            if (t_expr == Citron::EOP) 
                break;
            sequence.push_back(t_expr);
            list = list->next;
        }
    }
    virtual std::ostream& print (std::ostream& os) const {
        os << "[";
        std::for_each(sequence.begin(), sequence.end(), [&](argT v) { v->print(os); os << " : "; });
        os << "]";
        return os;
    }
    virtual codegen_t codegen(bool intern=false);
    llvm::Function* codegen(std::vector<Citron::TypeC> argtys);
    virtual Citron::TypeC ty() const { //force last type to be returned
        Citron::TypeC type;
        for (auto expr : sequence) //eagerly typecheck all
            type = expr->ty();
        return type;
    }
};

class BlockExprAST : public ExprAST {
    std::vector<std::string> formal_params;
    argT body;
    bool takes_self;
    std::map<std::vector<Citron::TypeC>, llvm::Function*> instances;
    std::vector<Citron::TypeC> tyvars;
public:
    BlockExprAST(ctr_tlistitem* args, argT b) : formal_params(), body(b), takes_self(false) {
        if(args && args->node && args->node->vlen == 4 && strncmp(args->node->value, "self", 4) == 0) {
            takes_self = true;
            args = args->next;
        }
        while (args) {
            if(!args->node) break;
            formal_params.push_back({args->node->value, args->node->vlen});
            tyvars.push_back(Citron::Type::tyvar({}));
            args = args->next;
        }
        auto rty = Citron::Type::tyvar(tyvars);
        tyvars.push_back(Citron::Type::FunTy.transform(rty));
    }
    virtual std::ostream& print (std::ostream& os) const {
        os << '{';
        std::for_each(formal_params.begin(), formal_params.end(), [&](std::string s) { os << ':' << s; });
        return os << ' ' << *body << " }";
    }
    virtual codegen_t codegen(bool intern=false);
    llvm::Function* codegen(std::vector<llvm::Value*>, std::vector<Citron::TypeC>);

    virtual Citron::TypeC ty() const {
        return tyvars.back();
    }
    llvm::Function* generate_now(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> argtys) {
        auto& instance = instances[argtys];
        if(instance) return instance;
        instance = codegen(args, argtys);
        return instance;
    }
};

static std::vector<Message> vectorize_names(ctr_tlistitem* tl);

struct nLLVMBasicNumericOp {
    static auto ffadd (std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFAdd(args[0], args[1], "faddtmp");
    }
    static auto ffsub(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFSub(args[0], args[1], "fsubtmp");
    }
    static auto ffmul(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFMul(args[0], args[1], "fmultmp");
    }
    static auto ffdiv(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFDiv(args[0], args[1], "fdivtmp");
    }
    static auto ffcmpoeq(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOEQ(args[0], args[1], "cmpoeqtmp");
    }
    static auto ffcmpogt(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOGT(args[0], args[1], "cmpogttmp");
    }
    static auto ffcmpolt(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOLT(args[0], args[1], "cmpolttmp");
    }
    static auto ffcmpole(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOLE(args[0], args[1]);
    }
    static auto ffcmpoge(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOGE(args[0], args[1]);
    }
    static auto fnot(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateNot(args[0]);
    }
    static auto beq(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        return Citron::llvmCoreValues::Builder.CreateICmpEQ(args[0], args[1]);
    }
    static auto nbeq(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        auto v = Citron::llvmCoreValues::Builder.CreateUIToFP(args[1], Citron::llvmTypes::DoubleTy);
        return Citron::llvmCoreValues::Builder.CreateFCmpOEQ(v, args[0]);
    }
    static auto bneq(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        auto v = Citron::llvmCoreValues::Builder.CreateUIToFP(args[0], Citron::llvmTypes::DoubleTy);
        return Citron::llvmCoreValues::Builder.CreateFCmpOEQ(v, args[1]);
    }
    static llvm::Value* ssadd(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        llvm::Value *s0 = args[0], *s1 = args[1];
        uint64_t e0 = arg_ts[0].data(), e1 = arg_ts[1].data();
        auto t0 = Citron::llvmCoreValues::getLLVMTy(arg_ts[0]);
        auto t1 = Citron::llvmCoreValues::getLLVMTy(arg_ts[1]);
        llvm::Type* t2 = static_cast<llvm::Type*>(llvm::ArrayType::get(Citron::llvmTypes::CharTy, e0+e1+1));
        auto memp = Citron::llvmCoreValues::Builder.CreateAlloca(t2, 0, "alloca_sdest");
        //TODO: Handle strcat
        return Citron::llvmCoreValues::Builder.CreateLoad(memp);
    }
    static auto bcall3(std::vector<llvm::Value*> args, std::vector<Citron::TypeC> arg_ts) {
        Citron::BlockExprAST* block = reinterpret_cast<Citron::BlockExprAST*>(args[0]);
        args.erase(args.begin());
        arg_ts.erase(arg_ts.begin());
        auto val = block->generate_now(args, arg_ts);
        return Citron::llvmCoreValues::Builder.CreateCall(val, args);
    }
    valProv_ operator() (char op) {
        switch(op) {
            case '+':
                return &ffadd;
            case '-':
                return &ffsub;
            case '*':
                return &ffmul;
            case '/':
                return &ffdiv;
            case '=':
                return &ffcmpoeq;
            case '>':
                return &ffcmpogt;
            case '<':
                return &ffcmpolt;
        }
    }
    valProv_ operator() (std::string name) {
            if (name == "<=:")
                return &ffcmpole;
            if (name == ">=:")
                return &ffcmpoge;
            if (name == "not")
                return &fnot;
            if (name == "bb=")
                return &beq;
            if (name == "nb=")
                return &nbeq;
            if (name == "bn=")
                return &bneq;
            if (name == "s+")
                return &ssadd;
            if (name == "apply3")
                return &bcall3;
            std::__throw_invalid_argument(("Invalid method " + name).c_str());
        }
};
}


#endif