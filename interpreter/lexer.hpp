#include "interpreter.hpp"
#include "module.hpp"
#include "lamina.hpp"
#include "parser.hpp"
#include "bigint.hpp"
#include <iostream>
#include <cmath>
#include <exception>
#include <fstream>
#include <sstream>
#include <cstdlib> // For std::exit
#include <cstring> // For strcmp
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#elif __linux__
#include <limits.h>
#include <unistd.h>
#include <unistd.h> // For isatty
#include <cstdlib>  // For getenv
#include <dlfcn.h>
#include <link.h>
#else
#include <limits.h>
#include <unistd.h>
#include <unistd.h> // For isatty
#include <cstdlib>  // For getenv
#include <dlfcn.h>
#endif



// 这些异常类已经移到了 interpreter.hpp

// Scope stack operations
void Interpreter::push_scope() {
    variable_stack.emplace_back();
}

void Interpreter::add_function(const std::string& name, FuncDefStmt* func) {
    functions[name] = func;
}

void Interpreter::pop_scope() {
    if (variable_stack.size() > 1) variable_stack.pop_back();
}

Value Interpreter::get_variable(const std::string& name) const {
    for (auto it = variable_stack.rbegin(); it != variable_stack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }

    // 如果变量找不到，检查是否是函数名
    auto func_it = functions.find(name);
    if (func_it != functions.end()) {
        // 返回一个函数类型的值（这里我们需要扩展Value类型）
        // 暂时返回一个特殊的字符串值表示函数
        return Value("__function_" + name);
    }

    RuntimeError error("Undefined variable '" + name + "'");
    error.stack_trace = get_stack_trace();
    throw error;
    return Value(); // Unreachable, but suppress compiler warning
}

void Interpreter::set_variable(const std::string& name, const Value& val) {
    if (!variable_stack.empty()) {
        (*variable_stack.rbegin())[name] = val;
    }
}

void Interpreter::set_global_variable(const std::string& name, const Value& val) {
    if (!variable_stack.empty()) {
        variable_stack.front()[name] = val;
    }
}

