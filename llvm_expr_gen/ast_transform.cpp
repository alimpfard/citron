#include "../citron.h"
ctr_size ctr_program_length;
#include "ast.hpp"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <numeric>
#include <memory>
#include <utility>
#include <tuple>

#include "fns.hpp"
#include "ops.hpp"

#define TEST

static int level = 0;
using Citron::argT;
using Citron::argVec;
using Citron::codegen_t;
using Citron::pipeline_t;

argT Citron::ExprAST::transform_expr(ctr_tnode* node) {
    switch(node->type) {
    case CTR_AST_NODE_EXPRMESSAGE: {
        auto receiver = transform_expr(node->nodes->node);
        return std::make_shared<MessagesExpr>(MessagesExpr(
            std::move(receiver),
            vectorize_names(node->nodes->next)
        ));
    }
    case CTR_AST_NODE_LTRNUM:
        return std::make_unique<NumberExprAST>(NumberExprAST(node->value));
    case CTR_AST_NODE_NESTED:
        return transform_expr(node->nodes->node);
    case CTR_AST_NODE_LTRBOOLTRUE:
        return std::make_unique<BoolExprAST>(BoolExprAST(true));
    case CTR_AST_NODE_LTRBOOLFALSE:
        return std::make_unique<BoolExprAST>(BoolExprAST(false));
    default:
        std::__throw_runtime_error("Unimplemented node type");
    }
}

std::vector<Citron::Message> Citron::vectorize_names(ctr_tlistitem* li) {
    std::vector<Message> vs;
    ctr_tnode* node;
    while(li) {
        node = li->node;
        if (!node) break;
        switch(node->type) {
        case CTR_AST_NODE_UNAMESSAGE: 
            vs.push_back(UnaMessage(node->value, node->vlen)); 
            break;
        case CTR_AST_NODE_BINMESSAGE: {
            argT arg = ExprAST::transform_expr(node->nodes->node);
            vs.push_back(BinMessage(node->value, node->vlen, std::forward<argT>(arg)));
            break;
        }
        case CTR_AST_NODE_KWMESSAGE: {
            std::vector<argT> args;
            ctr_tlistitem* arg_l = node->nodes;
            while(arg_l) { 
                argT arg = ExprAST::transform_expr(arg_l->node);
                args.push_back(arg);
                arg_l = arg_l->next;
            }
            vs.push_back(KWMessage(node->value, node->vlen, args));
        }
        }
        li = li->next;
    }
    return vs;
}

codegen_t Citron::NumberExprAST::codegen(bool intern) {
    return std::make_pair<llvm::Value*, std::tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>>(
        llvm::ConstantFP::get(llvmCoreValues::TheContext, llvm::APFloat(val)), 
        std::make_tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>(
            {nullptr},
            {}, 
            Citron::Type::DummyTy
        )
    );
}

codegen_t Citron::BoolExprAST::codegen(bool intern) {
    return std::make_pair<llvm::Value*, std::tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>>(
        llvm::ConstantInt::get(llvmCoreValues::TheContext, llvm::APInt(1, val, false)), 
        std::make_tuple<argT, std::vector<std::tuple<mCache_t, std::vector<llvm::Value*>, int>>, Citron::Type>(
            {nullptr},
            {}, 
            Citron::Type::DummyTy
        )
    );
}

void resolve_argument(int& did_recurse, codegen_t& v, std::vector<llvm::Value*>& ArgsV, pipeline_t& pipeline) {
    if(!v.first) {
        auto vs = v.second;
        for(int vi=0, e=std::get<1>(vs).size(); vi<e; vi++) {
            did_recurse = 2;
            std::tuple<mCache_t, std::vector<llvm::Value*>, int> vf = std::get<1>(vs)[vi];
            if(vi == 0) {
                if (e == 1)
                    did_recurse = 1;
                v = std::get<0>(vs)->codegen(true);
                if(v.first) {
                    auto va = std::get<1>(vf);
                    va.insert(va.begin(), {v.first});
                    vf = std::make_tuple(
                        std::get<0>(vf),
                        va,
                        did_recurse
                    );
                } else
                    resolve_argument(did_recurse, v, ArgsV, pipeline);
            }
            pipeline.push_back(vf);
        }
            // TODO: Do something with the resulting type of vs.second
    } else
        ArgsV.push_back(v.first);
}

void resolve_receiver(argT& receiver, llvm::Value*& last_value, pipeline_t& pipeline) {
    auto rec_ = receiver->codegen(true);
    if (rec_.first) {//native expression
        last_value = rec_.first;
    } else {//some complex expression
        auto plinet = rec_.second;
        auto rec_new = std::get<0>(plinet);
        resolve_receiver(rec_new, last_value, pipeline);
        auto pline = std::get<1>(plinet);
        pipeline.insert(pipeline.begin(), pline.begin(), pline.end());
    }
}

