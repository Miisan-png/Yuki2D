#include "token.hpp"
#include <cctype>
namespace yuki {
Tokenizer::Tokenizer(const std::string& source) : source(source) {}
std::vector<Token> Tokenizer::scanTokens() {
    std::vector<Token> tokens;
    size_t current = 0;
    int line = 1;
    int col = 1;
    while (current < source.size()) {
        int startLine = line;
        int startCol = col;
        char c = source[current++];
        col++;
        switch (c) {
            case ' ': case '\r': case '\t': break;
            case '\n': line++; col = 1; break;
            case '(': tokens.push_back({TokenType::LeftParen, "(", startLine, startCol}); break;
            case ')': tokens.push_back({TokenType::RightParen, ")", startLine, startCol}); break;
            case '{': tokens.push_back({TokenType::LeftBrace, "{", startLine, startCol}); break;
            case '}': tokens.push_back({TokenType::RightBrace, "}", startLine, startCol}); break;
            case ',': tokens.push_back({TokenType::Comma, ",", startLine, startCol}); break;
            case ':': tokens.push_back({TokenType::Colon, ":", startLine, startCol}); break;
            case ';': tokens.push_back({TokenType::Semicolon, ";", startLine, startCol}); break;
            case '+': tokens.push_back({TokenType::Plus, "+", startLine, startCol}); break;
            case '-': tokens.push_back({TokenType::Minus, "-", startLine, startCol}); break;
            case '*': tokens.push_back({TokenType::Star, "*", startLine, startCol}); break;
            case '/': tokens.push_back({TokenType::Slash, "/", startLine, startCol}); break;
            case '%': tokens.push_back({TokenType::Percent, "%", startLine, startCol}); break;
            case '=': 
                if (current < source.size() && source[current] == '=') {
                    current++;
                    col++;
                    tokens.push_back({TokenType::EqualEqual, "==", startLine, startCol});
                } else {
                    tokens.push_back({TokenType::Equal, "=", startLine, startCol});
                }
                break;
            case '!':
                if (current < source.size() && source[current] == '=') {
                    current++;
                    col++;
                    tokens.push_back({TokenType::BangEqual, "!=", startLine, startCol});
                } else {
                    tokens.push_back({TokenType::Bang, "!", startLine, startCol});
                }
                break;
            case '<':
                if (current < source.size() && source[current] == '=') {
                    current++;
                    col++;
                    tokens.push_back({TokenType::LessEqual, "<=", startLine, startCol});
                } else {
                    tokens.push_back({TokenType::Less, "<", startLine, startCol});
                }
                break;
            case '>':
                if (current < source.size() && source[current] == '=') {
                    current++;
                    col++;
                    tokens.push_back({TokenType::GreaterEqual, ">=", startLine, startCol});
                } else {
                    tokens.push_back({TokenType::Greater, ">", startLine, startCol});
                }
                break;
            case '"': {
                std::string val;
                while (current < source.size() && source[current] != '"') {
                    if (source[current] == '\n') { line++; col = 1; }
                    val += source[current++];
                    col++;
                }
                if (current < source.size()) { current++; col++; }
                tokens.push_back({TokenType::String, val, startLine, startCol});
                break;
            }
            case '[': tokens.push_back({TokenType::LeftBracket, "[", startLine, startCol}); break;
            case ']': tokens.push_back({TokenType::RightBracket, "]", startLine, startCol}); break;
            default:
                if (std::isdigit(c)) {
                    std::string val;
                    val += c;
                    while (current < source.size() && std::isdigit(source[current])) {
                        val += source[current++];
                        col++;
                    }
                    if (current < source.size() && source[current] == '.') {
                        val += source[current++];
                        col++;
                        while (current < source.size() && std::isdigit(source[current])) {
                            val += source[current++];
                            col++;
                        }
                    }
                    tokens.push_back({TokenType::Number, val, startLine, startCol});
                } else if (std::isalpha((unsigned char)c) || c == '_') {
                    std::string val;
                    val += c;
                    while (current < source.size() && (std::isalnum((unsigned char)source[current]) || source[current] == '_')) {
                        val += source[current++];
                        col++;
                    }
                    if (val == "true") tokens.push_back({TokenType::True, "true", startLine, startCol});
                    else if (val == "false") tokens.push_back({TokenType::False, "false", startLine, startCol});
                    else if (val == "nil") tokens.push_back({TokenType::Nil, "nil", startLine, startCol});
                    else if (val == "fn") tokens.push_back({TokenType::Identifier, "fn", startLine, startCol});
                    else if (val == "var") tokens.push_back({TokenType::Identifier, "var", startLine, startCol});
                    else if (val == "if") tokens.push_back({TokenType::Identifier, "if", startLine, startCol});
                    else if (val == "else") tokens.push_back({TokenType::Identifier, "else", startLine, startCol});
                    else if (val == "while") tokens.push_back({TokenType::Identifier, "while", startLine, startCol});
                    else if (val == "return") tokens.push_back({TokenType::Identifier, "return", startLine, startCol});
                    else tokens.push_back({TokenType::Identifier, val, startLine, startCol});
                }
                break;
        }
    }
    tokens.push_back({TokenType::Eof, "", line, col});
    return tokens;
}
}
