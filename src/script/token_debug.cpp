#include "token_debug.hpp"
#include <iostream>
#include <string>
namespace yuki {
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::Comma: return "Comma";
        case TokenType::Colon: return "Colon";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Equal: return "Equal";
        case TokenType::Eof: return "Eof";
        default: return "Unknown";
    }
}
void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << " \"" << token.text << "\"" << std::endl;
    }
}
}