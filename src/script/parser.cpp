#include "parser.hpp"
namespace yuki {
Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}
std::unique_ptr<Expr> Parser::parseExpression() {
    return primary();
}
Token Parser::peek() const {
    return tokens[current];
}
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}
bool Parser::isAtEnd() const {
    return peek().type == TokenType::Eof;
}
std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::Number) || match(TokenType::String)) {
        return std::make_unique<Literal>(tokens[current - 1].text);
    }
    if (match(TokenType::Identifier)) {
        return std::make_unique<Identifier>(tokens[current - 1].text);
    }
    if (match(TokenType::LeftParen)) {
        std::unique_ptr<Expr> expr = parseExpression();
        if (match(TokenType::RightParen)) {
            return std::make_unique<Grouping>(std::move(expr));
        }
    }
    return nullptr;
}
}
