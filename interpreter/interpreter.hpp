#pragma once
#include "ast.hpp"
#include "value.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <set>

class Interpreter {
public:
    void execute(const std::unique_ptr<Statement>& node);
    Value eval(const ASTNode* node);
private:
    // 变量作用域栈，栈顶为当前作用域
    std::vector<std::unordered_map<std::string, Value>> variable_stack{ { } };
    // 存储函数定义
    std::unordered_map<std::string, FuncDefStmt*> functions;
    // 已加载模块列表，防止循环导入
    std::set<std::string> loaded_modules;
    // 变量查找
    Value get_variable(const std::string& name) const;
    // 变量赋值
    void set_variable(const std::string& name, const Value& val);
    // 进入/退出作用域
    void push_scope();
    void pop_scope();
    // 加载并执行模块
    bool load_module(const std::string& module_name);
};
