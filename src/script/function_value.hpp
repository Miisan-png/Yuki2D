#pragma once

#include <vector>
#include <string>

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
    Environment* closure; // Not owned, but must ensure lifetime or use shared_ptr in Environment

    // Native Function
    NativeFn nativeFn;

    FunctionValue() 
        : isNative(false), body(nullptr), closure(nullptr), nativeFn(nullptr) {}
};

}
