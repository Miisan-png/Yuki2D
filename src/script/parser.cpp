#include "parser.hpp"
namespace yuki {
Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}
std::unique_ptr<Expr> Parser::parseExpression() {
    return parseCall();
}
Token Parser::peek() const {
    return tokens[current];
}
Token Parser::peekNext() const {
    if (current + 1 >= tokens.size()) return tokens[tokens.size() - 1];
    return tokens[current + 1];
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
bool Parser::checkNext(TokenType type) const {
    if (current + 1 >= tokens.size()) return false;
    return peekNext().type == type;
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
std::unique_ptr<Expr> Parser::parseBinary() {
    std::unique_ptr<Expr> expr = primary();
    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        Token op = tokens[current - 1];
        std::unique_ptr<Expr> right = primary();
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }
    return expr;
}
std::unique_ptr<Expr> Parser::parseCall() {
    std::unique_ptr<Expr> expr = primary();
    while (true) {
        if (match(TokenType::LeftParen)) {
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RightParen)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::Comma));
            }
            match(TokenType::RightParen);
            expr = std::make_unique<Call>(std::move(expr), std::move(args));
        } else {
            break;
        }
    }
    return expr;
}
}