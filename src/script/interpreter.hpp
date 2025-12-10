#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "ast.hpp"
#include "value.hpp"
#include "builtins.hpp"
namespace yuki {
class Interpreter {
public:
    Interpreter();
    Value evalExpr(const Expr* expr);
private:
    std::unordered_map<std::string, NativeFn> builtins;
};
}
