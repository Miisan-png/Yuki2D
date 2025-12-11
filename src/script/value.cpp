#include "value.hpp"

namespace yuki {

Value::Value() 
    : type(ValueType::Nil), 
      numberVal(0.0), 
      boolVal(false), 
      functionVal(nullptr) {}

Value Value::nilVal() {
    return Value();
}

Value Value::number(double val) {
    Value v;
    v.type = ValueType::Number;
    v.numberVal = val;
    return v;
}

Value Value::boolean(bool val) {
    Value v;
    v.type = ValueType::Bool;
    v.boolVal = val;
    return v;
}

Value Value::string(const std::string& val) {
    Value v;
    v.type = ValueType::String;
    v.stringVal = val;
    return v;
}

Value Value::function(FunctionValue* val) {
    Value v;
    v.type = ValueType::Function;
    v.functionVal = val;
    return v;
}
Value Value::map(const std::unordered_map<std::string, Value>& val) {
    Value v;
    v.type = ValueType::Map;
    v.mapPtr = std::make_shared<std::unordered_map<std::string, Value>>(val);
    return v;
}
Value Value::array(const std::vector<Value>& val) {
    Value v;
    v.type = ValueType::Array;
    v.arrayPtr = std::make_shared<std::vector<Value>>(val);
    return v;
}

bool Value::isNil() const {
    return type == ValueType::Nil;
}

bool Value::isNumber() const {
    return type == ValueType::Number;
}

bool Value::isBool() const {
    return type == ValueType::Bool;
}

bool Value::isString() const {
    return type == ValueType::String;
}

bool Value::isFunction() const {
    return type == ValueType::Function;
}
bool Value::isMap() const {
    return type == ValueType::Map;
}
bool Value::isArray() const {
    return type == ValueType::Array;
}

std::string Value::toString() const {
    switch (type) {
        case ValueType::Number:   return std::to_string(numberVal);
        case ValueType::Bool:     return boolVal ? "true" : "false";
        case ValueType::String:   return stringVal;
        case ValueType::Function: return "<function " + (functionVal ? functionVal->name : "") + ">";
        case ValueType::Map:      return "<map>";
        case ValueType::Array:    return "<array>";
        default:                  return "nil";
    }
}

}
