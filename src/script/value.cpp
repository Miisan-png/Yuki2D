#include "value.hpp"
namespace yuki {
Value::Value() : type(ValueType::Nil), number(0.0) {}
Value Value::numberVal(double x) {
    Value v;
    v.type = ValueType::Number;
    v.number = x;
    return v;
}
Value Value::stringVal(const std::string& s) {
    Value v;
    v.type = ValueType::String;
    v.text = s;
    return v;
}
Value Value::nilVal() {
    return Value();
}
}
