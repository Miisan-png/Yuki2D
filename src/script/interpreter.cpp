#include "interpreter.hpp"
#include "../core/engine_bindings.hpp"
#include "builtins.hpp"
#include <iostream>
#include <cmath>
#include <memory>

namespace yuki {

Interpreter::Interpreter() : globals(std::make_shared<Environment>(nullptr)), env(globals) {
    registerScriptBuiltins(builtins);
    EngineBindings::registerBuiltins(builtins);
}

Interpreter::~Interpreter() {
    for (auto fn : allocatedFunctions) delete fn;
}

void Interpreter::pushEnv(std::shared_ptr<Environment> newEnv) {
    env = std::move(newEnv);
}

void Interpreter::popEnv() {
    if (env->parent) {
        env = env->parent;
    }
}

bool isTruthy(const Value& v) {
    if (v.isNil()) return false;
    if (v.isBool()) return v.boolVal;
    if (v.isNumber()) return v.numberVal != 0;
    return true;
}

bool isEqual(const Value& a, const Value& b) {
    if (a.type != b.type) return false;
    if (a.isNil()) return true;
    if (a.isNumber()) return std::abs(a.numberVal - b.numberVal) < 1e-9;
    if (a.isBool()) return a.boolVal == b.boolVal;
    if (a.isString()) return a.stringVal == b.stringVal;
    return false;
}

Value Interpreter::callFunction(FunctionValue* fn, const std::vector<Value>& args) {
    if (!fn) return Value::nilVal();

    if (fn->isNative) {
        if (fn->nativeFn) return fn->nativeFn(args);
        return Value::nilVal();
    }

    std::shared_ptr<Environment> closure = std::make_shared<Environment>(fn->closure);
    for (size_t i = 0; i < fn->parameters.size(); ++i) {
        if (i < args.size()) {
            closure->define(fn->parameters[i], args[i]);
        } else {
            closure->define(fn->parameters[i], Value::nilVal());
        }
    }

    Value ret = Value::nilVal();
    std::shared_ptr<Environment> previous = env;
    pushEnv(closure);
    
    try {
        if (fn->body) {
            for (const auto& stmt : fn->body->statements) {
                evalStmt(stmt.get());
            }
        }
    } catch (const ReturnSignal& sig) {
        ret = sig.value;
    }
    
    env = previous;
    return ret;
}

Value Interpreter::callFunction(const Value& fn, const std::vector<Value>& args) {
    if (!fn.isFunction()) return Value::nilVal();
    return callFunction(fn.functionVal, args);
}

Value Interpreter::evalExpr(const Expr* expr) {
    if (!expr) return Value::nilVal();

    switch (expr->getKind()) {
        case ExprKind::Literal: {
            const auto* l = static_cast<const Literal*>(expr);
            char* end;
            double d = std::strtod(l->value.c_str(), &end);
            if (end != l->value.c_str() && *end == '\0') return Value::number(d);
            if (l->value == "true") return Value::boolean(true);
            if (l->value == "false") return Value::boolean(false);
            if (l->value == "nil") return Value::nilVal();
            return Value::string(l->value);
        }
        case ExprKind::VarExpr: {
            const auto* v = static_cast<const VarExpr*>(expr);
            auto val = env->get(v->name);
            if (val.has_value()) return val.value();
            if (builtins.count(v->name)) {
                FunctionValue* fn = new FunctionValue();
                fn->isNative = true;
                fn->name = v->name;
                fn->nativeFn = builtins[v->name];
                allocatedFunctions.push_back(fn);
                return Value::function(fn);
            }
            return Value::nilVal();
        }
        case ExprKind::AssignExpr: {
            const auto* a = static_cast<const AssignExpr*>(expr);
            Value val = evalExpr(a->value.get());
            if (!env->assign(a->name, val)) {
                env->define(a->name, val);
            }
            return val;
        }
        case ExprKind::Unary: {
            const auto* u = static_cast<const Unary*>(expr);
            Value right = evalExpr(u->right.get());
            std::string op = u->op.lexeme;
            if (op == "-") {
                if (right.isNumber()) return Value::number(-right.numberVal);
                return Value::number(0);
            }
            if (op == "!") {
                return Value::boolean(!isTruthy(right));
            }
            return Value::nilVal();
        }
        case ExprKind::Binary: {
            const auto* b = static_cast<const Binary*>(expr);
            std::string op = b->op.lexeme;
            if (op == "and") {
                Value left = evalExpr(b->left.get());
                if (!isTruthy(left)) return left;
                return evalExpr(b->right.get());
            }
            if (op == "or") {
                Value left = evalExpr(b->left.get());
                if (isTruthy(left)) return left;
                return evalExpr(b->right.get());
            }

            Value left = evalExpr(b->left.get());
            Value right = evalExpr(b->right.get());
            
            if (op == "+") {
                if (left.isNumber() && right.isNumber()) return Value::number(left.numberVal + right.numberVal);
                return Value::string(left.toString() + right.toString());
            }
            if (op == "-") return Value::number(left.numberVal - right.numberVal);
            if (op == "*") return Value::number(left.numberVal * right.numberVal);
            if (op == "/") return Value::number(left.numberVal / right.numberVal);
            if (op == "%") return Value::number(std::fmod(left.numberVal, right.numberVal));
            if (op == ">") return Value::boolean(left.numberVal > right.numberVal);
            if (op == ">=") return Value::boolean(left.numberVal >= right.numberVal);
            if (op == "<") return Value::boolean(left.numberVal < right.numberVal);
            if (op == "<=") return Value::boolean(left.numberVal <= right.numberVal);
            if (op == "==") return Value::boolean(isEqual(left, right));
            if (op == "!=") return Value::boolean(!isEqual(left, right));
            return Value::nilVal();
        }
        case ExprKind::Call: {
            const auto* c = static_cast<const Call*>(expr);
            Value callee = evalExpr(c->callee.get());
            std::vector<Value> args;
            for (const auto& arg : c->arguments) {
                args.push_back(evalExpr(arg.get()));
            }
            return callFunction(callee, args);
        }
    }
    return Value::nilVal();
}

Value Interpreter::evalStmt(const Stmt* stmt) {
    if (!stmt) return Value::nilVal();

    switch (stmt->getKind()) {
        case StmtKind::Expression: {
            const auto* es = static_cast<const ExpressionStmt*>(stmt);
            return evalExpr(es->expression.get());
        }
        case StmtKind::VarDecl: {
            const auto* vs = static_cast<const VarDecl*>(stmt);
            Value val = Value::nilVal();
            if (vs->initializer) {
                val = evalExpr(vs->initializer.get());
            }
            env->define(vs->name, val);
            return Value::nilVal();
        }
        case StmtKind::Block: {
            const auto* bs = static_cast<const Block*>(stmt);
            std::shared_ptr<Environment> newEnv = std::make_shared<Environment>(env);
            execBlock(bs, newEnv);
            return Value::nilVal();
        }
        case StmtKind::Function: {
            const auto* fs = static_cast<const FunctionDecl*>(stmt);
            FunctionValue* fn = new FunctionValue();
            fn->isNative = false;
            fn->name = fs->name;
            fn->parameters = fs->parameters;
            fn->body = fs->body.get();
            fn->closure = env;
            allocatedFunctions.push_back(fn);
            env->define(fs->name, Value::function(fn));
            return Value::nilVal();
        }
        case StmtKind::Return: {
            const auto* rs = static_cast<const ReturnStmt*>(stmt);
            Value val = Value::nilVal();
            if (rs->value) {
                val = evalExpr(rs->value.get());
            }
            throw ReturnSignal{val};
        }
        case StmtKind::If: {
            const auto* is = static_cast<const IfStmt*>(stmt);
            if (isTruthy(evalExpr(is->condition.get()))) {
                evalStmt(is->thenBranch.get());
            } else if (is->elseBranch) {
                evalStmt(is->elseBranch.get());
            }
            return Value::nilVal();
        }
        case StmtKind::While: {
            const auto* ws = static_cast<const WhileStmt*>(stmt);
            while (isTruthy(evalExpr(ws->condition.get()))) {
                evalStmt(ws->body.get());
            }
            return Value::nilVal();
        }
    }
    return Value::nilVal();
}

Value Interpreter::execBlock(const Block* block, std::shared_ptr<Environment> newEnv) {
    std::shared_ptr<Environment> previous = env;
    pushEnv(newEnv);
    try {
        for (const auto& stmt : block->statements) {
            evalStmt(stmt.get());
        }
    } catch (...) {
        env = previous;
        throw;
    }
    env = previous;
    return Value::nilVal();
}

Value Interpreter::exec(const std::vector<std::unique_ptr<Stmt>>& statements) {
    try {
        for (const auto& stmt : statements) {
            evalStmt(stmt.get());
        }
    } catch (const ReturnSignal& sig) {
        return sig.value;
    }
    return Value::nilVal();
}

}