void Interpreter::execute(const std::unique_ptr<Statement>& node) {
    if (!node) return;
    if (auto* v = dynamic_cast<VarDeclStmt*>(node.get())) {        if (v->expr) {
        Value val = eval(v->expr.get());
        set_variable(v->name, val);
    } else {
        RuntimeError error("Variable '" + v->name + "' declaration has null expression");
        error.stack_trace = get_stack_trace();
        throw error;
    }
    }
    else if (auto* d = dynamic_cast<DefineStmt*>(node.get())) {
        if (!d->value) {
            error_and_exit("Null expression in define statement for '" + d->name + "'");
        }
        Value val = eval(d->value.get());
        if (d->name == "MAX_RECURSION_DEPTH" && val.is_int()) {
            int new_depth = std::get<int>(val.data);
            if (new_depth > 0 && new_depth <= 10000) {
                max_recursion_depth = new_depth;
                std::cout << "Recursion depth limit set to: " << new_depth << std::endl;
            } else {
                error_and_exit("Invalid recursion depth value: " + std::to_string(new_depth) + " (must be between 1 and 10000)");
            }
        } else {
            // 原：std::cerr << "Error: Unknown define constant: " << d->name << std::endl;
            // 已替换

        }
    }
    else if (auto* bi = dynamic_cast<BigIntDeclStmt*>(node.get())) {
        if (bi->init_value) {
            Value val = eval(bi->init_value.get());
            if (val.is_bigint()) {
                // 如果值已经是BigInt，直接使用
                set_variable(bi->name, val);
            } else if (val.is_int()) {
                // 将普通整数转换为BigInt
                ::BigInt big_val(std::get<int>(val.data));
                set_variable(bi->name, Value(big_val));
            } else if (val.is_string()) {
                // 从字符串创建BigInt
                ::BigInt big_val(std::get<std::string>(val.data));
                set_variable(bi->name, Value(big_val));
            } else {
                error_and_exit("Cannot convert " + val.to_string() + " to BigInt");
            }
        } else {
            // 默认初始化为0
            set_variable(bi->name, Value(::BigInt(0)));
        }
    }
    else if (auto* a = dynamic_cast<AssignStmt*>(node.get())) {
        if (!a->expr) {
            error_and_exit("Null expression in assignment to '" + a->name + "'");
        }
        Value val = eval(a->expr.get());
        set_variable(a->name, val);
    }    else if (auto* ifs = dynamic_cast<IfStmt*>(node.get())) {
        if (!ifs->condition) {
            error_and_exit("Null condition in if statement");
        }
        Value cond = eval(ifs->condition.get());
        bool cond_true = cond.as_bool();
        if (cond_true) {
            for (auto& stmt : ifs->thenBlock->statements) execute(stmt);
        } else if (ifs->elseBlock) {
            for (auto& stmt : ifs->elseBlock->statements) execute(stmt);
        }
    } else if (auto* ws = dynamic_cast<WhileStmt*>(node.get())) {
        if (!ws->condition) {
            RuntimeError error("Loop condition cannot be null");
            error.stack_trace = get_stack_trace();
            throw error;
        }

        // 更健壮的while循环实现
        int iteration_count = 0;
        const int MAX_ITERATIONS = 100000; // 防止无限循环

        try {
            while (true) {
                // 安全检查：防止无限循环
                if (++iteration_count > MAX_ITERATIONS) {
                    Interpreter::print_warning("Loop exceeded " + std::to_string(MAX_ITERATIONS) +
                                              " iterations, possible infinite loop, terminating", true);
                    RuntimeError error("Possible infinite loop terminated");
                    error.stack_trace = get_stack_trace();
                    throw error;
                }

                // 评估循环条件
                Value cond;
                try {
                    cond = eval(ws->condition.get());
                } catch (const std::exception& e) {
                    // 如果条件计算出错，优雅地退出循环
                    RuntimeError error("Loop condition error: " + std::string(e.what()));
                    error.stack_trace = get_stack_trace();
                    throw error;
                }

                if (!cond.as_bool()) {
                    // 条件为假，退出循环
                    break;
                }

                bool continue_encountered = false;

                // 执行循环体
                for (auto& stmt : ws->body->statements) {
                    if (continue_encountered) continue;

                    try {
                        execute(stmt);
                    } catch (const BreakException&) {
                        // 退出整个循环
                        return;
                    } catch (const ContinueException&) {
                        // 跳到下一次迭代
                        continue_encountered = true;
                    }
                }
            }
        } catch (const BreakException&) {
            // 捕获可能从嵌套块冒泡上来的break异常
            return;
        } catch (const ReturnException&) {
            // 函数返回，直接传递给上层
            throw;
        } catch (const std::exception& e) {
            // 所有其他异常都作为运行时错误处理
            RuntimeError error("Loop body execution error: " + std::string(e.what()));
            error.stack_trace = get_stack_trace();
            throw error;
        }
    } else if (auto* func = dynamic_cast<FuncDefStmt*>(node.get())) {
        functions[func->name] = func;
    }
    else if (auto* block = dynamic_cast<BlockStmt*>(node.get())) {
        for (auto& stmt : block->statements) execute(stmt);
    }
    else if (auto* ret = dynamic_cast<ReturnStmt*>(node.get())) {
        Value val = eval(ret->expr.get());
        throw ReturnException(val);
    }
    else if (auto* breakStmt = dynamic_cast<BreakStmt*>(node.get())) {
        (void)breakStmt; // 避免未使用变量警告
        throw BreakException();
    }
    else if (auto* contStmt = dynamic_cast<ContinueStmt*>(node.get())) {
        (void)contStmt; // 避免未使用变量警告
        throw ContinueException();
    }
    else if (auto* includeStmt = dynamic_cast<IncludeStmt*>(node.get())) {
        if (!load_module(includeStmt->module)) {
            error_and_exit("Failed to include module '" + includeStmt->module + "'");
        }
    } else if (auto* exprstmt = dynamic_cast<ExprStmt*>(node.get())) {
        if (exprstmt->expr) {
            eval(exprstmt->expr.get());
        } else {
            error_and_exit("Empty expression statement");
        }
    }
}


