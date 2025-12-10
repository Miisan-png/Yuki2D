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
private:
    const std::vector<Token>& tokens;
    size_t current = 0;
    Token peek() const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    std::unique_ptr<Expr> primary();
};
}
