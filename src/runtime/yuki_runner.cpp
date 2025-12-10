#include "yuki_runner.hpp"
#include "yuki_script_loader.hpp"
#include "log.hpp"
#include "time.hpp"
#include "input.hpp"
#include "token.hpp"
#include "token_debug.hpp"
#include "parser.hpp"
#include "script/parser_stmt.hpp"
#include "ast.hpp"
#include "ast_debug.hpp"
#include "interpreter.hpp"
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
namespace yuki {
YukiRunner::YukiRunner(const std::string& scriptPath) : scriptPath(scriptPath) {}
void YukiRunner::run(Window& window) {
    logInfo("Running script: " + scriptPath);
    logInfo("Window: " + window.getTitle() + " (" + std::to_string(window.getWidth()) + "x" + std::to_string(window.getHeight()) + ")");
    ScriptLoader loader(scriptPath);
    std::string content = loader.load();
    logInfo("Loaded script: " + std::to_string(content.size()) + " bytes");
    Tokenizer tokenizer(content);
    std::vector<Token> tokens = tokenizer.scanTokens();
    logInfo("Tokenized " + std::to_string(tokens.size()) + " tokens");
    std::vector<Token> previewTokens;
    for (size_t i = 0; i < tokens.size() && i < 10; ++i) {
        previewTokens.push_back(tokens[i]);
    }
    printTokens(previewTokens);
    if (tokens.size() > 10) {
        logInfo("... and more");
    }
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parseStatements(parser);
    logInfo("Parsed " + std::to_string(statements.size()) + " statements");
    Interpreter interpreter;
    try {
        for (const auto& stmt : statements) {
            interpreter.evalStmt(stmt.get());
        }
    } catch (const ReturnSignal&) {
        // Top-level return ignored
    }
    logInfo("Executed program");
    Time time;
    initInput(window);
    while (!window.shouldClose()) {
        time.update();
        [[maybe_unused]] float dt = time.deltaTime();
        updateInput();
        if (isKeyPressed(GLFW_KEY_SPACE)) {
            logInfo("Space pressed");
        }
        window.clear(0.0f, 0.0f, 0.5f, 1.0f);
        window.swapBuffers();
        window.pollEvents();
    }
}
}
