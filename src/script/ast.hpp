#pragma once
#include <string>
#include <memory>
#include <vector>
#include "token.hpp"
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
struct Binary : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};
struct Call : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    Call(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
};
struct Stmt {
    virtual ~Stmt() = default;
};
struct ExpressionStmt : Stmt {
    std::unique_ptr<Expr> expression;
    ExpressionStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
};
struct VarDecl : Stmt {
    std::string name;
    std::unique_ptr<Expr> initializer;
    VarDecl(const std::string& name, std::unique_ptr<Expr> initializer)
        : name(name), initializer(std::move(initializer)) {}
};
struct Assign : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    Assign(const std::string& name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
};
struct Block : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    Block(std::vector<std::unique_ptr<Stmt>> statements) : statements(std::move(statements)) {}
};
struct FunctionDecl : Stmt {
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<Block> body;
    FunctionDecl(std::string name, std::vector<std::string> parameters, std::unique_ptr<Block> body)
        : name(std::move(name)), parameters(std::move(parameters)), body(std::move(body)) {}
};
struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};
struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
};
struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}
};
}
