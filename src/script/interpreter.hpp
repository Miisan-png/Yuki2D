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

class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    Value evalExpr(const Expr* expr);
    Value evalStmt(const Stmt* stmt);
    Value execBlock(const Block* block, Environment* newEnv);
    Value callFunction(FunctionValue* fn, const std::vector<Value>& args);
    Value callFunction(const Value& fn, const std::vector<Value>& args);
    Value exec(const std::vector<std::unique_ptr<Stmt>>& statements);

    Environment* globals;
    Environment* env; // Current environment

private:
    std::unordered_map<std::string, NativeFn> builtins;
    
    // To manage memory of created FunctionValues if they are dynamic.
    // In this simple engine, we might leak them or AST owns them.
    // Native functions are static. 
    // Script functions are created in FunctionDecl. We should track them or leak them.
    // For now, we will let them leak or rely on AST ownership if designed so.
    // But FunctionValue struct is small. 
    std::vector<FunctionValue*> allocatedFunctions; 
};

}
