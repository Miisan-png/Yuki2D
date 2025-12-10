#pragma once

#include <string>
#include <vector>
#include "function_value.hpp"

namespace yuki {

enum class ValueType {
    Nil,
    Number,
    Bool,
    String,
    Function
};

struct Value {
    ValueType type;

    double numberVal;
    bool boolVal;
    std::string stringVal;
    FunctionValue* functionVal; // We treat this as a raw pointer reference.

    Value();

    // Factories
    static Value nilVal();
    static Value number(double val);
    static Value boolean(bool val);
    static Value string(const std::string& val);
    static Value function(FunctionValue* val);

    // Checks
    bool isNil() const;
    bool isNumber() const;
    bool isBool() const;
    bool isString() const;
    bool isFunction() const;

    // Conversion
    std::string toString() const;
};

}