#include "ast_debug.hpp"
#include "ast.hpp"
namespace yuki {
std::string printExpr(const Expr* expr) {
    if (!expr) return "null";
    if (const auto* l = dynamic_cast<const Literal*>(expr)) {
        return "Literal(" + l->value + ")";
    }
    if (const auto* i = dynamic_cast<const Identifier*>(expr)) {
        return "Identifier(" + i->name + ")";
    }
    if (const auto* g = dynamic_cast<const Grouping*>(expr)) {
        return "Grouping(" + printExpr(g->expression.get()) + ")";
    }
    if (const auto* b = dynamic_cast<const Binary*>(expr)) {
        return "Binary(" + printExpr(b->left.get()) + ", " + b->op.text + ", " + printExpr(b->right.get()) + ")";
    }
    if (const auto* c = dynamic_cast<const Call*>(expr)) {
        std::string s = "Call(" + printExpr(c->callee.get());
        for (const auto& arg : c->arguments) {
            s += ", " + printExpr(arg.get());
        }
        s += ")";
        return s;
    }
    return "Unknown";
}
}
