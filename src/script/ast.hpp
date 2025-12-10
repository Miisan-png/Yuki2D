#pragma once
#include <string>
#include <memory>
namespace yuki {
struct Expr {
    virtual ~Expr() = default;
};
struct Literal : Expr {
    std::string value;
    Literal(const std::string& value) : value(value) {}
};
struct Identifier : Expr {
    std::string name;
    Identifier(const std::string& name) : name(name) {}
};
struct Grouping : Expr {
    std::unique_ptr<Expr> expression;
    Grouping(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
};
}
