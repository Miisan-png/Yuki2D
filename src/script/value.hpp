#pragma once
#include <string>
#include <vector>
#include "token.hpp"
namespace yuki {
class Environment;
struct Block;
enum class ValueType { Number, String, Function, Nil };
struct FunctionValue {
    std::vector<std::string> parameters;
    Block* body;
    Environment* closure;
};
struct Value {
    ValueType type;
    double number;
    std::string text;
    FunctionValue* function;
    Value();
    static Value numberVal(double x);
    static Value stringVal(const std::string& s);
    static Value functionVal(FunctionValue* fn);
    static Value nilVal();
};
}