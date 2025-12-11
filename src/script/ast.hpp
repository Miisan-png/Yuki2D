#pragma once

#include <string>
#include <vector>
#include <memory>

namespace yuki {

// Forward declarations
struct Expr;
struct Stmt;
struct Literal;
struct VarExpr; // Renamed from Variable
struct AssignExpr; // Renamed from Assign
struct Binary;
struct Unary;
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
    VarExpr, // Renamed
    AssignExpr, // Renamed
    Unary,
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
    std::string value;
    Literal(const std::string& value) : value(value) {}
    ExprKind getKind() const override { return ExprKind::Literal; }
};

struct VarExpr : Expr {
    std::string name;
    VarExpr(const std::string& name) : name(name) {}
    ExprKind getKind() const override { return ExprKind::VarExpr; }
};

struct AssignExpr : Expr {
    std::string name;
    std::unique_ptr<Expr> value;
    AssignExpr(const std::string& name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
    ExprKind getKind() const override { return ExprKind::AssignExpr; }
};

struct Binary : Expr {
    std::unique_ptr<Expr> left;
    struct Op { int type; std::string lexeme; } op; 
    std::unique_ptr<Expr> right;
    
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

struct Unary : Expr {
    struct Op { int type; std::string lexeme; } op;
    std::unique_ptr<Expr> right;
    Unary(int opType, std::string opLexeme, std::unique_ptr<Expr> right)
        : op{opType, opLexeme}, right(std::move(right)) {}
    ExprKind getKind() const override { return ExprKind::Unary; }
};

// Statements (Unchanged logic, just ensure consistency)

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
