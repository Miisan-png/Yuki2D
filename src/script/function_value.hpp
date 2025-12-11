#pragma once

#include <vector>
#include <string>
#include <memory>

namespace yuki {

class Environment;
struct Block;
struct Value;

using NativeFn = Value(*)(const std::vector<Value>&);

struct FunctionValue {
    bool isNative;
    std::string name;

    // Script Function
    std::vector<std::string> parameters;
    Block* body; // Owned by AST, not FunctionValue
    std::shared_ptr<Environment> closure; // Shared lifetime with captured scope

    // Native Function
    NativeFn nativeFn;

    FunctionValue() 
        : isNative(false), body(nullptr), closure(nullptr), nativeFn(nullptr) {}
};

}
