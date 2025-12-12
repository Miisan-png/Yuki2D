#pragma once
#include <unordered_map>
#include "../engine_bindings.hpp"

namespace yuki {
void registerCoreBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
void registerRenderBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
void registerAnimBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
void registerCollisionBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
void registerInputBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
void registerTweenBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
void registerMathBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
} // namespace yuki
