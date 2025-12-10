#pragma once
#include <unordered_map>
#include <string>
#include <vector>
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
private:
    Environment* globals;
    Environment* current;
    std::unordered_map<std::string, NativeFn> builtins;
};
}
