#pragma once

#include <utility>
#include <vector>
#include <cstdarg>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>

typedef llvm::Value* valProv_(std::vector<llvm::Value*>);
using llvmValueProvider = std::function<valProv_>;

llvm::Value* emptyllvmValueProvider_(std::vector<llvm::Value*> arg) {
    return nullptr;
}
constexpr valProv_* emptyllvmValueProvider = &emptyllvmValueProvider_;
using mCache_t = std::pair<llvm::Function*, valProv_*>;