Value Interpreter::eval(const ASTNode* node) {
    if (!node) {
        error_and_exit("Attempted to evaluate null expression");
    }
      if (auto* lit = dynamic_cast<const LiteralExpr*>(node)) {
        // Try to parse as number first
        try {
            // Check if it contains a decimal point for float
            if (lit->value.find('.') != std::string::npos) {
                double d = std::stod(lit->value);
                return Value(d);
            } else {
                int i = std::stoi(lit->value);
                return Value(i);
            }
        } catch (...) {
            // Check for boolean literals
            if (lit->value == "true") return Value(true);
            if (lit->value == "false") return Value(false);
            if (lit->value == "null") return Value(nullptr);
            // Otherwise it's a string
            return Value(lit->value);
        }
    }
    else if (auto* id = dynamic_cast<const IdentifierExpr*>(node)) {
        return get_variable(id->name);
    }
    else if (auto* var = dynamic_cast<const VarExpr*>(node)) {
        return get_variable(var->name);
    }
    else if (auto* bin = dynamic_cast<const BinaryExpr*>(node)) {
        Value l = eval(bin->left.get());
        Value r = eval(bin->right.get());

        // Handle arithmetic operations
        if (bin->op == "+") {
            // String concatenation
            if (l.is_string() || r.is_string()) {
                return Value(l.to_string() + r.to_string());
            }
            // Vector addition
            else if (l.is_array() && r.is_array()) {
                return l.vector_add(r);
            }
            // Complex number addition
            else if (l.is_complex() || r.is_complex()) {
                ::lamina::Complex left_complex = l.is_complex() ? std::get<::lamina::Complex>(l.data) : ::lamina::Complex(l.as_number(), 0.0);
                ::lamina::Complex right_complex = r.is_complex() ? std::get<::lamina::Complex>(r.data) : ::lamina::Complex(r.as_number(), 0.0);
                return Value(left_complex + right_complex);
            }
            // Numeric addition with irrational and rational number support
            else if (l.is_numeric() && r.is_numeric()) {
                // If either operand is irrational, use irrational arithmetic
                if (l.is_irrational() || r.is_irrational()) {
                    ::Irrational result = l.as_irrational() + r.as_irrational();
                    return Value(result);
                }
                // If either operand is rational, use rational arithmetic
                if (l.is_rational() || r.is_rational()) {
                    ::Rational result = l.as_rational() + r.as_rational();
                    return Value(result);
                }

                double result = l.as_number() + r.as_number();                // Return int if both operands are int and result is whole
                if (l.is_int() && r.is_int()) {
                    return Value(static_cast<int>(result));
                }
                return Value(result);
            }
            else {
                error_and_exit("Cannot add " + l.to_string() + " and " + r.to_string());
            }
        }        // Arithmetic operations (require numeric operands or vector operations)
        if (bin->op == "-" || bin->op == "*" || bin->op == "/" ||
            bin->op == "%" || bin->op == "^") {

            // Special handling for multiplication
            if (bin->op == "*") {
                // Vector and matrix operations
                if (l.is_array() && r.is_array()) {
                    // Try dot product for same-size vectors
                    const auto& la = std::get<std::vector<Value>>(l.data);
                    const auto& ra = std::get<std::vector<Value>>(r.data);
                    if (la.size() == ra.size()) {
                        return l.dot_product(r);
                    }
                }
                // Matrix multiplication
                if (l.is_matrix() && r.is_matrix()) {
                    return l.matrix_multiply(r);
                }
                // Scalar multiplication for vectors
                if (l.is_array() && r.is_numeric()) {
                    return l.scalar_multiply(r.as_number());
                }
                if (l.is_numeric() && r.is_array()) {
                    return r.scalar_multiply(l.as_number());
                }
                // Complex number multiplication
                if (l.is_complex() || r.is_complex()) {
                    ::lamina::Complex left_complex = l.is_complex() ? std::get<::lamina::Complex>(l.data) : ::lamina::Complex(l.as_number(), 0.0);
                    ::lamina::Complex right_complex = r.is_complex() ? std::get<::lamina::Complex>(r.data) : ::lamina::Complex(r.as_number(), 0.0);
                    return Value(left_complex * right_complex);
                }
                // Regular multiplication (both must be numeric)
                if (l.is_numeric() && r.is_numeric()) {
                    // If either operand is irrational, use irrational arithmetic
                    if (l.is_irrational() || r.is_irrational()) {
                        ::Irrational result = l.as_irrational() * r.as_irrational();
                        return Value(result);
                    }
                    // If either operand is rational, use rational arithmetic
                    if (l.is_rational() || r.is_rational()) {
                        ::Rational result = l.as_rational() * r.as_rational();
                        return Value(result);
                    }

                    double result = l.as_number() * r.as_number();
                    return (l.is_int() && r.is_int()) ? Value(static_cast<int>(result)) : Value(result);
                }
                // Error case
                error_and_exit("Cannot multiply " + l.to_string() + " and " + r.to_string());
            }

            // Complex number subtraction
            if (bin->op == "-") {
                if (l.is_complex() || r.is_complex()) {
                    ::lamina::Complex left_complex = l.is_complex() ? std::get<::lamina::Complex>(l.data) : ::lamina::Complex(l.as_number(), 0.0);
                    ::lamina::Complex right_complex = r.is_complex() ? std::get<::lamina::Complex>(r.data) : ::lamina::Complex(r.as_number(), 0.0);
                    return Value(left_complex - right_complex);
                }
            }

            // Complex number division
            if (bin->op == "/") {
                if (l.is_complex() || r.is_complex()) {
                    ::lamina::Complex left_complex = l.is_complex() ? std::get<::lamina::Complex>(l.data) : ::lamina::Complex(l.as_number(), 0.0);
                    ::lamina::Complex right_complex = r.is_complex() ? std::get<::lamina::Complex>(r.data) : ::lamina::Complex(r.as_number(), 0.0);
                    return Value(left_complex / right_complex);
                }
            }

            // Other arithmetic operations require both operands to be numeric
            if (!l.is_numeric() || !r.is_numeric()) {
                error_and_exit("Arithmetic operation '" + bin->op + "' requires numeric operands");
            }

            // For division, always use rational arithmetic for precise results
            if (bin->op == "/") {
                // If either operand is irrational, use irrational arithmetic
                if (l.is_irrational() || r.is_irrational()) {
                    ::Irrational lr = l.as_irrational();
                    ::Irrational rr = r.as_irrational();
                    if (rr.is_zero()) {
                        error_and_exit("Division by zero");
                    }
                    return Value(lr / rr);
                }

                ::Rational lr = l.as_rational();
                ::Rational rr = r.as_rational();
                if (rr.is_zero()) {
                    error_and_exit("Division by zero");
                }
                return Value(lr / rr);
            }

            // Use irrational arithmetic if either operand is irrational
            if (l.is_irrational() || r.is_irrational()) {
                ::Irrational lr = l.as_irrational();
                ::Irrational rr = r.as_irrational();

                if (bin->op == "-") {
                    return Value(lr - rr);
                }
                // Note: Other operations (%, ^) may fall back to double arithmetic
                // for irrational numbers as they're complex to handle exactly
            }

            // Use rational arithmetic if either operand is rational
            if (l.is_rational() || r.is_rational()) {
                ::Rational lr = l.as_rational();
                ::Rational rr = r.as_rational();

                if (bin->op == "-") {
                    return Value(lr - rr);
                }
                if (bin->op == "%") {
                    // For rational modulo, convert to double temporarily
                    double ld = lr.to_double();
                    double rd = rr.to_double();
                    if (rd == 0.0) {
                        error_and_exit("Modulo by zero");
                    }
                    return Value(static_cast<int>(ld) % static_cast<int>(rd));
                }
                if (bin->op == "^") {
                    // For rational exponentiation, use integer exponent if possible
                    if (rr.is_integer() && rr.get_denominator() == 1) {
                        int exp = static_cast<int>(rr.get_numerator());
                        if (exp >= -1000 && exp <= 1000) { // Reasonable range
                            return Value(lr.pow(exp));
                        }
                    }
                    // Fall back to double arithmetic for non-integer or large exponents
                    return Value(std::pow(lr.to_double(), rr.to_double()));
                }
            }

            // Fall back to double arithmetic
            double ld = l.as_number();
            double rd = r.as_number();

            if (bin->op == "-") {
                double result = ld - rd;
                return (l.is_int() && r.is_int()) ? Value(static_cast<int>(result)) : Value(result);
            }
            if (bin->op == "%") {
                if (rd == 0.0) {
                    // 原：std::cerr << "Error: Modulo by zero" << std::endl;
                    error_and_exit("Modulo by zero");
                }
                return Value(static_cast<int>(ld) % static_cast<int>(rd));
            }
            if (bin->op == "^") {
                return Value(std::pow(ld, rd));
            }
        }

        // Comparison operators
        if (bin->op == "==" || bin->op == "!=" || bin->op == "<" ||
            bin->op == "<=" || bin->op == ">" || bin->op == ">=") {

            // Handle different type combinations
            if (l.is_numeric() && r.is_numeric()) {
                double ld = l.as_number();
                double rd = r.as_number();

                if (bin->op == "==") return Value(ld == rd);
                if (bin->op == "!=") return Value(ld != rd);
                if (bin->op == "<") return Value(ld < rd);
                if (bin->op == "<=") return Value(ld <= rd);
                if (bin->op == ">") return Value(ld > rd);
                if (bin->op == ">=") return Value(ld >= rd);
            }
            else if (l.is_string() && r.is_string()) {
                std::string ls = std::get<std::string>(l.data);
                std::string rs = std::get<std::string>(r.data);

                if (bin->op == "==") return Value(ls == rs);
                if (bin->op == "!=") return Value(ls != rs);
                if (bin->op == "<") return Value(ls < rs);
                if (bin->op == "<=") return Value(ls <= rs);
                if (bin->op == ">") return Value(ls > rs);
                if (bin->op == ">=") return Value(ls >= rs);
            }
            else if (l.is_bool() && r.is_bool()) {
                bool lb = std::get<bool>(l.data);
                bool rb = std::get<bool>(r.data);
                if (bin->op == "==") return Value(lb == rb);
                if (bin->op == "!=") return Value(lb != rb);
                // For booleans, false < true
                if (bin->op == "<") return Value(lb < rb);
                if (bin->op == "<=") return Value(lb <= rb);
                if (bin->op == ">") return Value(lb > rb);
                if (bin->op == ">=") return Value(lb >= rb);
            }
            else {
                // Type mismatch - only equality/inequality make sense
                if (bin->op == "==") return Value(false); // Different types are never equal
                if (bin->op == "!=") return Value(true); // Different types are always not equal

                error_and_exit("Cannot compare different types with operator '" + bin->op + "'");
                return Value();
            }
        }

        error_and_exit("Unknown binary operator '" + bin->op + "'");

    }
    else if (auto* unary = dynamic_cast<const UnaryExpr*>(node)) {
        Value v = eval(unary->operand.get());
          if (v.type != Value::Type::Int && v.type != Value::Type::BigInt) {
            RuntimeError error("Unary operator requires integer or big integer operand");
            error.stack_trace = get_stack_trace();
            throw error;
        }

        if (unary->op == "-") {
            if (v.type == Value::Type::Int) {
                int vi = std::get<int>(v.data);
                return Value(-vi);
            } else {
                // For BigInt, we need to implement negation
                ::BigInt big_val = std::get<::BigInt>(v.data);
                // For now, convert to int if possible
                return Value(-big_val.to_int());
            }
        }

        if (unary->op == "!") {
            int vi;
            if (v.type == Value::Type::Int) {
                vi = std::get<int>(v.data);
            } else {
                vi = std::get<::BigInt>(v.data).to_int();
            }

            if (vi < 0) {
                RuntimeError error("Cannot calculate factorial of negative number");
                error.stack_trace = get_stack_trace();
                throw error;
            }

            // Use BigInt for factorial if the number is large or result would be large
            if (vi > 20) {
                ::BigInt result(1);
                for (int j = 2; j <= vi; ++j) {
                    result = result * ::BigInt(j);
                }
                return Value(result);
            } else {
                // Use regular int for small factorials
                int res = 1;
                for (int j = 1; j <= vi; ++j) res *= j;
                return Value(res);
            }
        }

        std::cerr << "Error: Unknown unary operator '" << unary->op << "'" << std::endl;
        return Value("<unknown op>");
    }    // Support function calls
    else if (auto* call = dynamic_cast<const CallExpr*>(node)) {
        std::string actual_callee = call->callee;

        // 检查调用的名称是否是一个参数，如果是，获取其实际值
        try {
            Value callee_value = get_variable(actual_callee);
            if (callee_value.is_string() && std::get<std::string>(callee_value.data).substr(0, 11) == "__function_") {
                // 这是一个函数参数，提取实际的函数名
                actual_callee = std::get<std::string>(callee_value.data).substr(11);
            }
        } catch (const RuntimeError&) {
            // 如果不是变量，保持原名称
        }

        // Check builtin functions first
        auto builtin_it = builtin_functions.find(actual_callee);
        if (builtin_it != builtin_functions.end()) {
            std::vector<Value> args;
            for (const auto& arg : call->args) {
                if (arg) {
                    args.push_back(eval(arg.get()));
                } else {
                    std::cerr << "Error: Null argument in call to builtin function '" << actual_callee << "'" << std::endl;
                    return Value();
                }
            }
            return builtin_it->second(args);
        }

        // Check user-defined functions
        auto it = functions.find(actual_callee);
        if (it != functions.end()) {
            FuncDefStmt* func = it->second;
            if (!func) {
                std::cerr << "Error: Function object for '" << actual_callee << "' is null" << std::endl;
                return Value("<func error>");
            }

            // Check recursion depth
            if (recursion_depth >= max_recursion_depth) {
                RuntimeError error("Maximum recursion depth exceeded (" + std::to_string(max_recursion_depth) + ")");
                error.stack_trace = get_stack_trace();
                throw error;
            }

            recursion_depth++;
            push_scope();
            push_frame(actual_callee, "<script>", 0); // Add to call stack

            // Check parameter count
            if (call->args.size() > func->params.size()) {
                Interpreter::print_warning("Too many arguments provided to function '" + actual_callee +
                                          "'. Expected " + std::to_string(func->params.size()) +
                                          ", got " + std::to_string(call->args.size()), true);
            }

            // Pass arguments
            for (size_t j = 0; j < func->params.size(); ++j) {
                if (j < call->args.size()) {
                    if (call->args[j]) {
                        set_variable(func->params[j], eval(call->args[j].get()));
                    } else {
                        Interpreter::print_error("Null argument " + std::to_string(j+1) + " in call to function '" + actual_callee + "'", true);
                        set_variable(func->params[j], Value("<null arg>"));
                    }
                } else {
                    Interpreter::print_warning("Missing argument '" + func->params[j] + "' in call to function '" + actual_callee + "'", true);
                    set_variable(func->params[j], Value("<undefined>"));
                }
            }

            // Execute function body, capture return
            try {
                for (const auto& stmt : func->body->statements) {
                    try {
                        execute(stmt);
                    } catch (const ReturnException& re) {
                        // Normal return handling
                        pop_frame();
                        pop_scope();
                        recursion_depth--;
                        return re.value;
                    } catch (const RuntimeError& re) {
                        // If the error doesn't have a stack trace yet, capture current state
                        RuntimeError enriched(re.message);
                        if (re.stack_trace.empty()) {
                            enriched.stack_trace = get_stack_trace();
                        } else {
                            enriched.stack_trace = re.stack_trace; // Preserve existing trace
                        }
                        pop_frame();
                        pop_scope();
                        recursion_depth--;
                        throw enriched;
                    } catch (const std::exception& e) {
                        // Wrap standard exception as RuntimeError
                        RuntimeError enriched("In function '" + actual_callee + "': " + std::string(e.what()));
                        enriched.stack_trace = get_stack_trace(); // Get stack trace before cleanup
                        pop_frame();
                        pop_scope();
                        recursion_depth--;
                        throw enriched;
                    }
                }
            } catch (const ReturnException& re) {
                // Ensure cleanup in any case
                pop_frame();
                pop_scope();
                recursion_depth--;
                return re.value;
            }

            pop_frame();
            pop_scope();
            recursion_depth--;
            return Value(); // Default value when no return
        }

        std::cerr << "Error: Call to undefined function '" << actual_callee << "'" << std::endl;
        return Value("<undefined function>");
    }
    else if (auto* arr = dynamic_cast<const ArrayExpr*>(node)) {
        std::vector<Value> elements;
        for (const auto& element : arr->elements) {
            if (element) {
                elements.push_back(eval(element.get()));
            } else {
                elements.push_back(Value()); // Null element
            }
        }
        return Value(elements);
    }
    else if (auto* lit = dynamic_cast<const LiteralExpr*>(node)) {
        // Check if it's a complex number literal (like "3i" or "2.5i")
        if (lit->value.back() == 'i' && lit->value.length() > 1) {
            try {
                // Extract the numeric part (without 'i')
                std::string num_part = lit->value.substr(0, lit->value.length() - 1);
                double imag_val = std::stod(num_part);
                return Value(::lamina::Complex(0.0, imag_val));
            } catch (...) {
                // If parsing fails, treat as regular string
                return Value(lit->value);
            }
        }
        // Check if it's just "i" (imaginary unit)
        else if (lit->value == "i") {
            return Value(::lamina::Complex(0.0, 1.0));
        }
        // Regular literal processing
        else {
            // Try to parse as number first
            try {
                // Check if it contains a decimal point for float
                if (lit->value.find('.') != std::string::npos) {
                    double d = std::stod(lit->value);
                    return Value(d);
                } else {
                    int i = std::stoi(lit->value);
                    return Value(i);
                }
            } catch (...) {
                // Check for boolean literals
                if (lit->value == "true") return Value(true);
                if (lit->value == "false") return Value(false);
                if (lit->value == "null") return Value(nullptr);
                // Otherwise it's a string
                return Value(lit->value);
            }
        }
    }

    std::cerr << "Error: Unknown expression type" << std::endl;
    return Value("<unknown expr>");
}

