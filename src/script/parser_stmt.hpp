#pragma once
#include "parser.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>
namespace yuki {
std::vector<std::unique_ptr<Stmt>> parseStatements(Parser& parser);
}
