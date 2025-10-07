/*
 * Lamina CAS (计算机代数系统) 扩展实现
 * 提供符号计算、微积分、方程求解等功能
 */

#include "standard.hpp"
#include "cas.hpp"
#include "../../interpreter/lamina_api/value.hpp"
#include <cstring>
#include <iostream>
#include <sstream>

using namespace LaminaCAS;

// 表达式存储
static std::map<std::string, ExprPtr> stored_expressions;

// 辅助函数：将Lamina Value转换为Expression
ExprPtr valueToExpression(const Value& value) {
    if (value.is_int()) {
        return std::make_unique<Number>(static_cast<double>(std::get<int>(value.data)));
    } else if (value.is_float()) {
        return std::make_unique<Number>(std::get<double>(value.data));
    } else if (value.is_string()) {
        std::string str = std::get<std::string>(value.data);
        try {
            Parser parser(str);
            return parser.parse();
        } catch (const std::exception& e) {
            return std::make_unique<Variable>(str);
        }
    }
    throw std::runtime_error("Unsupported value type for CAS operation");
}

// 辅助函数：将Expression转换为Lamina Value
Value expressionToValue(const Expr& expr) {
    if (const auto* num = dynamic_cast<const Number*>(&expr)) {
        return Value(num->getValue());
    } else {
        return Value(expr.toString());
    }
}

// CAS函数实现

Value cas_parse(const std::vector<Value>& args) {
    if (args.size() != 1 || !args[0].is_string()) {
        std::cerr << "Error: cas_parse() requires one string argument" << std::endl;
        return Value();
    }

    try {
        std::string expr_str = std::get<std::string>(args[0].data);
        Parser parser(expr_str);
        auto expr = parser.parse();
        auto simplified = expr->simplify();
        return expressionToValue(*simplified);
    } catch (const std::exception& e) {
        std::cerr << "CAS Parse Error: " << e.what() << std::endl;
        return Value();
    }
}

Value cas_simplify(const std::vector<Value>& args) {
    if (args.size() != 1) {
        std::cerr << "Error: cas_simplify() requires one argument" << std::endl;
        return Value();
    }

    try {
        auto expr = valueToExpression(args[0]);
        auto simplified = expr->simplify();
        return expressionToValue(*simplified);
    } catch (const std::exception& e) {
        std::cerr << "CAS Simplify Error: " << e.what() << std::endl;
        return Value();
    }
}

Value cas_differentiate(const std::vector<Value>& args) {
    if (args.size() != 2 || !args[1].is_string()) {
        std::cerr << "Error: cas_differentiate() requires expression and variable name" << std::endl;
        return Value();
    }

    try {
        auto expr = valueToExpression(args[0]);
        std::string variable = std::get<std::string>(args[1].data);
        auto derivative = expr->differentiate(variable);
        auto simplified = derivative->simplify();
        return expressionToValue(*simplified);
    } catch (const std::exception& e) {
        std::cerr << "CAS Differentiate Error: " << e.what() << std::endl;
        return Value();
    }
}

Value cas_evaluate(const std::vector<Value>& args) {
    if (args.size() < 1) {
        std::cerr << "Error: cas_evaluate() requires at least one argument" << std::endl;
        return Value();
    }

    try {
        auto expr = valueToExpression(args[0]);
        std::map<std::string, double> variables;

        // 处理变量赋值
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i].is_string()) {
                std::string assignment = std::get<std::string>(args[i].data);
                size_t eq_pos = assignment.find('=');
                if (eq_pos != std::string::npos) {
                    std::string var_name = assignment.substr(0, eq_pos);
                    std::string value_str = assignment.substr(eq_pos + 1);
                    double value = std::stod(value_str);
                    variables[var_name] = value;
                }
            }
        }

        double result = expr->evaluate(variables);
        return Value(result);
    } catch (const std::exception& e) {
        std::cerr << "CAS Evaluate Error: " << e.what() << std::endl;
        return Value();
    }
}

// 存储表达式
Value cas_store(const std::vector<Value>& args) {
    if (args.size() != 2 || !args[0].is_string()) {
        std::cerr << "Error: cas_store() requires name and expression" << std::endl;
        return Value();
    }

    try {
        std::string name = std::get<std::string>(args[0].data);
        auto expr = valueToExpression(args[1]);
        stored_expressions[name] = std::move(expr);
        return Value("Expression stored as: " + name);
    } catch (const std::exception& e) {
        std::cerr << "CAS Store Error: " << e.what() << std::endl;
        return Value();
    }
}

