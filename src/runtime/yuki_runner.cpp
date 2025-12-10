#include "yuki_runner.hpp"
#include "yuki_script_loader.hpp"
#include "log.hpp"
#include "time.hpp"
#include "input.hpp"
#include "token.hpp"
#include "parser.hpp"
#include "script/parser_stmt.hpp"
#include "ast.hpp"
#include "interpreter.hpp"
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include <optional>
namespace yuki {
YukiRunner::YukiRunner(const std::string& scriptPath) : scriptPath(scriptPath) {}
void YukiRunner::run(Window& window) {
    ScriptLoader loader(scriptPath);
    std::string content = loader.load();
    Tokenizer tokenizer(content);
    std::vector<Token> tokens = tokenizer.scanTokens();
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parseStatements(parser);
    Interpreter interpreter;
    Value initFn = Value::nilVal();
    Value updateFn = Value::nilVal();
    interpreter.exec(statements);
    auto initVal = interpreter.env->get("init");
    auto updateVal = interpreter.env->get("update");
    if (initVal.has_value()) initFn = initVal.value();
    if (updateVal.has_value()) updateFn = updateVal.value();
    if (initFn.isFunction()) {
        interpreter.callFunction(initFn, {});
    }
    Time time;
    while (!window.shouldClose()) {
        time.update();
        float dt = time.deltaTime();
        if (updateFn.isFunction()) {
            interpreter.callFunction(updateFn, { Value::numberVal(dt) });
        }
        window.pollEvents();
        window.swapBuffers();
    }
}
}
