#include "interpreter.hpp"
#include <cstdlib>
namespace yuki {
Interpreter::Interpreter() {
    builtins["print"] = builtinPrint;
}
Value Interpreter::evalExpr(const Expr* expr) {
    if (!expr) return Value::nilVal();
    if (const auto* l = dynamic_cast<const Literal*>(expr)) {
        char* end;
        double d = std::strtod(l->value.c_str(), &end);
        if (end != l->value.c_str() && *end == '\0') {
            return Value::numberVal(d);
        }
        return Value::stringVal(l->value);
    }
    if (const auto* i = dynamic_cast<const Identifier*>(expr)) {
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
        return Value::nilVal();
    }
    if (const auto* c = dynamic_cast<const Call*>(expr)) {
        if (const auto* i = dynamic_cast<const Identifier*>(c->callee.get())) {
            if (builtins.count(i->name)) {
                std::vector<Value> args;
                for (const auto& arg : c->arguments) {
                    args.push_back(evalExpr(arg.get()));
                }
                return builtins[i->name](args);
            }
        }
        return Value::nilVal();
    }
    return Value::nilVal();
}
}
