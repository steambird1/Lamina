#include "interpreter.hpp"
#include <iostream>

void Interpreter::execute(const std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    if (auto* p = dynamic_cast<PrintNode*>(node.get())) {
        Value val = eval(p->expr.get());
        std::cout << val.to_string() << std::endl;
    } else if (auto* v = dynamic_cast<VarDeclNode*>(node.get())) {
        Value val = eval(v->expr.get());
        variables[v->name] = val;
    }
}

Value Interpreter::eval(const ASTNode* node) {
    if (auto* lit = dynamic_cast<const LiteralNode*>(node)) {
        // 判断是否为数字
        try {
            int i = std::stoi(lit->value);
            return Value(i);
        } catch (...) {
            return Value(lit->value);
        }
    } else if (auto* id = dynamic_cast<const IdentifierNode*>(node)) {
        auto it = variables.find(id->name);
        if (it != variables.end()) return it->second;
        return Value("<undefined>");
    }
    return Value("<error>");
}
