#include "environment.hpp"

namespace yuki {

Environment::Environment(Environment* parent) : parent(parent) {}

void Environment::define(const std::string& name, const Value& value) {
    values[name] = value;
}

bool Environment::assign(const std::string& name, const Value& value) {
    if (values.count(name)) {
        values[name] = value;
        return true;
    }
    if (parent) {
        return parent->assign(name, value);
    }
    return false;
}

std::optional<Value> Environment::get(const std::string& name) {
    if (values.count(name)) {
        return values.at(name);
    }
    if (parent) {
        return parent->get(name);
    }
    return std::nullopt;
}

}
