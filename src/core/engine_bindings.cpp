#include "engine_bindings.hpp"
#include "../script/value.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>

namespace yuki {

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

void EngineBindings::registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["engine_log"] = engineLog;
    builtins["time"] = engineTime;
    builtins["random"] = engineRandom;
}

} // namespace yuki
