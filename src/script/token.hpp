#pragma once
#include <string>
#include <vector>
namespace yuki {
enum class TokenType {
    Identifier, Number, String,
    LeftParen, RightParen, LeftBrace, RightBrace,
    Comma, Colon, Semicolon,
    Plus, Minus, Star, Slash, Percent, Equal, Bang,
    EqualEqual, BangEqual, Less, LessEqual, Greater, GreaterEqual,
    True, False,
    Eof
};
struct Token {
    TokenType type;
    std::string text;
};
class Tokenizer {
public:
    Tokenizer(const std::string& source);
    std::vector<Token> scanTokens();
private:
    std::string source;
};
}
