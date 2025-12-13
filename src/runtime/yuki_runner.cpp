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
#include "dev_console.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <optional>
#include <filesystem>
namespace yuki {
YukiRunner::YukiRunner(const std::string& scriptPath) : scriptPath(scriptPath) {}
YukiRunner::YukiRunner(const std::string& scriptPath, bool watch) : scriptPath(scriptPath), watch(watch) {}
void YukiRunner::run(Window& window) {
    if (!window.getNativeWindow()) {
        yuki::logError("Window initialization failed; aborting run loop.");
        return;
    }

    auto scriptDirAbs = std::filesystem::absolute(std::filesystem::path(scriptPath).parent_path()).lexically_normal();
    auto scriptPathAbs = std::filesystem::absolute(std::filesystem::path(scriptPath)).lexically_normal();

    Renderer2D renderer;
    renderer.setDebugEnabled(false);
    std::unique_ptr<Interpreter> interpreter;
    DevConsole console(&renderer, nullptr);
    initInput(window);
    Value updateFn = Value::nilVal();

    auto parseProgram = [&](std::vector<std::unique_ptr<Stmt>>& out, std::vector<std::string>& outErrs) -> bool {
        ScriptLoader loader(scriptPathAbs.string());
        std::string content = loader.load();
        if (content.empty()) {
            outErrs.push_back("Failed to load script: " + scriptPathAbs.string());
            return false;
        }
        Tokenizer tokenizer(content);
        std::vector<Token> tokens = tokenizer.scanTokens();
        if (tokenizer.hadError()) {
            for (const auto& e : tokenizer.getErrors()) outErrs.push_back(e);
            return false;
        }
        Parser parser(tokens);
        out = parser.parse();
        if (parser.hadError()) {
            for (const auto& e : parser.getErrors()) outErrs.push_back(e);
            return false;
        }
        return true;
    };

    std::unordered_map<std::string, std::filesystem::file_time_type> watchTimes;
    auto refreshWatchTimes = [&]() {
        std::vector<std::string> paths = EngineBindings::getLoadedModulePaths();
        paths.push_back(scriptPathAbs.string());
        for (const auto& p : paths) {
            std::error_code ec;
            auto t = std::filesystem::last_write_time(p, ec);
            if (ec) continue;
            if (watchTimes.find(p) == watchTimes.end()) watchTimes.emplace(p, t);
        }
    };
    auto checkWatchChanged = [&]() -> bool {
        refreshWatchTimes();
        for (auto& kv : watchTimes) {
            std::error_code ec;
            auto t = std::filesystem::last_write_time(kv.first, ec);
            if (ec) continue;
            if (t > kv.second) {
                kv.second = t;
                return true;
            }
        }
        return false;
    };

    auto loadAndInit = [&](bool logSuccess) -> bool {
        std::vector<std::unique_ptr<Stmt>> statements;
        std::vector<std::string> errs;
        if (!parseProgram(statements, errs)) {
            for (const auto& e : errs) {
                logError(e);
                console.log(e);
            }
            return false;
        }

        interpreter = std::make_unique<Interpreter>();
        EngineBindings::init(&window, &renderer, interpreter.get());
        EngineBindings::setAssetBase(scriptDirAbs.string());
        console.setInterpreter(interpreter.get());

        updateFn = Value::nilVal();
        interpreter->exec(statements);
        interpreter->retainModule(std::move(statements));
        if (interpreter->hasRuntimeErrors()) {
            for (const auto& e : interpreter->getRuntimeErrors()) {
                logError(e);
                console.log(e);
            }
            updateFn = Value::nilVal();
            return false;
        }

        auto initVal = interpreter->env->get("init");
        auto updateVal = interpreter->env->get("update");
        if (updateVal.has_value()) updateFn = updateVal.value();
        if (initVal.has_value() && initVal.value().isFunction()) {
            std::vector<Value> noArgs;
            interpreter->callFunction(initVal.value(), noArgs);
        }
        if (interpreter->hasRuntimeErrors()) {
            for (const auto& e : interpreter->getRuntimeErrors()) {
                logError(e);
                console.log(e);
            }
            updateFn = Value::nilVal();
            return false;
        }

        if (watch) {
            watchTimes.clear();
            refreshWatchTimes();
        }
        if (logSuccess) {
            console.log("reloaded");
            logInfo("reloaded");
        }
        return true;
    };

    if (!loadAndInit(false)) return;

    Time time;
    double watchAccum = 0.0;
    while (!window.shouldClose()) {
        time.update();
        float dt = time.deltaTime();
        updateInput(window);
        console.updateInput();

        bool wantReload = false;
        if (watch) {
            watchAccum += dt;
            if (watchAccum >= 0.25) {
                watchAccum = 0.0;
                if (checkWatchChanged()) wantReload = true;
            }
            if (isKeyPressed(GLFW_KEY_F5)) wantReload = true;
        }
        if (wantReload) {
            loadAndInit(true);
        }

        window.clear();
        if (updateFn.isFunction() && interpreter) {
            std::vector<Value> args;
            args.push_back(Value::number(dt));
            interpreter->callFunction(updateFn, args);
        }
        if (interpreter && interpreter->hasRuntimeErrors()) {
            break;
        }
        EngineBindings::update(dt);
        int fbW = 0;
        int fbH = 0;
        window.getFramebufferSize(fbW, fbH);

        renderer.flush(fbW, fbH);
        if (console.isActive()) {
            console.drawOverlay(renderer.getVirtualWidth(), renderer.getVirtualHeight());
            renderer.flush(fbW, fbH, false);
        }
        window.swapBuffers();
        window.pollEvents();
    }
}
}
