#pragma once
#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>
namespace yuki {
class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<Expr> parseExpression();
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token peekNext() const;
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool checkNext(TokenType type) const;
private:
    const std::vector<Token>& tokens;
    size_t current = 0;
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> parseBinary();
    std::unique_ptr<Expr> parseCall();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseEquality();
};
}