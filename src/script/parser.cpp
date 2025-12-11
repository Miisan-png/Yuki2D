#include "parser.hpp"
#include <iostream>

namespace yuki {

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        std::unique_ptr<Stmt> stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
    try {
        if (matchKeyword("fn")) return funDecl();
        if (matchKeyword("var")) return varDecl();
        return statement();
    } catch (const std::runtime_error&) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<Stmt> Parser::funDecl() {
    Token name = consume(TokenType::Identifier, "Expect function name.");
    consume(TokenType::LeftParen, "Expect '(' after function name.");
    
    std::vector<std::string> parameters;
    if (!check(TokenType::RightParen)) {
        do {
            parameters.push_back(consume(TokenType::Identifier, "Expect parameter name.").text);
        } while (match({TokenType::Comma}));
    }
    consume(TokenType::RightParen, "Expect ')' after parameters.");
    std::unique_ptr<Block> body = block();
    return std::make_unique<FunctionDecl>(name.text, std::move(parameters), std::move(body));
}

std::unique_ptr<Stmt> Parser::varDecl() {
    Token name = consume(TokenType::Identifier, "Expect variable name.");
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::Equal})) {
        initializer = expression();
    }
    consume(TokenType::Semicolon, "Expect ';' after variable declaration.");
    return std::make_unique<VarDecl>(name.text, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (matchKeyword("if")) return ifStatement();
    if (matchKeyword("while")) return whileStatement();
    if (matchKeyword("for")) return forStatement();
    if (matchKeyword("return")) return returnStatement();
    if (check(TokenType::LeftBrace)) return block(); 
    return exprStmt();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LeftParen, "Expect '(' after 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RightParen, "Expect ')' after if condition.");
    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (matchKeyword("else")) {
        elseBranch = statement();
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LeftParen, "Expect '(' after 'while'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RightParen, "Expect ')' after while condition.");
    std::unique_ptr<Stmt> body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LeftParen, "Expect '(' after 'for'.");

    std::unique_ptr<Stmt> initializer;
    if (match({TokenType::Semicolon})) {
        initializer = nullptr;
    } else if (matchKeyword("var")) {
        initializer = varDecl();
    } else {
        initializer = exprStmt();
    }

    std::unique_ptr<Expr> condition;
    if (!check(TokenType::Semicolon)) {
        condition = expression();
    }
    consume(TokenType::Semicolon, "Expect ';' after loop condition.");

    std::unique_ptr<Expr> increment;
    if (!check(TokenType::RightParen)) {
        increment = expression();
    }
    consume(TokenType::RightParen, "Expect ')' after for clauses.");

    std::unique_ptr<Stmt> body = statement();

    if (increment) {
        std::vector<std::unique_ptr<Stmt>> stmts;
        stmts.push_back(std::move(body));
        stmts.push_back(std::make_unique<ExpressionStmt>(std::move(increment)));
        body = std::make_unique<Block>(std::move(stmts));
    }

    if (!condition) {
        condition = std::make_unique<Literal>("true");
    }
    body = std::make_unique<WhileStmt>(std::move(condition), std::move(body));

    if (initializer) {
        std::vector<std::unique_ptr<Stmt>> stmts;
        stmts.push_back(std::move(initializer));
        stmts.push_back(std::move(body));
        body = std::make_unique<Block>(std::move(stmts));
    }

    return body;
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::Semicolon)) {
        value = expression();
    }
    consume(TokenType::Semicolon, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Block> Parser::block() {
    consume(TokenType::LeftBrace, "Expect '{' to begin block.");
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RightBrace, "Expect '}' after block.");
    return std::make_unique<Block>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::exprStmt() {
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::Semicolon, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = logicOr();

    if (match({TokenType::Equal})) {
        Token equals = previous();
        std::unique_ptr<Expr> value = assignment();

        if (expr->getKind() == ExprKind::VarExpr) {
            std::string name = static_cast<VarExpr*>(expr.get())->name;
            return std::make_unique<AssignExpr>(name, std::move(value));
        }

        error(equals, "Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expr> Parser::logicOr() {
    std::unique_ptr<Expr> expr = logicAnd();
    while (matchKeyword("or")) {
        Token op = previous();
        std::unique_ptr<Expr> right = logicAnd();
        expr = std::make_unique<Binary>(std::move(expr), (int)op.type, op.text, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicAnd() {
    std::unique_ptr<Expr> expr = equality();
    while (matchKeyword("and")) {
        Token op = previous();
        std::unique_ptr<Expr> right = equality();
        expr = std::make_unique<Binary>(std::move(expr), (int)op.type, op.text, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();
    while (match({TokenType::BangEqual, TokenType::EqualEqual})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<Binary>(std::move(expr), (int)op.type, op.text, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();
    while (match({TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<Binary>(std::move(expr), (int)op.type, op.text, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();
    while (match({TokenType::Minus, TokenType::Plus})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<Binary>(std::move(expr), (int)op.type, op.text, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();
    while (match({TokenType::Slash, TokenType::Star, TokenType::Percent})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        expr = std::make_unique<Binary>(std::move(expr), (int)op.type, op.text, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::Bang})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        return std::make_unique<Unary>((int)op.type, op.text, std::move(right));
    }
    if (match({TokenType::Minus})) { 
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        return std::make_unique<Unary>((int)op.type, op.text, std::move(right));
    }
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();
    while (true) {
        if (match({TokenType::LeftParen})) {
            expr = finishCall(std::move(expr));
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> arguments;
    if (!check(TokenType::RightParen)) {
        do {
            arguments.push_back(expression());
        } while (match({TokenType::Comma}));
    }
    consume(TokenType::RightParen, "Expect ')' after arguments.");
    return std::make_unique<Call>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::False})) return std::make_unique<Literal>("false");
    if (match({TokenType::True})) return std::make_unique<Literal>("true");
    if (match({TokenType::Nil})) return std::make_unique<Literal>("nil");
    
    if (match({TokenType::Number, TokenType::String})) {
        return std::make_unique<Literal>(previous().text);
    }

    if (match({TokenType::Identifier})) {
        return std::make_unique<VarExpr>(previous().text);
    }

    if (match({TokenType::LeftParen})) {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::RightParen, "Expect ')' after expression.");
        return expr;
    }

    error(peek(), "Expect expression.");
    throw std::runtime_error("Expect expression.");
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::matchKeyword(const std::string& keyword) {
    if (checkKeyword(keyword)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::checkKeyword(const std::string& keyword) {
    if (isAtEnd()) return false;
    return peek().type == TokenType::Identifier && peek().text == keyword;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() {
    return peek().type == TokenType::Eof;
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(peek(), message);
    throw std::runtime_error(message);
}

void Parser::error(Token token, const std::string& message) {
    std::string loc = "line " + std::to_string(token.line) + ", col " + std::to_string(token.column);
    std::string msg = "[Parser] " + loc + " at '" + token.text + "': " + message;
    errors.push_back(msg);
    std::cerr << msg << std::endl;
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        if (peek().type == TokenType::Identifier) {
            const std::string& t = peek().text;
            if (t == "fn" || t == "var" || t == "if" || t == "while" || t == "return") return;
        }
        advance();
    }
}

}
