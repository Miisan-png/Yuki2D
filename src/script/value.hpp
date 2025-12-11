#pragma once

#include <string>
#include <vector>
#include "function_value.hpp"
#include <unordered_map>
#include <memory>
#include <vector>

namespace yuki {

enum class ValueType {
    Nil,
    Number,
    Bool,
    String,
    Function,
    Map,
    Array
};

struct Value {
    ValueType type;

    double numberVal;
    bool boolVal;
    std::string stringVal;
    FunctionValue* functionVal; // We treat this as a raw pointer reference.
    std::shared_ptr<std::unordered_map<std::string, Value>> mapPtr;
    std::shared_ptr<std::vector<Value>> arrayPtr;

    Value();

    // Factories
    static Value nilVal();
    static Value number(double val);
    static Value boolean(bool val);
    static Value string(const std::string& val);
    static Value function(FunctionValue* val);
    static Value map(const std::unordered_map<std::string, Value>& val);
    static Value array(const std::vector<Value>& val);

    // Checks
    bool isNil() const;
    bool isNumber() const;
    bool isBool() const;
    bool isString() const;
    bool isFunction() const;
    bool isMap() const;
    bool isArray() const;

    // Conversion
    std::string toString() const;
};

}
