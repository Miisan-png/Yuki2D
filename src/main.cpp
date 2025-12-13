#include "window.hpp"
#include "yuki_runner.hpp"
#include "log.hpp"
#include "config.hpp"
#include "engine_bindings.hpp"
#include "../script/yuki_script_loader.hpp"
#include "../script/token.hpp"
#include "../script/parser.hpp"
#include "../script/ast.hpp"
#include "../script/interpreter.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace {
int headlessRun(const std::string& scriptPath, bool execute) {
    yuki::ScriptLoader loader(scriptPath);
    std::string content = loader.load();
    if (content.empty()) {
        yuki::logError("Failed to load script: " + scriptPath);
        return 1;
    }
    yuki::Tokenizer tokenizer(content);
    std::vector<yuki::Token> tokens = tokenizer.scanTokens();
    if (tokenizer.hadError()) {
        for (const auto& err : tokenizer.getErrors()) {
            yuki::logError(err);
        }
        return 1;
    }
    yuki::Parser parser(tokens);
    std::vector<std::unique_ptr<yuki::Stmt>> statements = parser.parse();
    if (parser.hadError()) {
        for (const auto& err : parser.getErrors()) {
            yuki::logError(err);
        }
        return 1;
    }
    if (!execute) return 0;
    yuki::Interpreter interpreter;
    yuki::EngineBindings::init(nullptr, nullptr, &interpreter);
    std::filesystem::path scriptDir = std::filesystem::path(scriptPath).parent_path();
    yuki::EngineBindings::setAssetBase(scriptDir.string());
    interpreter.exec(statements);
    interpreter.retainModule(std::move(statements));
    if (interpreter.hasRuntimeErrors()) return 1;
    yuki::Value initFn = yuki::Value::nilVal();
    auto initVal = interpreter.env->get("init");
    if (initVal.has_value()) initFn = initVal.value();
    if (initFn.isFunction()) {
        std::vector<yuki::Value> noArgs;
        interpreter.callFunction(initFn, noArgs);
    }
    return interpreter.hasRuntimeErrors() ? 1 : 0;
}

int headlessSimulate(const std::string& scriptPath, int steps, double dt) {
    yuki::ScriptLoader loader(scriptPath);
    std::string content = loader.load();
    if (content.empty()) {
        yuki::logError("Failed to load script: " + scriptPath);
        return 1;
    }
    yuki::Tokenizer tokenizer(content);
    std::vector<yuki::Token> tokens = tokenizer.scanTokens();
    if (tokenizer.hadError()) {
        for (const auto& err : tokenizer.getErrors()) {
            yuki::logError(err);
        }
        return 1;
    }
    yuki::Parser parser(tokens);
    std::vector<std::unique_ptr<yuki::Stmt>> statements = parser.parse();
    if (parser.hadError()) {
        for (const auto& err : parser.getErrors()) {
            yuki::logError(err);
        }
        return 1;
    }
    yuki::Interpreter interpreter;
    yuki::EngineBindings::init(nullptr, nullptr, &interpreter);
    std::filesystem::path scriptDir = std::filesystem::path(scriptPath).parent_path();
    yuki::EngineBindings::setAssetBase(scriptDir.string());
    interpreter.exec(statements);
    interpreter.retainModule(std::move(statements));
    if (interpreter.hasRuntimeErrors()) return 1;

    yuki::Value initFn = yuki::Value::nilVal();
    yuki::Value updateFn = yuki::Value::nilVal();
    auto initVal = interpreter.env->get("init");
    auto updateVal = interpreter.env->get("update");
    if (initVal.has_value()) initFn = initVal.value();
    if (updateVal.has_value()) updateFn = updateVal.value();
    if (initFn.isFunction()) {
        std::vector<yuki::Value> noArgs;
        interpreter.callFunction(initFn, noArgs);
        if (interpreter.hasRuntimeErrors()) return 1;
    }

    for (int i = 0; i < steps; i++) {
        if (updateFn.isFunction()) {
            std::vector<yuki::Value> args;
            args.push_back(yuki::Value::number(dt));
            interpreter.callFunction(updateFn, args);
            if (interpreter.hasRuntimeErrors()) return 1;
        }
        yuki::EngineBindings::update(dt);
        if (interpreter.hasRuntimeErrors()) return 1;
    }
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    std::string scriptPath = "scripts/example_main.ys";
    if (argc > 1 && std::string(argv[1]) == "--check") {
        if (argc < 3) {
            yuki::logError("Usage: yuki2d --check <script.ys>");
            return 2;
        }
        return headlessRun(argv[2], false);
    }
    if (argc > 1 && std::string(argv[1]) == "--run") {
        if (argc < 3) {
            yuki::logError("Usage: yuki2d --run <script.ys>");
            return 2;
        }
        return headlessRun(argv[2], true);
    }
    if (argc > 1 && std::string(argv[1]) == "--simulate") {
        if (argc < 4) {
            yuki::logError("Usage: yuki2d --simulate <script.ys> <steps> [dt]");
            return 2;
        }
        int steps = std::stoi(argv[3]);
        double dt = 1.0 / 60.0;
        if (argc >= 5) dt = std::stod(argv[4]);
        return headlessSimulate(argv[2], steps, dt);
    }
    if (argc > 1) {
        scriptPath = argv[1];
    }
    yuki::logInfo("Engine starting");
    yuki::logInfo("Using script: " + scriptPath);
    yuki::logInfo("Time system initialized");
    yuki::EngineConfig config;
    yuki::Window window(config.width, config.height, config.title);
    yuki::YukiRunner runner(scriptPath);
    runner.run(window);
    return 0;
}
