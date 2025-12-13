#pragma once
#include <string>
#include <unordered_map>
#include <vector>
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
    static std::string resolveAssetPath(const std::string& rel);
    static std::vector<std::string> getLoadedModulePaths();
    static void registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
};
} // namespace yuki
