#pragma once
#include <string>
namespace yuki {
enum class ValueType { Number, String, Nil };
struct Value {
    ValueType type;
    double number;
    std::string text;
    Value();
    static Value numberVal(double x);
    static Value stringVal(const std::string& s);
    static Value nilVal();
};
}
