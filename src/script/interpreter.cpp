#include "interpreter.hpp"
#include "../core/engine_bindings.hpp"
#include <cstdlib>
namespace yuki {
Interpreter::Interpreter() {
    globals = new Environment();
    current = globals;
    builtins["print"] = builtinPrint;
    EngineBindings::registerBuiltins(builtins);
}
Interpreter::~Interpreter() {
    while (current != globals) {
        Environment* temp = current;
        current = nullptr; 
        delete temp;
    }
    delete globals;
}
bool isTruthy(const Value& v) {
    if (v.type == ValueType::Nil) return false;
    if (v.type == ValueType::String && v.text == "false") return false;
    if (v.type == ValueType::Number && v.number == 0) return false;
    return true;
}
bool isEqual(const Value& a, const Value& b) {
    if (a.type != b.type) return false;
    if (a.type == ValueType::Nil) return true;
    if (a.type == ValueType::Number) return a.number == b.number;
    if (a.type == ValueType::String) return a.text == b.text;
    return false;
}
Value Interpreter::evalExpr(const Expr* expr) {
    if (!expr) return Value::nilVal();
    if (const auto* l = dynamic_cast<const Literal*>(expr)) {
        if (l->value == "true") return Value::stringVal("true"); 
        if (l->value == "false") return Value::stringVal("false");
        char* end;
        double d = std::strtod(l->value.c_str(), &end);
        if (end != l->value.c_str() && *end == '\0') {
            return Value::numberVal(d);
        }
        return Value::stringVal(l->value);
    }
    if (const auto* i = dynamic_cast<const Identifier*>(expr)) {
        Value v = current->get(i->name);
        if (v.type != ValueType::Nil) return v;
        if (builtins.count(i->name)) {
            return Value::numberVal(0); 
        }
        return Value::nilVal();
    }
    if (const auto* b = dynamic_cast<const Binary*>(expr)) {
        Value left = evalExpr(b->left.get());
        Value right = evalExpr(b->right.get());
        if (b->op.type == TokenType::Plus) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return Value::numberVal(left.number + right.number);
            }
            if (left.type == ValueType::String && right.type == ValueType::String) {
                return Value::stringVal(left.text + right.text);
            }
        }
        if (b->op.type == TokenType::Minus) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return Value::numberVal(left.number - right.number);
            }
        }
        if (b->op.type == TokenType::Star) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return Value::numberVal(left.number * right.number);
            }
        }
        if (b->op.type == TokenType::Slash) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return Value::numberVal(left.number / right.number);
            }
        }
        if (b->op.type == TokenType::Greater) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return left.number > right.number ? Value::stringVal("true") : Value::stringVal("false");
            }
        }
        if (b->op.type == TokenType::GreaterEqual) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return left.number >= right.number ? Value::stringVal("true") : Value::stringVal("false");
            }
        }
        if (b->op.type == TokenType::Less) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return left.number < right.number ? Value::stringVal("true") : Value::stringVal("false");
            }
        }
        if (b->op.type == TokenType::LessEqual) {
            if (left.type == ValueType::Number && right.type == ValueType::Number) {
                return left.number <= right.number ? Value::stringVal("true") : Value::stringVal("false");
            }
        }
        if (b->op.type == TokenType::EqualEqual) {
            return isEqual(left, right) ? Value::stringVal("true") : Value::stringVal("false");
        }
        if (b->op.type == TokenType::BangEqual) {
            return !isEqual(left, right) ? Value::stringVal("true") : Value::stringVal("false");
        }
        return Value::nilVal();
    }
    if (const auto* c = dynamic_cast<const Call*>(expr)) {
        Value callee = evalExpr(c->callee.get());
        std::vector<Value> args;
        for (const auto& arg : c->arguments) {
            args.push_back(evalExpr(arg.get()));
        }
        if (callee.type == ValueType::Function) {
            FunctionValue* fn = callee.function;
            if (args.size() != fn->parameters.size()) return Value::nilVal();
            Environment* closure = new Environment(fn->closure);
            for (size_t i = 0; i < args.size(); ++i) {
                closure->define(fn->parameters[i], args[i]);
            }
            Value ret = Value::nilVal();
            try {
                execBlock(fn->body, closure);
            } catch (const ReturnSignal& sig) {
                ret = sig.value;
            }
            delete closure;
            return ret;
        }
        if (const auto* i = dynamic_cast<const Identifier*>(c->callee.get())) {
            if (builtins.count(i->name)) {
                return builtins[i->name](args);
            }
        }
        return Value::nilVal();
    }
    return Value::nilVal();
}
Value Interpreter::evalStmt(const Stmt* stmt) {
    if (const auto* estmt = dynamic_cast<const ExpressionStmt*>(stmt)) {
        return evalExpr(estmt->expression.get());
    }
    if (const auto* varDecl = dynamic_cast<const VarDecl*>(stmt)) {
        Value val = Value::nilVal();
        if (varDecl->initializer) {
            val = evalExpr(varDecl->initializer.get());
        }
        current->define(varDecl->name, val);
        return Value::nilVal();
    }
    if (const auto* assign = dynamic_cast<const Assign*>(stmt)) {
        Value val = evalExpr(assign->value.get());
        current->assign(assign->name, val);
        return val;
    }
    if (const auto* block = dynamic_cast<const Block*>(stmt)) {
        Environment* newEnv = new Environment(current);
        execBlock(block, newEnv);
        delete newEnv;
        return Value::nilVal();
    }
    if (const auto* funcDecl = dynamic_cast<const FunctionDecl*>(stmt)) {
        FunctionValue* fn = new FunctionValue();
        fn->parameters = funcDecl->parameters;
        fn->body = funcDecl->body.get();
        fn->closure = current;
        current->define(funcDecl->name, Value::functionVal(fn));
        return Value::nilVal();
    }
    if (const auto* retStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        Value val = Value::nilVal();
        if (retStmt->value) {
            val = evalExpr(retStmt->value.get());
        }
        throw ReturnSignal{val};
    }
    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
        if (isTruthy(evalExpr(ifStmt->condition.get()))) {
            evalStmt(ifStmt->thenBranch.get());
        } else if (ifStmt->elseBranch) {
            evalStmt(ifStmt->elseBranch.get());
        }
        return Value::nilVal();
    }
    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
        while (isTruthy(evalExpr(whileStmt->condition.get()))) {
            evalStmt(whileStmt->body.get());
        }
        return Value::nilVal();
    }
    return Value::nilVal();
}
Value Interpreter::execBlock(const Block* block, Environment* newEnv) {
    Environment* previous = current;
    current = newEnv;
    for (const auto& stmt : block->statements) {
        evalStmt(stmt.get());
    }
    current = previous;
    return Value::nilVal();
}
}
