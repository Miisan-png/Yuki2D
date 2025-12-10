#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "../script/value.hpp"
#include "../script/builtins.hpp"
namespace yuki {
class EngineBindings {
public:
    static void registerBuiltins(std::unordered_map<std::string, NativeFn>& target);
};
}