// 加载存储的表达式
Value cas_load(const std::vector<Value>& args) {
    if (args.size() != 1 || !args[0].is_string()) {
        std::cerr << "Error: cas_load() requires expression name" << std::endl;
        return Value();
    }

    try {
        std::string name = std::get<std::string>(args[0].data);
        auto it = stored_expressions.find(name);
        if (it != stored_expressions.end()) {
            return expressionToValue(*it->second);
        } else {
            return Value("Expression not found: " + name);
        }
    } catch (const std::exception& e) {
        std::cerr << "CAS Load Error: " << e.what() << std::endl;
        return Value();
    }
}

// 在特定点求值（支持多个变量）
Value cas_evaluate_at(const std::vector<Value>& args) {
    if (args.size() != 3 || !args[1].is_string()) {
        std::cerr << "Error: cas_evaluate_at() requires expression, variable, and value" << std::endl;
        return Value();
    }

    try {
        auto expr = valueToExpression(args[0]);
        std::string variable = std::get<std::string>(args[1].data);

        double point;
        if (args[2].is_int()) {
            point = static_cast<double>(std::get<int>(args[2].data));
        } else if (args[2].is_float()) {
            point = std::get<double>(args[2].data);
        } else {
            throw std::runtime_error("Point must be a number");
        }

        std::map<std::string, double> variables = {{variable, point}};
        double result = expr->evaluate(variables);
        return Value(result);
    } catch (const std::exception& e) {
        std::cerr << "CAS Evaluate At Error: " << e.what() << std::endl;
        return Value();
    }
}

// 简单的方程求解（仅支持线性方程）
Value cas_solve_linear(const std::vector<Value>& args) {
    if (args.size() != 2 || !args[1].is_string()) {
        std::cerr << "Error: cas_solve_linear() requires equation and variable" << std::endl;
        return Value();
    }

    try {
        auto expr = valueToExpression(args[0]);
        std::string variable = std::get<std::string>(args[1].data);

        // 简单的线性方程求解：ax + b = 0 的解为 x = -b/a
        // 计算 f(0) 和 f(1) 来估算线性系数
        std::map<std::string, double> vars_0 = {{variable, 0.0}};
        std::map<std::string, double> vars_1 = {{variable, 1.0}};

        double f_0 = expr->evaluate(vars_0);    // b
        double f_1 = expr->evaluate(vars_1);    // a + b

        double a = f_1 - f_0;   // 斜率
        double b = f_0;         // 截距

        if (std::abs(a) < 1e-10) {
            if (std::abs(b) < 1e-10) {
                return Value("Infinitely many solutions");
            } else {
                return Value("No solution");
            }
        }

        double solution = -b / a;
        return Value(solution);

    } catch (const std::exception& e) {
        std::cerr << "CAS Solve Error: " << e.what() << std::endl;
        return Value();
    }
}

// 数值求导（用于验证符号求导）
Value cas_numerical_derivative(const std::vector<Value>& args) {
    if (args.size() != 3 || !args[1].is_string()) {
        std::cerr << "Error: cas_numerical_derivative() requires expression, variable, and point" << std::endl;
        return Value();
    }

    try {
        auto expr = valueToExpression(args[0]);
        std::string variable = std::get<std::string>(args[1].data);
        double point;

        if (args[2].is_int()) {
            point = static_cast<double>(std::get<int>(args[2].data));
        } else if (args[2].is_float()) {
            point = std::get<double>(args[2].data);
        } else {
            throw std::runtime_error("Point must be a number");
        }

        // 使用差分近似求导数：f'(x) ≈ (f(x+h) - f(x-h)) / (2h)
        double h = 1e-8;
        std::map<std::string, double> vars_plus = {{variable, point + h}};
        std::map<std::string, double> vars_minus = {{variable, point - h}};

        double f_plus = expr->evaluate(vars_plus);
        double f_minus = expr->evaluate(vars_minus);

        double derivative = (f_plus - f_minus) / (2 * h);
        return Value(derivative);

    } catch (const std::exception& e) {
        std::cerr << "CAS Numerical Derivative Error: " << e.what() << std::endl;
        return Value();
    }
}
