#ifndef CTR_LLVM_PASS_AST_H
#define CTR_LLVM_PASS_AST_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
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

namespace llvmCoreValues {
    static llvm::LLVMContext TheContext;
    static llvm::IRBuilder<> Builder(TheContext);
    static std::unique_ptr<llvm::Module> TheModule (new llvm::Module("citron", TheContext));
    static std::map<std::string, std::map<std::vector<Citron::Type>, std::pair<Citron::Type, mCache_t>>> MethodCache;
    static int id = 0;
    static std::ostringstream _temp_os;
    static std::string fresh() {
        _temp_os.str("");
        _temp_os << "_fresh$#" << id++;
        return _temp_os.str();
    }
    static std::string errors;
    static void populateCache();
    static std::map<std::string, std::pair<Citron::Type, llvm::Value*>> NamedValues;
}

namespace llvmTypes {
    static llvm::Type* DoubleTy = llvm::Type::getDoubleTy(llvmCoreValues::TheContext);
    static llvm::Type* CharTy = llvm::Type::getInt8Ty(llvmCoreValues::TheContext);
    static llvm::Type* BoolTy = llvm::Type::getInt1Ty(llvmCoreValues::TheContext);
    static llvm::Type* VoidTy = llvm::Type::getVoidTy(llvmCoreValues::TheContext);
    static std::function<llvm::ArrayType*(uint64_t)> StringTy_gen = [](uint64_t length){ return llvm::ArrayType::get(llvmTypes::CharTy, length); };
};

namespace llvmCoreValues {
    static llvm::Type* getLLVMTy(Citron::Type t, std::variant<bool, uint64_t, std::monostate> data = {}) {
        switch(t) {
        case Citron::Type::NumberTy: return llvmTypes::DoubleTy;
        case Citron::Type::StringTy: return llvmTypes::StringTy_gen(std::get<uint64_t>(data));
        case Citron::Type::BoolTy  : return llvmTypes::BoolTy;
        case Citron::Type::DummyTy : return llvmTypes::VoidTy;
        }
    }
};

class ExprAST;

using argT = std::shared_ptr<ExprAST>;
using argVec = std::vector<argT>;
using pipeline_t = std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>;
using codegen_t = std::pair<llvm::Value*, std::tuple<argT, pipeline_t, Citron::Type>>;

class ExprAST {
public:
    virtual ~ExprAST() = default;
    static std::shared_ptr<ExprAST> transform_expr(ctr_tnode* node);
    virtual std::ostream& print (std::ostream& os) const = 0;
    friend std::ostream& operator<< (std::ostream& os, const ExprAST& v) {
        return v.print(os);
    } 
    virtual codegen_t codegen(bool intern=false) = 0;
    virtual Citron::Type ty() const = 0;
};

class NumberExprAST : public ExprAST {
    double val;
public:
    NumberExprAST(double val) : val(val) {}
    NumberExprAST(char const* val) : val(atof(val)) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << val;
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::Type ty() const { return Citron::Type::NumberTy; }
};

class BoolExprAST : public ExprAST {
    bool val;
public:
    BoolExprAST(bool val) : val(val) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << (val ? "True" : "False");
    }
    virtual codegen_t codegen(bool intern=false);
    virtual Citron::Type ty() const { return Citron::Type::BoolTy; }
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
    virtual Citron::Type ty() const { 
        if(message_line.size() == 0) 
            return receiver->ty();
        Citron::Type last_type = receiver->ty();
        for(int i=0; i<message_line.size(); i++) {
            auto msg = message_line[i];
            auto&& protos = llvmCoreValues::MethodCache[msg.name];
            auto arg_v = std::vector<Citron::Type>{last_type};
            for (auto arg : msg.arguments)
                arg_v.push_back(arg->ty());
            bool foundty = false;
            for(auto it = protos.cbegin(), ite = protos.cend(); it != ite; it++) {
                auto proto_kv = *it;
                if (proto_kv.first.size() != arg_v.size()) continue;
                if (!std::equal(proto_kv.first.begin(), proto_kv.first.end(), arg_v.begin(), citron_type_equals)) continue;
                foundty = true;
                last_type = proto_kv.second.first;
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

static std::vector<Message> vectorize_names(ctr_tlistitem* tl);

struct nLLVMBasicNumericOp {
    static auto ffadd (std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFAdd(args[0], args[1], "faddtmp");
    }
    static auto ffsub(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFSub(args[0], args[1], "fsubtmp");
    }
    static auto ffmul(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFMul(args[0], args[1], "fmultmp");
    }
    static auto ffdiv(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFDiv(args[0], args[1], "fdivtmp");
    }
    static auto ffcmpoeq(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOEQ(args[0], args[1], "cmpoeqtmp");
    }
    static auto ffcmpogt(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOGT(args[0], args[1], "cmpogttmp");
    }
    static auto ffcmpolt(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOLT(args[0], args[1], "cmpolttmp");
    }
    static auto ffcmpole(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOLE(args[0], args[1]);
    }
    static auto ffcmpoge(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateFCmpOGE(args[0], args[1]);
    }
    static auto fnot(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateNot(args[0]);
    }
    static auto beq(std::vector<llvm::Value*> args) {
        return Citron::llvmCoreValues::Builder.CreateICmpEQ(args[0], args[1]);
    }
    static auto nbeq(std::vector<llvm::Value*> args) {
        auto v = Citron::llvmCoreValues::Builder.CreateUIToFP(args[1], Citron::llvmTypes::DoubleTy);
        return Citron::llvmCoreValues::Builder.CreateFCmpOEQ(v, args[0]);
    }
    static auto bneq(std::vector<llvm::Value*> args) {
        auto v = Citron::llvmCoreValues::Builder.CreateUIToFP(args[0], Citron::llvmTypes::DoubleTy);
        return Citron::llvmCoreValues::Builder.CreateFCmpOEQ(v, args[1]);
    }
    valProv_* operator() (char op) {
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
    valProv_* operator() (std::string name) {
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
            std::__throw_invalid_argument(("Invalid method " + name).c_str());
        }
};
}


#endif