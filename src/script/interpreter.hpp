#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "ast.hpp"
#include "value.hpp"
#include "builtins.hpp"
#include "environment.hpp"

namespace yuki {

struct ReturnSignal {
    Value value;
};

struct BreakSignal {};
struct ContinueSignal {};

class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    Value evalExpr(const Expr* expr);
    Value evalStmt(const Stmt* stmt);
    Value execBlock(const Block* block, std::shared_ptr<Environment> newEnv);
    Value callFunction(FunctionValue* fn, const std::vector<Value>& args);
    Value callFunction(const Value& fn, const std::vector<Value>& args);
    Value exec(const std::vector<std::unique_ptr<Stmt>>& statements);
    void clearRuntimeErrors();
    void runtimeError(const std::string& message);
    void retainModule(std::vector<std::unique_ptr<Stmt>>&& statements);
    bool hasRuntimeErrors() const { return !runtimeErrors.empty(); }
    const std::vector<std::string>& getRuntimeErrors() const { return runtimeErrors; }
    
    void pushEnv(std::shared_ptr<Environment> newEnv);
    void popEnv();

    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> env; // Current environment

private:
    void reportRuntimeError(const std::string& message);

    std::unordered_map<std::string, NativeFn> builtins;
    std::unordered_map<std::string, Value> builtinValueCache;
    std::vector<FunctionValue*> allocatedFunctions;
    std::vector<std::string> runtimeErrors;
    std::vector<std::string> callStack;
    std::vector<std::vector<std::unique_ptr<Stmt>>> ownedModules;
    int functionDepth = 0;
    int loopDepth = 0;
};

}
