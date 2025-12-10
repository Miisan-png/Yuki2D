#pragma once

#include <unordered_map>
#include <string>
#include "../script/function_value.hpp"

namespace yuki {
    void registerScriptBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
    Value builtinPrint(const std::vector<Value>& args);
}