codegen_t Citron::MessagesExpr::codegen(bool intern) {
    std::cout << " >> " << message_line << '\n';
    if(message_line.size() == 0) 
        return receiver->codegen(intern);
    Citron::Type last_type = receiver->ty();
    pipeline_t pipeline;
    for(int i=0; i<message_line.size(); i++) {
        Message msg = message_line[i];
        std::cout << "Processing some " << msg << "\n"; 
        auto& protos = llvmCoreValues::MethodCache[msg.name];
        auto arg_v = std::vector<Citron::Type>{last_type};
        for (auto arg : msg.arguments)
            arg_v.push_back(arg->ty());
        auto& proto = protos[arg_v];
        bool native_op = true;
        mCache_t fn = std::get<1>(proto);
        std::vector<llvm::Value*> ArgsV {};
        int did_recurse = 0;
        for(int i=0, e=msg.arguments.size(); i<e; i++) {
            did_recurse = 0;
            std::cout << std::string(level++, '\t') << "| argument#" << i << " of (" << msg << ") start\n";
            auto v=msg.arguments[i]->codegen(true);
            resolve_argument(did_recurse, v, ArgsV, pipeline);
            std::cout << std::string(--level, '\t') << "| argument#" << i << " of (" << msg << ") end\n";
            if(ArgsV.size() > 0 && !ArgsV.back()) {
                std::cout << "Saw a null argument: " << ArgsV << std::endl;
                std::__throw_bad_variant_access("ArgsV");
                return {nullptr, {{nullptr}, {}, Citron::Type::DummyTy}};
            }
        }
        std::cout << " << (" << msg.name << "): ArgsV " << std::flush;
        for(auto arg : ArgsV) arg->print(llvm::errs(), false);
        std::cout << "  did_recurse " << did_recurse << "\n";
        valProv_* fp = fn.second;
        if(!fn.first && !fp) {
            std::string err = "Invalid function target for op " + msg.name;
            std::__throw_runtime_error(err.c_str());
        }
        pipeline.push_back(std::make_tuple(fn, ArgsV, did_recurse?msg.arguments.size()+2:0));
        last_type = std::get<0>(proto);
    }
    //std::cout << pipeline << "\n";
    if(intern) return {nullptr, {receiver, pipeline, last_type}};
    llvm::FunctionType* FT = llvm::FunctionType::get(llvmCoreValues::getLLVMTy(last_type), false);
    llvm::Function* F = llvm::Function::Create(FT, llvm::Function::InternalLinkage, llvmCoreValues::fresh(), llvmCoreValues::TheModule.get());
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvmCoreValues::TheContext, "entry", F);
    llvm::Value* vp;
    std::vector<llvm::Value*> tmp;
    llvm::Value* last_value;
    resolve_receiver(receiver, last_value, pipeline);
    bool addnext = false;
    for(decltype(pipeline)::iterator it = pipeline.begin(); it < pipeline.end(); it++) {
        llvmCoreValues::Builder.SetInsertPoint(BB);
        std::vector<llvm::Value*> vec = std::get<1>(*it);
        int val = std::get<2>(*it);
        std::cout << "val = " << val << ", vec = " << vec << ", last_value = " << last_value << ", tmp = " << tmp <<'\n';
        if(val == 0)
            vec.insert(vec.begin(), {last_value});
        else {
            if (tmp.size() == 0) {
                if (val == 2) {
                    vec.insert(vec.begin(), {last_value});
                } else if (val > 2) {
                    std::__throw_runtime_error("Too many temporaries requested");
                }
            } else {
                if (val >= 2) {
                    std::vector<llvm::Value*> pvs = {last_value};
                    for (int x=0; x<val-2; x++) {
                        last_value = tmp.back();
                        tmp.pop_back();
                        pvs.push_back(last_value);
                    }
                    vec.insert(vec.begin(), pvs.rbegin(), pvs.rend());
                }
            }
            tmp.push_back(last_value);
        }
        llvm::Function* fp;
        auto& variant = std::get<0>(*it);
        if((fp = variant.first))
            vp = llvmCoreValues::Builder.CreateCall(fp, vec, "calltmp");
        else {
            valProv_* f = variant.second;
            vp = f(vec);
        }
        std::cout << vp << " = {" << std::flush;
        vp->print(llvm::errs(), false);
        std::cout << "  }" << std::endl;

        last_value = vp;
    }
    llvmCoreValues::Builder.CreateRet(vp);
    llvm::raw_fd_ostream os(1, false, false);
    std::cout << "v-----------------------------------------------v\n";
    // F->print(llvm::errs(), nullptr, false, true);
    if(llvm::verifyFunction(*F, &os)) {
        // Citron::llvmCoreValues::TheModule->print(llvm::errs(), nullptr, false, true);
        // return nullptr;
    }
    std::cout << "^-----------------------------------------------^\n";
    auto p = F;//llvmCoreValues::Builder.CreateCall(F, {}, "calltmp");
    
    return {p, {nullptr, {}, Citron::Type::DummyTy}};
}

