#pragma once
#include <string>
#include <vector>
namespace yuki {
enum class TokenType {
    Identifier, Number, String,
    Nil,
    LeftBracket, RightBracket,
    LeftParen, RightParen, LeftBrace, RightBrace,
    Comma, Colon, Semicolon,
    Dot,
    Plus, Minus, Star, Slash, Percent, Equal, Bang,
    EqualEqual, BangEqual, Less, LessEqual, Greater, GreaterEqual,
    True, False,
    Eof
};
struct Token {
    TokenType type;
    std::string text;
    int line = 1;
    int column = 1;
};
class Tokenizer {
public:
    Tokenizer(const std::string& source);
    std::vector<Token> scanTokens();
    bool hadError() const { return !errors.empty(); }
    const std::vector<std::string>& getErrors() const { return errors; }
private:
    std::string source;
    std::vector<std::string> errors;
};
}
