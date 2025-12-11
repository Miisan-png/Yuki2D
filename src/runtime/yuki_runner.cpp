#include "yuki_runner.hpp"
#include "../script/yuki_script_loader.hpp"
#include "../core/log.hpp"
#include "../core/time.hpp"
#include "../core/input.hpp"
#include "../core/engine_bindings.hpp"
#include "../core/renderer2d.hpp"
#include "../script/token.hpp"
#include "../script/parser.hpp"
#include "../script/ast.hpp"
#include "../script/interpreter.hpp"
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include <optional>
#include <filesystem>
namespace yuki {
YukiRunner::YukiRunner(const std::string& scriptPath) : scriptPath(scriptPath) {}
void YukiRunner::run(Window& window) {
    if (!window.getNativeWindow()) {
        yuki::logError("Window initialization failed; aborting run loop.");
        return;
    }
    ScriptLoader loader(scriptPath);
    std::string content = loader.load();
    Tokenizer tokenizer(content);
    std::vector<Token> tokens = tokenizer.scanTokens();
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
    if (parser.hadError()) {
        for (const auto& err : parser.getErrors()) {
            logError(err);
        }
        return;
    }
    Interpreter interpreter;
    Renderer2D renderer;
    EngineBindings::init(&window, &renderer, &interpreter);
    std::filesystem::path scriptDir = std::filesystem::path(scriptPath).parent_path();
    EngineBindings::setAssetBase(scriptDir.string());
    initInput(window);
    Value initFn = Value::nilVal();
    Value updateFn = Value::nilVal();
    interpreter.exec(statements);
    interpreter.retainModule(std::move(statements));
    if (interpreter.hasRuntimeErrors()) {
        return;
    }
    auto initVal = interpreter.env->get("init");
    auto updateVal = interpreter.env->get("update");
    if (initVal.has_value()) initFn = initVal.value();
    if (updateVal.has_value()) updateFn = updateVal.value();
    if (initFn.isFunction()) {
        std::vector<Value> noArgs;
        interpreter.callFunction(initFn, noArgs);
    }
    Time time;
    bool debugToggled = renderer.isDebugEnabled();
    while (!window.shouldClose()) {
        time.update();
        float dt = time.deltaTime();
        updateInput(window);
        if (isKeyPressed(GLFW_KEY_F3)) {
            debugToggled = !debugToggled;
            renderer.setDebugEnabled(debugToggled);
        }
        EngineBindings::update(dt);
        window.clear();
        if (updateFn.isFunction()) {
            std::vector<Value> args;
            args.push_back(Value::number(dt));
            interpreter.callFunction(updateFn, args);
        }
        if (interpreter.hasRuntimeErrors()) {
            break;
        }
        int fbW = 0;
        int fbH = 0;
        window.getFramebufferSize(fbW, fbH);
        renderer.flush(fbW, fbH);
        window.swapBuffers();
        window.pollEvents();
    }
}
}
