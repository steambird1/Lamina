#include "interpreter.hpp"
#include <iostream>
#include <cmath> // 添加 cmath 头文件支持数学函数

void Interpreter::execute(const std::unique_ptr<Statement>& node) {
    if (!node) return;
    if (auto* p = dynamic_cast<PrintStmt*>(node.get())) {
        Value val = eval(p->expr.get());
        std::cout << val.to_string() << std::endl;
    } else if (auto* v = dynamic_cast<VarDeclStmt*>(node.get())) {
        Value val = eval(v->expr.get());
        variables[v->name] = val;
    } else if (auto* ifs = dynamic_cast<IfStmt*>(node.get())) {
        Value cond = eval(ifs->condition.get());
        bool cond_true = cond.type == Value::Type::Int ? std::get<int>(cond.data) != 0 : false;
        if (cond_true) {
            for (auto& stmt : ifs->thenBlock->statements) execute(stmt);
        } else if (ifs->elseBlock) {
            for (auto& stmt : ifs->elseBlock->statements) execute(stmt);
        }
    } else if (auto* ws = dynamic_cast<WhileStmt*>(node.get())) {
        while (true) {
            Value cond = eval(ws->condition.get());
            bool cond_true = cond.type == Value::Type::Int ? std::get<int>(cond.data) != 0 : false;
            if (!cond_true) break;
            for (auto& stmt : ws->body->statements) execute(stmt);
        }
    } else if (auto* func = dynamic_cast<FuncDefStmt*>(node.get())) {
        functions[func->name] = func;
    } else if (auto* block = dynamic_cast<BlockStmt*>(node.get())) {
        for (auto& stmt : block->statements) execute(stmt);
    }
}

Value Interpreter::eval(const ASTNode* node) {
    if (auto* lit = dynamic_cast<const LiteralExpr*>(node)) {
        // 判断是否为数字
        try {
            int i = std::stoi(lit->value);
            return Value(i);
        } catch (...) {
            return Value(lit->value);
        }
    } else if (auto* id = dynamic_cast<const IdentifierExpr*>(node)) {
        auto it = variables.find(id->name);
        if (it != variables.end()) return it->second;
        return Value("<undefined>");
    } else if (auto* bin = dynamic_cast<const BinaryExpr*>(node)) {
        Value l = eval(bin->left.get());
        Value r = eval(bin->right.get());
        int li = l.type == Value::Type::Int ? std::get<int>(l.data) : 0;
        int ri = r.type == Value::Type::Int ? std::get<int>(r.data) : 0;
        if (bin->op == "+") return Value(li + ri);
        if (bin->op == "-") return Value(li - ri);
        if (bin->op == "*") return Value(li * ri);
        if (bin->op == "/") return Value(ri != 0 ? (double)li / ri : 0); // 支持浮点除法
        if (bin->op == "//") return Value(ri != 0 ? li / ri : 0);
        if (bin->op == "%") return Value(ri != 0 ? li % ri : 0);
        if (bin->op == "^") {
            int res = 1;
            for (int j = 0; j < ri; ++j) res *= li;
            return Value(res);
        }
        return Value("<binop error>");
    } else if (auto* unary = dynamic_cast<const UnaryExpr*>(node)) {
        Value v = eval(unary->operand.get());
        int vi = v.type == Value::Type::Int ? std::get<int>(v.data) : 0;
        if (unary->op == "-") return Value(-vi);
        if (unary->op == "!") {
            int res = 1;
            for (int j = 1; j <= vi; ++j) res *= j;
            return Value(res);
        }
        return Value("<unary error>");
    }
    // 支持函数调用
    else if (auto* call = dynamic_cast<const CallExpr*>(node)) {
        auto it = functions.find(call->callee);
        if (it != functions.end()) {
            FuncDefStmt* func = it->second;
            Interpreter local;
            // 传参
            for (size_t j = 0; j < func->params.size(); ++j) {
                if (j < call->args.size())
                    local.variables[func->params[j]] = eval(call->args[j].get());
            }
            // 执行函数体
            for (const auto& stmt : func->body->statements) local.execute(stmt);
            // 暂不支持 return 返回值
            return Value();
        }
        return Value("<call error>");
    }
    return Value("<error>");
}
