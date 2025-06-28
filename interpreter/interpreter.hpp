#pragma once
#include "ast.hpp"
#include "value.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <set>
#include <functional>

// 定义一个运行时错误类，供解释器内部使用
class RuntimeError : public std::exception {
public:
    std::string message;
    RuntimeError(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Exception for return statements
class ReturnException : public std::exception {
public:
    Value value;
    ReturnException(const Value& v) : value(v) {}
};

// Exception for break statements
class BreakException : public std::exception {
public:
    BreakException() {}
};

// Exception for continue statements
class ContinueException : public std::exception {
public:
    ContinueException() {}
};

class Interpreter {
public:
    Interpreter() { register_builtin_functions(); }
    void execute(const std::unique_ptr<Statement>& node);
    Value eval(const ASTNode* node);
    // 打印当前作用域中的所有变量
    void printVariables() const;
private:
    // Variable scope stack, top is the current scope
    std::vector<std::unordered_map<std::string, Value>> variable_stack{ { } };    
    // Store function definitions
    std::unordered_map<std::string, FuncDefStmt*> functions;
    // Builtin function type
    using BuiltinFunction = std::function<Value(const std::vector<Value>&)>;
    // Store builtin functions
    std::unordered_map<std::string, BuiltinFunction> builtin_functions;    
    // List of loaded modules to prevent circular imports
    std::set<std::string> loaded_modules;    
    // Store loaded module ASTs to keep function pointers valid
    std::vector<std::unique_ptr<ASTNode>> loaded_module_asts;    // Recursion depth tracking
    int recursion_depth = 0;
    int max_recursion_depth = 100;  // 可变的递归深度限制
    // Variable lookup
    Value get_variable(const std::string& name) const;
    // Variable assignment
    void set_variable(const std::string& name, const Value& val);
    // Enter/exit scope
    void push_scope();
    void pop_scope();
    // Load and execute module
    bool load_module(const std::string& module_name);
    // Register builtin functions
    void register_builtin_functions();
};
