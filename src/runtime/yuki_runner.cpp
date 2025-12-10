#include "yuki_runner.hpp"
#include "../script/yuki_script_loader.hpp"
#include "../core/log.hpp"
#include "../core/time.hpp"
#include "../core/input.hpp"
#include "../script/token.hpp"
#include "../script/parser.hpp"
#include "../script/ast.hpp"
#include "../script/interpreter.hpp"
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
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
    
    Interpreter interpreter;
    
    Value initFn = Value::nilVal();
    Value updateFn = Value::nilVal();
    
    interpreter.exec(statements);
    
    auto initVal = interpreter.env->get("init");
    auto updateVal = interpreter.env->get("update");
    
    if (initVal.has_value()) initFn = initVal.value();
    if (updateVal.has_value()) updateFn = updateVal.value();
    
    if (initFn.isFunction()) {
        std::vector<Value> noArgs;
        interpreter.callFunction(initFn, noArgs);
    }
    
    Time time;
    
    while (!window.shouldClose()) {
        time.update();
        float dt = time.deltaTime();
        
        if (updateFn.isFunction()) {
            std::vector<Value> args;
            args.push_back(Value::number(dt));
            interpreter.callFunction(updateFn, args);
        }
        
        window.pollEvents();
        window.swapBuffers();
    }
}

}
