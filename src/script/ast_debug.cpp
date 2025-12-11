#include "ast_debug.hpp"
#include <sstream>

namespace yuki {

std::string printExpr(const Expr* expr) {
    if (!expr) return "nil";

    switch (expr->getKind()) {
        case ExprKind::Literal: {
            const auto* l = static_cast<const Literal*>(expr);
            return l->value;
        }
        case ExprKind::VarExpr: { // Renamed from Variable
            const auto* v = static_cast<const VarExpr*>(expr); // Renamed type
            return v->name;
        }
        case ExprKind::AssignExpr: { // Renamed from Assign
            const auto* a = static_cast<const AssignExpr*>(expr); // Renamed type
            return "(" + a->name + " = " + printExpr(a->value.get()) + ")";
        }
        case ExprKind::Unary: {
            const auto* u = static_cast<const Unary*>(expr);
            return "(" + u->op.lexeme + printExpr(u->right.get()) + ")";
        }
        case ExprKind::Binary: {
            const auto* b = static_cast<const Binary*>(expr);
            return "(" + printExpr(b->left.get()) + " " + b->op.lexeme + " " + printExpr(b->right.get()) + ")";
        }
        case ExprKind::Call: {
            const auto* c = static_cast<const Call*>(expr);
            std::stringstream ss;
            ss << printExpr(c->callee.get()) << "(";
            for (size_t i = 0; i < c->arguments.size(); ++i) {
                ss << printExpr(c->arguments[i].get());
                if (i < c->arguments.size() - 1) ss << ", ";
            }
            ss << ")";
            return ss.str();
        }
        default:
            return "UnknownExpr";
    }
}

std::string printStmt(const Stmt* stmt) {
    if (!stmt) return "";

    switch (stmt->getKind()) {
        case StmtKind::Expression: {
            const auto* e = static_cast<const ExpressionStmt*>(stmt);
            return printExpr(e->expression.get()) + ";";
        }
        case StmtKind::VarDecl: {
            const auto* v = static_cast<const VarDecl*>(stmt);
            std::string s = "var " + v->name;
            if (v->initializer) {
                s += " = " + printExpr(v->initializer.get());
            }
            return s + ";";
        }
        case StmtKind::Block: {
            const auto* b = static_cast<const Block*>(stmt);
            std::string s = "{ ";
            for (const auto& st : b->statements) {
                s += printStmt(st.get()) + " ";
            }
            s += "}";
            return s;
        }
        case StmtKind::Function: {
            const auto* f = static_cast<const FunctionDecl*>(stmt);
            std::string s = "fn " + f->name + "(";
            for (size_t i = 0; i < f->parameters.size(); ++i) {
                s += f->parameters[i];
                if (i < f->parameters.size() - 1) s += ", ";
            }
            s += ") " + printStmt(f->body.get());
            return s;
        }
        case StmtKind::Return: {
            const auto* r = static_cast<const ReturnStmt*>(stmt);
            std::string s = "return";
            if (r->value) {
                s += " " + printExpr(r->value.get());
            }
            return s + ";";
        }
        case StmtKind::If: {
            const auto* i = static_cast<const IfStmt*>(stmt);
            std::string s = "if (" + printExpr(i->condition.get()) + ") " + printStmt(i->thenBranch.get());
            if (i->elseBranch) {
                s += " else " + printStmt(i->elseBranch.get());
            }
            return s;
        }
        case StmtKind::While: {
            const auto* w = static_cast<const WhileStmt*>(stmt);
            return "while (" + printExpr(w->condition.get()) + ") " + printStmt(w->body.get());
        }
        default:
            return "UnknownStmt";
    }
}

}
