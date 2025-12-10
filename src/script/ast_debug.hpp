#pragma once

#include <string>
#include "ast.hpp"

namespace yuki {

std::string printExpr(const Expr* expr);
std::string printStmt(const Stmt* stmt);

}