void Citron::llvmCoreValues::populateCache() {
    Citron::Type NumberTy = Citron::Type::NumberTy;
    Citron::Type BoolTy = Citron::Type::BoolTy;
    std::map<std::vector<Citron::Type>, std::pair<Citron::Type, llvm::Function*>> fps;
    // llvm::Module* TheeModule = TheModule.get();
    // auto ebuild = llvm::EngineBuilder(std::move(TheModule));
    LLVMInitializeNativeTarget();
    // llvm::ExecutionEngine* ee = ebuild.setErrorStr(&errors).setEngineKind(llvm::EngineKind::JIT).create();
    // if (!ee) {
    //     std::cout << errors << "\n";
    //     exit(EXIT_FAILURE);
    // }
    llvm::Type *Dtype = llvm::Type::getDoubleTy(TheContext);
    llvm::FunctionType* FT = llvm::FunctionType::get(Dtype, {Dtype, Dtype}, false);
    llvm::Function* fp;
    //  llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "mplus_num", TheModule.get());
    Citron::nLLVMBasicNumericOp opgen;
    std::vector<Citron::Type> twoDoubles = {NumberTy, NumberTy};
    auto opV = opgen('+');
    mCache_t plusop = mCache_t(nullptr, opV);
    MethodCache["+"] = {
        {twoDoubles, {NumberTy, plusop}}
    };
    // fp = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "msub_num", TheModule.get());
    MethodCache["-"] = {
        {{NumberTy, NumberTy}, {NumberTy, mCache_t(nullptr, opgen('-'))}}
    };
    // fp = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "mmul_num", TheModule.get());
    MethodCache["*"] = {
        {{NumberTy, NumberTy}, {NumberTy, mCache_t(nullptr, opgen('*'))}}
    };
    // fp = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "mdiv_num", TheModule.get());
    MethodCache["/"] = {
        {{NumberTy, NumberTy}, {NumberTy, mCache_t(nullptr, opgen('/'))}}
    };
    MethodCache["="] = {
        {{NumberTy, NumberTy}, {BoolTy, mCache_t(nullptr, opgen('='))}},
        {{BoolTy, BoolTy}, {BoolTy, mCache_t(nullptr, opgen("bb="))}},
        {{NumberTy, BoolTy}, {BoolTy, mCache_t(nullptr, opgen("nb="))}},
        {{BoolTy, NumberTy}, {NumberTy, mCache_t(nullptr, opgen("bn="))}}
    };
    MethodCache[">"] = {
        {{NumberTy, NumberTy}, {BoolTy, mCache_t(nullptr, opgen('>'))}}
    };
    MethodCache["<"] = {
        {{NumberTy, NumberTy}, {BoolTy, mCache_t(nullptr, opgen('<'))}}
    };
    MethodCache[">=:"] = {
        {{NumberTy, NumberTy}, {BoolTy, mCache_t(nullptr, opgen(">=:"))}}
    };
    MethodCache["<=:"] = {
        {{NumberTy, NumberTy}, {BoolTy, mCache_t(nullptr, opgen("<=:"))}}
    };
    MethodCache["not"] = {
        {{BoolTy}, {BoolTy, mCache_t(nullptr, opgen("not"))}}
    };
    fp = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "mpow_num", TheModule.get());
    MethodCache["pow:"] = {
        {{NumberTy, NumberTy}, {NumberTy, mCache_t(fp, emptyllvmValueProvider)}}
    };

    FT = llvm::FunctionType::get(Dtype, {Dtype, Dtype, Dtype}, false);
    fp = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "mdummy_num", TheModule.get());
    MethodCache["test:this:"] = {
        {{NumberTy, NumberTy, NumberTy}, {NumberTy, mCache_t(fp, emptyllvmValueProvider)}}
    };

    auto vtest = MethodCache["+"][{NumberTy, NumberTy}];
    MethodCache["+"][{NumberTy, NumberTy}] = vtest;
}

#ifdef TEST

#include <fstream>
#include <streambuf>
#include <llvm/Support/FileSystem.h>

int main() {
    Citron::llvmCoreValues::populateCache();
    ctr_initialize_ex(83886080, 0);
    std::ifstream t("file.txt");
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto s = buffer.str();
    std::cout << "\x1b[92mCompiling " << s << "\x1b[0m\n";
    ctr_program_length = s.length();
    ctr_clex_load(const_cast<char*>(s.c_str()));
    ctr_tnode* tnode = ctr_cparse_parse(const_cast<char*>(s.c_str()), "llvm");
    tnode = tnode->nodes->node;
    Citron::argT ast = Citron::ExprAST::transform_expr(tnode);
    std::cout << "\x1b[91mAST: " << *ast << "\x1b[0m\nDebug stuff:\n\x1b[2m";
    auto fn = ast->codegen();
    std::cout << "\x1b[0m\n";
    fn.first->print(llvm::errs());
    std::error_code EC;
    auto os = llvm::raw_fd_ostream("out.ll", EC, llvm::sys::fs::OpenFlags::F_None);
    Citron::llvmCoreValues::TheModule->print(os, nullptr);
    return 0;
}
#endif