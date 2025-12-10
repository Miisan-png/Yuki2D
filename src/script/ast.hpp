#pragma once

#include <string>
#include <vector>
#include <memory>

namespace yuki {

// Forward declarations
struct Expr;
struct Stmt;
struct Literal;
struct Variable;
struct Assign;
struct Binary;
struct Call;
struct ExpressionStmt;
struct VarDecl;
struct Block;
struct FunctionDecl;
struct ReturnStmt;
struct IfStmt;
struct WhileStmt;

enum class ExprKind {
    Literal,
    Variable,
    Assign,
    Binary,
    Call
};

enum class StmtKind {
    Expression,
    VarDecl,
    Block,
    Function,
    Return,
    If,
    While
};

// Base Expression
struct Expr {
    virtual ~Expr() = default;
    virtual ExprKind getKind() const = 0;
};

// Base Statement
struct Stmt {
    virtual ~Stmt() = default;
    virtual StmtKind getKind() const = 0;
};

// Expressions

struct Literal : Expr {
    std::string value; // Storing as string to be parsed at runtime by Interpreter/Value
    Literal(const std::string& value) : value(value) {}
    ExprKind getKind() const override { return ExprKind::Literal; }
};

struct Variable : Expr {
    std::string name;
    Variable(const std::string& name) : name(name) {}
    ExprKind getKind() const override { return ExprKind::Variable; }
};

struct Assign : Expr {
    std::string name;
    std::unique_ptr<Expr> value;
    Assign(const std::string& name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
    ExprKind getKind() const override { return ExprKind::Assign; }
};

struct Binary : Expr {
    std::unique_ptr<Expr> left;
    // We assume TokenType is defined in token.hpp, but here we just need the operator token
    // For simplicity in this header, we can store the operator as a struct or include token.hpp
    // To minimize dependencies in AST, we usually include token.hpp
    struct Op { int type; std::string lexeme; } op; 
    std::unique_ptr<Expr> right;
    
    // Constructor matching typical parser usage. 
    // We'll use a simplified Op struct if Token isn't available, but let's assume we can include "token.hpp"
    // However, to keep this file standalone as requested, I'll allow the Op to be passed.
    // Ideally, include "token.hpp".
    Binary(std::unique_ptr<Expr> left, int opType, std::string opLexeme, std::unique_ptr<Expr> right)
        : left(std::move(left)), op{opType, opLexeme}, right(std::move(right)) {}
    ExprKind getKind() const override { return ExprKind::Binary; }
};

struct Call : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    Call(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
    ExprKind getKind() const override { return ExprKind::Call; }
};

// Statements

struct ExpressionStmt : Stmt {
    std::unique_ptr<Expr> expression;
    ExpressionStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}
    StmtKind getKind() const override { return StmtKind::Expression; }
};

struct VarDecl : Stmt {
    std::string name;
    std::unique_ptr<Expr> initializer;
    VarDecl(const std::string& name, std::unique_ptr<Expr> initializer)
        : name(name), initializer(std::move(initializer)) {}
    StmtKind getKind() const override { return StmtKind::VarDecl; }
};

struct Block : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    Block(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}
    StmtKind getKind() const override { return StmtKind::Block; }
};

struct FunctionDecl : Stmt {
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<Block> body;
    FunctionDecl(const std::string& name, std::vector<std::string> parameters, std::unique_ptr<Block> body)
        : name(name), parameters(std::move(parameters)), body(std::move(body)) {}
    StmtKind getKind() const override { return StmtKind::Function; }
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
    StmtKind getKind() const override { return StmtKind::Return; }
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
    StmtKind getKind() const override { return StmtKind::If; }
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}
    StmtKind getKind() const override { return StmtKind::While; }
};

}