void Interpreter::register_builtin_functions()
{
    for (auto entry : entry_functions) {
        entry(*this);
    }
}

// 在文件顶端添加错误处理函数
void error_and_exit(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
    std::exit(EXIT_FAILURE);
}

// 添加一个新的错误报告函数，只打印错误但不终止程序
// static void report_error(const std::string& msg) {
//     std::cerr << "Error: " << msg << std::endl;
// }

// 打印当前所有变量
void Interpreter::printVariables() const {
    if (variable_stack.empty()) {
        std::cout << "No variables defined." << std::endl;
        return;
    }

    std::cout << "\nCurrent variable list:" << std::endl;
    std::cout << "--------------------" << std::endl;

    // 从最内层作用域开始打印
    bool hasVars = false;
    for (auto it = variable_stack.rbegin(); it != variable_stack.rend(); ++it) {
        const auto& scope = *it;
        for (const auto& [name, value] : scope) {
            std::cout << name << " = " << value.to_string() << std::endl;
            hasVars = true;
        }
    }

    if (!hasVars) {
        std::cout << "No variables defined." << std::endl;
    }

    // 打印函数列表
    if (!functions.empty()) {
        std::cout << "\nDefined functions:" << std::endl;
        std::cout << "------------------" << std::endl;
        for (const auto& [name, _] : functions) {
            std::cout << name << "()" << std::endl;
        }
    }
}

