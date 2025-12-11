#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include "value.hpp"

namespace yuki {

class Environment {
public:
    Environment* parent;
    std::unordered_map<std::string, Value> values;

    Environment(Environment* parent);
    
    void define(const std::string& name, const Value& value);
    bool assign(const std::string& name, const Value& value);
    std::optional<Value> get(const std::string& name);
};

}
