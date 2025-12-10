#pragma once

#include <vector>
#include <memory>
#include <string>
#include "token.hpp"
#include "ast.hpp"

namespace yuki {

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    const std::vector<Token>& tokens;
    int current = 0;

    // Statement parsing
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> funDecl();
    std::unique_ptr<Stmt> varDecl();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> returnStatement();
    std::unique_ptr<Stmt> exprStmt();
    std::unique_ptr<Block> block(); // Parses { ... } and returns Block node

    // Expression parsing
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logicOr();
    std::unique_ptr<Expr> logicAnd();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> primary();

    // Helpers
    bool match(const std::vector<TokenType>& types);
    bool matchKeyword(const std::string& keyword);
    bool check(TokenType type);
    bool checkKeyword(const std::string& keyword);
    Token advance();
    bool isAtEnd();
    Token peek();
    Token previous();
    Token consume(TokenType type, const std::string& message);
    void error(Token token, const std::string& message);
    void synchronize();
};

}