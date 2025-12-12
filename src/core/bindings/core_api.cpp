#include "core_api.hpp"
#include "state.hpp"
#include "../window.hpp"
#include "../log.hpp"
#include "../../script/yuki_script_loader.hpp"
#include "../../script/token.hpp"
#include "../../script/parser.hpp"
#include "../../script/ast.hpp"
#include "../../script/interpreter.hpp"
#include <GLFW/glfw3.h>
#include <filesystem>

namespace yuki {
namespace {
BindingsState& st = bindingsState();
}

Value apiLog(const std::vector<Value>& args) {
    std::string msg;
    for (size_t i = 0; i < args.size(); i++) {
        msg += args[i].toString();
    }
    printf("[INFO] %s\n", msg.c_str());
    return Value::nilVal();
}
Value apiImport(const std::vector<Value>& args) {
    if (args.empty() || !st.interpreter) return Value::nilVal();
    std::filesystem::path p(args[0].toString());
    if (p.extension().empty()) p.replace_extension(".ys");
    std::vector<std::filesystem::path> candidates;
    if (p.is_absolute()) candidates.push_back(p);
    else {
        for (const auto& base : st.importPaths) candidates.push_back(base / p);
        candidates.push_back(p);
        std::filesystem::path modulesDir = std::filesystem::path("scripts/modules") / p;
        candidates.push_back(modulesDir);
    }
    std::filesystem::path found;
    for (const auto& cand : candidates) {
        if (std::filesystem::exists(cand)) { found = cand; break; }
    }
    if (found.empty()) {
        logError("Import path not found: " + p.string());
        return Value::nilVal();
    }
    std::error_code ec;
    std::filesystem::path canon = std::filesystem::weakly_canonical(found, ec);
    if (ec) canon = found;
    std::string key = canon.string();
    std::optional<std::string> alias;
    if (args.size() > 1 && args[1].isString()) alias = args[1].toString();
    if (st.loadedModules.count(key)) {
        if (alias) {
            auto envVal = st.interpreter->globals->get(*alias);
            if (envVal.has_value()) return envVal.value();
        }
        auto exports = st.interpreter->globals->get("exports");
        if (exports.has_value()) return exports.value();
        return Value::nilVal();
    }
    logInfo("Importing " + p.string());
    ScriptLoader loader(canon.string());
    std::string content = loader.load();
    Tokenizer tokenizer(content);
    std::vector<Token> tokens = tokenizer.scanTokens();
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
    if (parser.hadError()) {
        for (const auto& err : parser.getErrors()) {
            logError(err);
        }
        return Value::nilVal();
    }
    logInfo("Executing module " + p.string());
    st.interpreter->exec(statements);
    if (st.interpreter->hasRuntimeErrors()) {
        for (const auto& err : st.interpreter->getRuntimeErrors()) {
            logError(err);
        }
        return Value::nilVal();
    }
    st.interpreter->retainModule(std::move(statements));
    st.loadedModules.insert(key);
    if (alias) {
        auto envVal = st.interpreter->globals->get(*alias);
        if (envVal.has_value()) return envVal.value();
    }
    auto exports = st.interpreter->globals->get("exports");
    if (exports.has_value()) return exports.value();
    return Value::nilVal();
}
Value apiTime(const std::vector<Value>&) {
    return Value::number(glfwGetTime());
}
Value apiRandom(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValueType::Number) return Value::number(0);
    double max = args[0].numberVal;
    double r = (double(rand()) / double(RAND_MAX)) * max;
    return Value::number(r);
}
Value apiGetScreenSize(const std::vector<Value>&) {
    if (!st.window || !st.renderer) return Value::map({});
    int w = st.renderer->getVirtualWidth();
    int h = st.renderer->getVirtualHeight();
    std::unordered_map<std::string, Value> m;
    m["w"] = Value::number((double)w);
    m["h"] = Value::number((double)h);
    return Value::map(m);
}
} // namespace yuki
