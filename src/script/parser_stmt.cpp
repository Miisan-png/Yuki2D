#include "script/parser_stmt.hpp"
namespace yuki {
std::unique_ptr<Stmt> parseDeclaration(Parser& parser) {
    if (parser.check(TokenType::Identifier) && parser.peek().text == "fn") {
        parser.advance(); 
        if (parser.check(TokenType::Identifier)) {
            std::string funcName = parser.advance().text;
            if (parser.match(TokenType::LeftParen)) {
                std::vector<std::string> params;
                if (!parser.check(TokenType::RightParen)) {
                    do {
                        if (parser.check(TokenType::Identifier)) {
                            params.push_back(parser.advance().text);
                        }
                    } while (parser.match(TokenType::Comma));
                }
                parser.match(TokenType::RightParen);
                if (parser.check(TokenType::LeftBrace)) {
                    parser.advance();
                    std::vector<std::unique_ptr<Stmt>> bodyStmts;
                    while (!parser.check(TokenType::RightBrace) && !parser.isAtEnd()) {
                        if (auto stmt = parseDeclaration(parser)) {
                            bodyStmts.push_back(std::move(stmt));
                        } else {
                            parser.advance();
                        }
                    }
                    parser.match(TokenType::RightBrace);
                    auto block = std::make_unique<Block>(std::move(bodyStmts));
                    return std::make_unique<FunctionDecl>(funcName, std::move(params), std::move(block));
                }
            }
        }
    }
    if (parser.check(TokenType::Identifier) && parser.peek().text == "if") {
        parser.advance();
        if (parser.match(TokenType::LeftParen)) {
            std::unique_ptr<Expr> condition = parser.parseExpression();
            parser.match(TokenType::RightParen);
            std::unique_ptr<Stmt> thenBranch = parseDeclaration(parser);
            std::unique_ptr<Stmt> elseBranch = nullptr;
            if (parser.check(TokenType::Identifier) && parser.peek().text == "else") {
                parser.advance();
                elseBranch = parseDeclaration(parser);
            }
            return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
        }
    }
    if (parser.check(TokenType::Identifier) && parser.peek().text == "while") {
        parser.advance();
        if (parser.match(TokenType::LeftParen)) {
            std::unique_ptr<Expr> condition = parser.parseExpression();
            parser.match(TokenType::RightParen);
            std::unique_ptr<Stmt> body = parseDeclaration(parser);
            return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
        }
    }
    if (parser.check(TokenType::Identifier) && parser.peek().text == "let") {
        parser.advance(); 
        if (parser.check(TokenType::Identifier)) { 
            Token nameToken = parser.advance(); 
            if (parser.match(TokenType::Equal)) { 
                std::unique_ptr<Expr> initializer = parser.parseExpression();
                return std::make_unique<VarDecl>(nameToken.text, std::move(initializer));
            }
        }
        parser.advance(); 
        return nullptr;
    }
    if (parser.check(TokenType::Identifier) && parser.peek().text == "return") {
        parser.advance();
        std::unique_ptr<Expr> value = nullptr;
        if (!parser.check(TokenType::Semicolon)) { 
            value = parser.parseExpression();
        }
        return std::make_unique<ReturnStmt>(std::move(value));
    }
    if (parser.check(TokenType::Identifier) && parser.checkNext(TokenType::Equal)) {
        Token nameToken = parser.advance(); 
        parser.advance(); 
        std::unique_ptr<Expr> value = parser.parseExpression();
        return std::make_unique<Assign>(nameToken.text, std::move(value));
    }
    if (parser.check(TokenType::LeftBrace)) {
        parser.advance(); 
        std::vector<std::unique_ptr<Stmt>> blockStatements;
        while (!parser.check(TokenType::RightBrace) && !parser.isAtEnd()) {
            if (auto stmt = parseDeclaration(parser)) { 
                blockStatements.push_back(std::move(stmt));
            } else {
                parser.advance(); 
            }
        }
        if (!parser.match(TokenType::RightBrace)) { 
        }
        return std::make_unique<Block>(std::move(blockStatements));
    }
    std::unique_ptr<Expr> expr = parser.parseExpression();
    if (expr) {
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }
    parser.advance();
    return nullptr;
}
std::vector<std::unique_ptr<Stmt>> parseStatements(Parser& parser) {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!parser.isAtEnd()) {
        if (auto stmt = parseDeclaration(parser)) {
            statements.push_back(std::move(stmt));
        } else {
        }
    }
    return statements;
}
}