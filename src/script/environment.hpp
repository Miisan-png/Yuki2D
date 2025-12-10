#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include "value.hpp"
namespace yuki {
class Environment {
public:
    Environment();
    Environment(Environment* parent);
    void define(const std::string& name, const Value& value);
    bool assign(const std::string& name, const Value& value);
    std::optional<Value> get(const std::string& name) const;
private:
    std::unordered_map<std::string, Value> values;
    Environment* parent;
};
}
