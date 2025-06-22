#pragma once
#include "ast.hpp"
#include "value.hpp"
#include <unordered_map>
#include <string>
#include <memory>

class Interpreter {
public:
    void execute(const std::unique_ptr<Statement>& node);
    Value eval(const ASTNode* node);
private:
    std::unordered_map<std::string, Value> variables;
    // 存储函数定义
    std::unordered_map<std::string, FuncDefStmt*> functions;
};
