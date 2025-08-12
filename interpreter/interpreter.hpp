#pragma once
#include "lamina.hpp"
#ifdef _WIN32
#ifdef LAMINA_CORE_EXPORTS
#define LAMINA_API __declspec(dllexport)
#else
#define LAMINA_API __declspec(dllimport)
#endif
#else
#define LAMINA_API
#endif
#include "ast.hpp"
#include "module_loader.hpp"
#include "value.hpp"
#include <functional>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

// 前向声明在 lamina.hpp 中已定义，无需重复声明

// Stack frame for function call tracking
struct StackFrame {
    std::string function_name;
    std::string file_name;
    int line_number;

    StackFrame(const std::string& func, const std::string& file, int line)
        : function_name(func), file_name(file), line_number(line) {}
};

// Enhanced runtime error class with stack trace support
class RuntimeError : public std::exception {
public:
    std::string message;
    std::vector<StackFrame> stack_trace;

    RuntimeError(const std::string& msg) : message(msg) {}
    RuntimeError(const std::string& msg, const std::vector<StackFrame>& trace)
        : message(msg), stack_trace(trace) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Exception for return statements
class ReturnException : public std::exception {
public:
    Value value;
    explicit ReturnException(const Value& v) : value(v) {}
};


// Exception for break statements
class BreakException : public std::exception {
public:
    BreakException() = default;
};


// Exception for continue statements
class ContinueException : public std::exception {
public:
    ContinueException() = default;
};

class LAMINA_API Interpreter {
    // 禁止拷贝，允许移动
    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
    Interpreter(Interpreter&&) = default;
    Interpreter& operator=(Interpreter&&) = default;

public:
    Interpreter() {
        register_builtin_functions();
    }
    void execute(const std::unique_ptr<Statement>& node);
    Value eval(const ASTNode* node);
    // Print all variables in current scope
    void printVariables() const;
    void add_function(const std::string& name, FuncDefStmt* func);
    // Save AST in REPL mode to keep function pointers valid
    void save_repl_ast(std::unique_ptr<ASTNode> ast);
    // Stack trace management
    void push_frame(const std::string& function_name, const std::string& file_name = "<script>", int line_number = 0);
    void pop_frame();
    std::vector<StackFrame> get_stack_trace() const;
    void print_stack_trace(const RuntimeError& error, bool use_colors = true) const;

    // Utility functions for error display
    static bool supports_colors();
    static void print_error(const std::string& message, bool use_colors = true);
    static void print_warning(const std::string& message, bool use_colors = true);
    // Builtin function type
    using BuiltinFunction = std::function<Value(const std::vector<Value>&)>;
    // Store builtin functions
    std::unordered_map<std::string, BuiltinFunction> builtin_functions;
    using EntryFunction = void (*)(Interpreter&);
    static void register_entry(EntryFunction func);
    // Variable assignment
    void set_variable(const std::string& name, const Value& val);
    // built global variable in interpreter
    void set_global_variable(const std::string& name, const Value& val);
    // Variable lookup
    Value get_variable(const std::string& name) const;
    // Call module function
    Value call_module_function(const std::string& func_name, const std::vector<Value>& args);
    // Variable scope stack, top is the current scope
    std::vector<std::unordered_map<std::string, Value>> variable_stack{{}};

private:
    // Store function definitions
    std::unordered_map<std::string, FuncDefStmt*> functions;
    // List of loaded modules to prevent circular imports
    std::set<std::string> loaded_modules;
    // Store loaded module ASTs to keep function pointers valid
    std::vector<std::unique_ptr<ASTNode>> loaded_module_asts;
    // Store REPL ASTs to keep function pointers valid in interactive mode
    std::vector<std::unique_ptr<ASTNode>> repl_asts;
    // Store loaded module loaders for function calls
    std::vector<std::unique_ptr<ModuleLoader>> module_loaders;

    // Stack trace for function calls
    std::vector<StackFrame> call_stack;
    // Recursion depth tracking
    int recursion_depth = 0;
    int max_recursion_depth = 100;// 可变的递归深度限制
    // Enter/exit scope
    void push_scope();
    void pop_scope();
    // Load and execute module
    bool load_module(const std::string& module_name);
    // Register builtin functions
    void register_builtin_functions();
    // 移除静态成员变量声明，改用函数内静态变量
    // static std::vector<EntryFunction> entry_functions;
};