// Stack trace management functions
void Interpreter::push_frame(const std::string& function_name, const std::string& file_name, int line_number) {
    call_stack.emplace_back(function_name, file_name, line_number);
}

void Interpreter::pop_frame() {
    if (!call_stack.empty()) {
        call_stack.pop_back();
    }
}

std::vector<StackFrame> Interpreter::get_stack_trace() const {
    return call_stack;
}

void Interpreter::print_stack_trace(const RuntimeError& error, bool use_colors) const {
    bool colors_enabled = supports_colors() && use_colors;

    // Use stack trace from error if available, otherwise use current call stack
    const auto& trace = error.stack_trace.empty() ? call_stack : error.stack_trace;

    // Print error header only if we have stack frames to show
    if (!trace.empty()) {
        if (colors_enabled) {
            std::cerr << "\033[1;31mTraceback (most recent call last):\033[0m\n";
        } else {
            std::cerr << "Traceback (most recent call last):\n";
        }

        // Print stack frames
        for (const auto& frame : trace) {
            if (colors_enabled) {
                std::cerr << "  File "\033[1;34m" << frame.file_name << "\033[0m", line "
                          << frame.line_number << ", in \033[1;33m" << frame.function_name << "\033[0m\n";
            } else {
                std::cerr << "  File "" << frame.file_name << "", line "
                          << frame.line_number << ", in " << frame.function_name << "\n";
            }
        }
    }

    // Print error message
    if (colors_enabled) {
        std::cerr << "\033[1;31mRuntimeError: " << error.message << "\033[0m\n";
    } else {
        std::cerr << "RuntimeError: " << error.message << "\n";
    }
}

bool Interpreter::supports_colors() {
    static bool checked = false;
    static bool colors_supported = false;

    if (!checked) {
        checked = true;
#ifdef _WIN32
        // On Windows, check if running in a terminal that supports ANSI colors
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
                colors_supported = true;
            }
        }
#else
        // On Unix-like systems, check if stdout is a terminal
        colors_supported = isatty(STDOUT_FILENO);
#endif
    }

    return colors_supported;
}

void Interpreter::print_warning(const std::string& message, bool use_colors) {
    bool colors_enabled = supports_colors() && use_colors;

    if (colors_enabled) {
        std::cerr << "\033[1;33mWarning: " << message << "\033[0m\n";
    } else {
        std::cerr << "Warning: " << message << "\n";
    }
}

void Interpreter::print_error(const std::string& message, bool use_colors) {
    bool colors_enabled = supports_colors() && use_colors;

    if (colors_enabled) {
        std::cerr << "\033[1;31mError: " << message << "\033[0m\n";
    } else {
        std::cerr << "Error: " << message << "\n";
    }
}

// Entry function registration
std::vector<void(*)(Interpreter&)> Interpreter::entry_functions;

void Interpreter::register_entry(void (*entry)(Interpreter&)) {
    entry_functions.push_back(entry);
}
