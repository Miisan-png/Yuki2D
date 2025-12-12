#pragma once
#include <string>
#include <unordered_map>
#include "../script/function_value.hpp"

namespace yuki {
class Window;
class Renderer2D;
class Interpreter;

class EngineBindings {
public:
    static void init(Window* window, Renderer2D* renderer, Interpreter* interpreter);
    static void update(double dt);
    static void setAssetBase(const std::string& base);
    static void registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
};
} // namespace yuki
