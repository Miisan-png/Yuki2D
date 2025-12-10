#include "builtins.hpp"
#include <iostream>
namespace yuki {
Value builtinPrint(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i].type == ValueType::Number) {
            std::cout << args[i].number;
        } else if (args[i].type == ValueType::String) {
            std::cout << args[i].text;
        } else {
            std::cout << "nil";
        }
        if (i < args.size() - 1) std::cout << " ";
    }
    std::cout << std::endl;
    return Value::nilVal();
}
}
