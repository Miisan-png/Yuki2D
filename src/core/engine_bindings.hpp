#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "../script/value.hpp"
#include "../script/function_value.hpp"
namespace yuki {
class Window;
class Renderer2D;
class EngineBindings {
public:
    static void init(Window* window, Renderer2D* renderer);
    static void registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
};
Value engineLog(const std::vector<Value>& args);
Value engineTime(const std::vector<Value>& args);
Value engineRandom(const std::vector<Value>& args);
Value engineSetClearColor(const std::vector<Value>& args);
Value engineDrawRect(const std::vector<Value>& args);
Value engineLoadSprite(const std::vector<Value>& args);
Value engineDrawSprite(const std::vector<Value>& args);
Value engineSin(const std::vector<Value>& args);
Value engineCos(const std::vector<Value>& args);
}