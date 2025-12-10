#include "builtins.hpp"
#include "value.hpp"
#include <iostream>

namespace yuki {

Value builtinPrint(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << args[i].toString();
        if (i < args.size() - 1) std::cout << " ";
    }
    std::cout << std::endl;
    return Value::nilVal();
}

void registerScriptBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["print"] = builtinPrint;
}

}