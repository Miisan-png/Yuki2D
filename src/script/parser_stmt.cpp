#include "script/parser_stmt.hpp"
namespace yuki {
std::unique_ptr<Stmt> parseDeclaration(Parser& parser) {
    // Try to parse VarDecl
    if (parser.check(TokenType::Identifier) && parser.peek().text == "let") {
        parser.advance(); // consume "let"
        if (parser.check(TokenType::Identifier)) { // peek identifier
            Token nameToken = parser.advance(); // consume identifier
            if (parser.match(TokenType::Equal)) { // consume "="
                std::unique_ptr<Expr> initializer = parser.parseExpression();
                return std::make_unique<VarDecl>(nameToken.text, std::move(initializer));
            }
        }
        // If malformed after "let", skip tokens to avoid infinite loop.
        parser.advance(); 
        return nullptr;
    }
    // Try to parse Assignment (IDENTIFIER = expression)
    if (parser.check(TokenType::Identifier) && parser.checkNext(TokenType::Equal)) {
        Token nameToken = parser.advance(); // consume Identifier
        parser.advance(); // consume Equal
        std::unique_ptr<Expr> value = parser.parseExpression();
        return std::make_unique<Assign>(nameToken.text, std::move(value));
    }
    // Try to parse Block
    if (parser.check(TokenType::LeftBrace)) {
        parser.advance(); // consume {
        std::vector<std::unique_ptr<Stmt>> blockStatements;
        while (!parser.check(TokenType::RightBrace) && !parser.isAtEnd()) {
            if (auto stmt = parseDeclaration(parser)) { // Recursively parse statements within block
                blockStatements.push_back(std::move(stmt));
            } else {
                parser.advance(); // Skip unparsed token
            }
        }
        // If EOF reached before '}', for minimal just return what we have.
        if (!parser.match(TokenType::RightBrace)) { // consume }
            // Error, but for minimal, we proceed
        }
        return std::make_unique<Block>(std::move(blockStatements));
    }
    // Default to ExpressionStmt
    std::unique_ptr<Expr> expr = parser.parseExpression();
    if (expr) {
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }
    // If parseExpression returns nullptr, advance to avoid infinite loop
    parser.advance();
    return nullptr;
}
std::vector<std::unique_ptr<Stmt>> parseStatements(Parser& parser) {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!parser.isAtEnd()) {
        if (auto stmt = parseDeclaration(parser)) {
            statements.push_back(std::move(stmt));
        } else {
            // parseDeclaration already handles error/skipping.
        }
    }
    return statements;
}
}
