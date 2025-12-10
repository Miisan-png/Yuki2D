#include "token.hpp"
#include <cctype>
namespace yuki {
Tokenizer::Tokenizer(const std::string& source) : source(source) {}
std::vector<Token> Tokenizer::scanTokens() {
    std::vector<Token> tokens;
    size_t current = 0;
    while (current < source.size()) {
        char c = source[current++];
        switch (c) {
            case ' ': case '\r': case '\t': case '\n': break;
            case '(': tokens.push_back({TokenType::LeftParen, "("}); break;
            case ')': tokens.push_back({TokenType::RightParen, ")"}); break;
            case '{': tokens.push_back({TokenType::LeftBrace, "{"}); break;
            case '}': tokens.push_back({TokenType::RightBrace, "}"}); break;
            case ',': tokens.push_back({TokenType::Comma, ","}); break;
            case ':': tokens.push_back({TokenType::Colon, ":"}); break;
            case ';': tokens.push_back({TokenType::Semicolon, ";"}); break;
            case '+': tokens.push_back({TokenType::Plus, "+"}); break;
            case '-': tokens.push_back({TokenType::Minus, "-"}); break;
            case '*': tokens.push_back({TokenType::Star, "*"}); break;
            case '/': tokens.push_back({TokenType::Slash, "/"}); break;
            case '=': tokens.push_back({TokenType::Equal, "="}); break;
            case '"': {
                std::string val;
                while (current < source.size() && source[current] != '"') {
                    val += source[current++];
                }
                if (current < source.size()) current++;
                tokens.push_back({TokenType::String, val});
                break;
            }
            default:
                if (std::isdigit(c)) {
                    std::string val;
                    val += c;
                    while (current < source.size() && std::isdigit(source[current])) {
                        val += source[current++];
                    }
                    tokens.push_back({TokenType::Number, val});
                } else if (std::isalpha(c) || c == '_') {
                    std::string val;
                    val += c;
                    while (current < source.size() && (std::isalnum(source[current]) || source[current] == '_')) {
                        val += source[current++];
                    }
                    tokens.push_back({TokenType::Identifier, val});
                }
                break;
        }
    }
    tokens.push_back({TokenType::Eof, ""});
    return tokens;
}
}
