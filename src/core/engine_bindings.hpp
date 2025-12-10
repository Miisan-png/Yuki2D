#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "../script/value.hpp"
#include "../script/function_value.hpp"

namespace yuki {

class EngineBindings {
public:
    static void registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
};

Value engineLog(const std::vector<Value>& args);
Value engineTime(const std::vector<Value>& args);
Value engineRandom(const std::vector<Value>& args);

}