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
    // Variable scope stack, top is the current scope
    std::vector<std::unordered_map<std::string, Value>> variable_stack{ { } };
    // Store function definitions
    std::unordered_map<std::string, FuncDefStmt*> functions;
    // List of loaded modules to prevent circular imports
    std::set<std::string> loaded_modules;
    // Variable lookup
    Value get_variable(const std::string& name) const;
    // Variable assignment
    void set_variable(const std::string& name, const Value& val);
    // Enter/exit scope
    void push_scope();
    void pop_scope();
    // Load and execute module
    bool load_module(const std::string& module_name);
};
