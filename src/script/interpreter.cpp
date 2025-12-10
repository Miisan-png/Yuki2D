#include "interpreter.hpp"
#include "../core/engine_bindings.hpp"
#include "builtins.hpp"
#include <iostream>
#include <cmath>

namespace yuki {

Interpreter::Interpreter() : globals(new Environment()), env(globals) {
    // Register script builtins
    registerScriptBuiltins(builtins);
    // Register engine builtins
    EngineBindings::registerBuiltins(builtins);
}

Interpreter::~Interpreter() {
    // Clean up environments
    Environment* currentEnv = env;
    while (currentEnv != globals && currentEnv != nullptr) {
        Environment* temp = currentEnv;
        currentEnv = currentEnv->getParent();
        delete temp;
    }
    if (env != globals) { 
        // If we are deep in stack, we might have skipped cleaning globals if we went up.
        // But simplified: just ensure globals is deleted.
        // Actually, the loop above cleans up strictly upwards from 'env'.
        // If 'env' is globals, loop doesn't run.
    }
    delete globals;
    
    for (auto fn : allocatedFunctions) {
        delete fn;
    }
}

bool isTruthy(const Value& v) {
    if (v.isNil()) return false;
    if (v.isBool()) return v.boolVal;
    if (v.isNumber()) return v.numberVal != 0;
    return true; // Strings, Functions are true
}

bool isEqual(const Value& a, const Value& b) {
    if (a.type != b.type) return false;
    if (a.isNil()) return true;
    if (a.isNumber()) return std::abs(a.numberVal - b.numberVal) < 1e-9;
    if (a.isBool()) return a.boolVal == b.boolVal;
    if (a.isString()) return a.stringVal == b.stringVal;
    return false; // Functions not comparable for equality in this simple version
}

Value Interpreter::callFunction(FunctionValue* fn, const std::vector<Value>& args) {
    if (!fn) return Value::nilVal();

    if (fn->isNative) {
        if (fn->nativeFn) {
            return fn->nativeFn(args);
        }
        return Value::nilVal();
    }

    // Script Function
    if (args.size() != fn->parameters.size()) {
        // We could error or warn here. For now, nil for missing, ignore extra.
    }

    Environment* closure = new Environment(fn->closure);
    for (size_t i = 0; i < fn->parameters.size(); ++i) {
        if (i < args.size()) {
            closure->define(fn->parameters[i], args[i]);
        } else {
            closure->define(fn->parameters[i], Value::nilVal());
        }
    }

    Value ret = Value::nilVal();
    try {
        // We need to switch 'env' to closure during execution
        Environment* previous = env;
        env = closure;
        // Body execution
        if (fn->body) {
            // We reuse execBlock logic but without creating NEW env, using closure
            for (const auto& stmt : fn->body->statements) {
                evalStmt(stmt.get());
            }
        }
        env = previous;
    } catch (const ReturnSignal& sig) {
        ret = sig.value;
        // Ensure we restore environment if exception thrown (though we caught it)
        // If we didn't restore in try block (e.g. exception thrown in evalStmt), we need to handle that.
        // But ReturnSignal is our control flow.
        // If we are here, 'env' is still 'closure' unless we restored it.
        // Actually, we need to restore env in catch block too if we didn't reach end of try.
        // But wait, if exception is thrown, we jump out. 'env' was set to 'closure'.
        // We must restore it.
        // Proper way involves RAII for environment switching, but let's just fix it manually here.
        // Since we are IN the catch, 'env' IS 'closure'.
        // We need to restore it to 'fn->closure' parent? No, to 'previous'.
        // But we lost 'previous' scope? No, 'previous' is local var.
        // Wait, local vars are accessible in catch block.
        // But we need to make sure 'previous' is visible.
    }
    
    // Correction: We need to ensure 'env' is restored.
    // Let's refactor execution to be safe.
    // The issue: 'previous' is declared inside 'try' block? No.
    // Let's rewrite safely.
    
    Environment* previous = env;
    env = closure;
    
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
    delete closure;
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
            // Try parse number
            char* end;
            double d = std::strtod(l->value.c_str(), &end);
            if (end != l->value.c_str() && *end == '\0') {
                return Value::number(d);
            }
            if (l->value == "true") return Value::boolean(true);
            if (l->value == "false") return Value::boolean(false);
            if (l->value == "nil") return Value::nilVal();
            // String literal (assumed already stripped of quotes by parser or is raw)
            // If parser keeps quotes, we should strip them. Assuming parser provides raw string content.
            return Value::string(l->value);
        }
        case ExprKind::Variable: {
            const auto* v = static_cast<const Variable*>(expr);
            auto val = env->get(v->name);
            if (val.has_value()) return val.value();
            // Check builtins if not found in env
            if (builtins.count(v->name)) {
                // Create a native function value wrapper on the fly? 
                // Or builtins should be in globals?
                // Better: put builtins in globals during init.
                // But if we want to fallback:
                FunctionValue* fn = new FunctionValue();
                fn->isNative = true;
                fn->name = v->name;
                fn->nativeFn = builtins[v->name];
                allocatedFunctions.push_back(fn);
                return Value::function(fn);
            }
            return Value::nilVal();
        }
        case ExprKind::Assign: {
            const auto* a = static_cast<const Assign*>(expr);
            Value val = evalExpr(a->value.get());
            env->assign(a->name, val);
            return val;
        }
        case ExprKind::Binary: {
            const auto* b = static_cast<const Binary*>(expr);
            Value left = evalExpr(b->left.get());
            Value right = evalExpr(b->right.get());
            
            // Assuming we have token types available or checking Op
            // For now, let's map lexeme. 
            // In a real engine, use integer types.
            std::string op = b->op.lexeme;
            
            if (op == "+") {
                if (left.isNumber() && right.isNumber()) {
                    return Value::number(left.numberVal + right.numberVal);
                }
                return Value::string(left.toString() + right.toString());
            }
            if (op == "-") return Value::number(left.numberVal - right.numberVal);
            if (op == "*") return Value::number(left.numberVal * right.numberVal);
            if (op == "/") return Value::number(left.numberVal / right.numberVal); // TODO: Div by zero check
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
            Environment* newEnv = new Environment(env);
            execBlock(bs, newEnv);
            delete newEnv;
            return Value::nilVal();
        }
        case StmtKind::Function: {
            const auto* fs = static_cast<const FunctionDecl*>(stmt);
            FunctionValue* fn = new FunctionValue();
            fn->isNative = false;
            fn->name = fs->name;
            fn->parameters = fs->parameters;
            fn->body = fs->body.get(); // Raw pointer, owned by AST
            fn->closure = env; // Capture current env
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

Value Interpreter::execBlock(const Block* block, Environment* newEnv) {
    Environment* previous = env;
    env = newEnv;
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
