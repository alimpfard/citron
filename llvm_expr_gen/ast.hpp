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
#include <sstream>
#include "../citron.h"
#include "ops.hpp"

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

enum class Type {
    DummyTy,
    NumberTy,
    StringTy,
    BoolTy
};

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
}

namespace llvmTypes {
    static llvm::Type* DoubleTy = llvm::Type::getDoubleTy(llvmCoreValues::TheContext);
    static llvm::Type* CharTy = llvm::Type::getInt8Ty(llvmCoreValues::TheContext);
    static llvm::Type* BoolTy = llvm::Type::getInt1Ty(llvmCoreValues::TheContext);
    static std::function<llvm::ArrayType*(uint64_t)> StringTy_gen = [](uint64_t length){ return llvm::ArrayType::get(llvmTypes::CharTy, length); };
};

namespace llvmCoreValues {
    static llvm::Type* getLLVMTy(Citron::Type t, std::variant<bool, uint64_t, nullptr_t> data = nullptr) {
        switch(t) {
        case Citron::Type::NumberTy: return llvmTypes::DoubleTy;
        case Citron::Type::StringTy: return llvmTypes::StringTy_gen(std::get<uint64_t>(data));
        case Citron::Type::BoolTy  : return llvmTypes::BoolTy;
        }
    }
};

class ExprAST;

using argT = std::shared_ptr<ExprAST>;
using argVec = std::vector<argT>;

class ExprAST {
public:
    virtual ~ExprAST() = default;
    static std::shared_ptr<ExprAST> transform_expr(ctr_tnode* node);
    virtual std::ostream& print (std::ostream& os) const = 0;
    friend std::ostream& operator<< (std::ostream& os, const ExprAST& v) {
        return v.print(os);
    } 
    virtual std::pair<llvm::Value*, std::tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>> codegen(bool intern=false) = 0;
    virtual Citron::Type ty() = 0;
};

class NumberExprAST : public ExprAST {
    double val;
public:
    NumberExprAST(double val) : val(val) {}
    NumberExprAST(char const* val) : val(atof(val)) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << val;
    }
    virtual std::pair<llvm::Value*, std::tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>> codegen(bool intern=false);
    virtual Citron::Type ty() { return Citron::Type::NumberTy; }
};

class BoolExprAST : public ExprAST {
    bool val;
public:
    BoolExprAST(bool val) : val(val) {}
    virtual std::ostream& print (std::ostream& os) const {
        return os << (val ? "True" : "False");
    }
    virtual std::pair<llvm::Value*, std::tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>> codegen(bool intern=false);
    virtual Citron::Type ty() { return Citron::Type::BoolTy; }
};

class Message {
public:
    std::string name;
    argVec arguments;
    friend std::ostream& operator<< (std::ostream& os, const Message& m) {
        std::vector<std::string> strs;
        boost::split(strs, m.name, boost::is_any_of(":"));
        if (strs.size() == 1) {
            os << strs[0]; *(m.arguments[0]);
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
    virtual std::pair<llvm::Value*, std::tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>> codegen(bool intern=false);
    virtual Citron::Type ty() { 
        if(message_line.size() == 0) 
            return receiver->ty();
        auto msg = message_line[0];
        Citron::Type last_type = receiver->ty();
        for(int i=0; i<message_line.size(); i++) {
            auto& protos = llvmCoreValues::MethodCache[message_line[i].name];
            auto arg_v = std::vector<Citron::Type>{last_type};
            for (auto arg : msg.arguments)
                arg_v.push_back(arg->ty());
            auto& proto = protos[arg_v];
            last_type = std::get<0>(proto);
        }
        return last_type;
    }
};

static std::vector<Message> vectorize_names(ctr_tlistitem* tl);

struct nLLVMBasicNumericOp {
    std::function<llvm::Value*(std::vector<llvm::Value*>)> operator() (char op) {
        switch(op) {
            case '+':
                return [](std::vector<llvm::Value*> args) {
                    return Citron::llvmCoreValues::Builder.CreateFAdd(args[0], args[1], "faddtmp");
                };
            case '-':
                return [](std::vector<llvm::Value*> args) {
                    return Citron::llvmCoreValues::Builder.CreateFSub(args[0], args[1], "fsubtmp");
                };
            case '*':
                return [](std::vector<llvm::Value*> args) {
                    return Citron::llvmCoreValues::Builder.CreateFMul(args[0], args[1], "fmultmp");
                };
            case '/':
                return [](std::vector<llvm::Value*> args) {
                    return Citron::llvmCoreValues::Builder.CreateFDiv(args[0], args[1], "fdivtmp");
                };
            case '=':
                return [](std::vector<llvm::Value*> args) {
                    return Citron::llvmCoreValues::Builder.CreateFCmpOEQ(args[0], args[1], "cmpoeqtmp");
                };
            case '>':
                return [](std::vector<llvm::Value*> args) {
                    return Citron::llvmCoreValues::Builder.CreateFCmpOGT(args[0], args[1], "cmpogttmp");
                };
            case '<':
                return [](std::vector<llvm::Value*> args) {
                    return Citron::llvmCoreValues::Builder.CreateFCmpOLT(args[0], args[1], "cmpolttmp");
                };
            // case '<=:':
            //     return [](std::vector<llvm::Value*> args) {
            //         auto vp = Citron::llvmCoreValues::Builder.CreateFCmpOLE(args[0], args[1]);
            //         return Citron::llvmCoreValues::Builder.CreateUIToFP(vp, Citron::llvmTypes::DoubleTy);
            //     };
            // case '>=:':
            //     return [](std::vector<llvm::Value*> args) {
            //         auto vp = Citron::llvmCoreValues::Builder.CreateFCmpOGE(args[0], args[1]);
            //         return Citron::llvmCoreValues::Builder.CreateUIToFP(vp, Citron::llvmTypes::DoubleTy);
            //     };
        }
    }
};
}


#endif