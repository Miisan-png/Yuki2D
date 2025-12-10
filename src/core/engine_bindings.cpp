#include "engine_bindings.hpp"
#include "log.hpp"
#include "time.hpp"
#include <iostream>
#include <cstdlib>
#include <GLFW/glfw3.h>
namespace yuki {
Value engineLog(const std::vector<Value>& args) {
    std::string msg;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i].type == ValueType::Number) msg += std::to_string(args[i].number);
        else if (args[i].type == ValueType::String) msg += args[i].text;
        else msg += "nil";
        if (i < args.size() - 1) msg += " ";
    }
    logInfo(msg);
    return Value::nilVal();
}
Value engineTime(const std::vector<Value>& args) {
    (void)args;
    return Value::numberVal(glfwGetTime());
}
Value engineRandom(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValueType::Number) return Value::numberVal(0);
    double max = args[0].number;
    double r = (static_cast<double>(std::rand()) / RAND_MAX) * max;
    return Value::numberVal(r);
}
void EngineBindings::registerBuiltins(std::unordered_map<std::string, NativeFn>& target) {
    target["engine_log"] = engineLog;
    target["engine_time"] = engineTime;
    target["engine_random"] = engineRandom;
}
}
