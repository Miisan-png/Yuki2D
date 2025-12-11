#include "engine_bindings.hpp"
#include "window.hpp"
#include "renderer2d.hpp"
#include "../script/value.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
namespace yuki {
static Window* g_Window = nullptr;
static Renderer2D* g_Renderer = nullptr;
void EngineBindings::init(Window* window, Renderer2D* renderer) {
    g_Window = window;
    g_Renderer = renderer;
}
Value engineLog(const std::vector<Value>& args) {
    std::string msg;
    for (size_t i = 0; i < args.size(); i++) {
        msg += args[i].toString();
    }
    printf("[INFO] %s\n", msg.c_str());
    return Value::nilVal();
}
Value engineTime(const std::vector<Value>&) {
    return Value::number(glfwGetTime());
}
Value engineRandom(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValueType::Number) return Value::number(0);
    double max = args[0].numberVal;
    double r = (double(rand()) / double(RAND_MAX)) * max;
    return Value::number(r);
}
Value engineSetClearColor(const std::vector<Value>& args) {
    if (args.size() >= 3 && g_Window) {
        g_Window->setClearColor(args[0].numberVal, args[1].numberVal, args[2].numberVal);
    }
    return Value::nilVal();
}
Value engineDrawRect(const std::vector<Value>& args) {
    if (args.size() >= 7 && g_Renderer) {
        g_Renderer->drawRect(args[0].numberVal, args[1].numberVal, args[2].numberVal, args[3].numberVal, args[4].numberVal, args[5].numberVal, args[6].numberVal);
    }
    return Value::nilVal();
}
Value engineLoadSprite(const std::vector<Value>& args) {
    if (args.size() >= 1 && g_Renderer) {
        return Value::number(g_Renderer->loadSprite(args[0].stringVal));
    }
    return Value::number(-1);
}
Value engineDrawSprite(const std::vector<Value>& args) {
    if (args.size() >= 3 && g_Renderer) {
        g_Renderer->drawSprite((int)args[0].numberVal, args[1].numberVal, args[2].numberVal);
    }
    return Value::nilVal();
}
Value engineSin(const std::vector<Value>& args) {
    if (args.size() >= 1 && args[0].isNumber()) {
        return Value::number(std::sin(args[0].numberVal));
    }
    return Value::number(0);
}
Value engineCos(const std::vector<Value>& args) {
    if (args.size() >= 1 && args[0].isNumber()) {
        return Value::number(std::cos(args[0].numberVal));
    }
    return Value::number(0);
}
void EngineBindings::registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["engine_log"] = engineLog;
    builtins["time"] = engineTime;
    builtins["random"] = engineRandom;
    builtins["set_clear_color"] = engineSetClearColor;
    builtins["draw_rect"] = engineDrawRect;
    builtins["load_sprite"] = engineLoadSprite;
    builtins["draw_sprite"] = engineDrawSprite;
    builtins["sin"] = engineSin;
    builtins["cos"] = engineCos;
}
} // namespace yuki
