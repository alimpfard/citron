#pragma once

#include <variant>
#include <vector>
#include <cstdarg>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>

using mCache_t=std::variant<llvm::Function*, std::function<llvm::Value*(std::vector<llvm::Value*>)>>;
