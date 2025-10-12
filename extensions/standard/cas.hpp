// Lamina CAS系统 - 计算机代数系统
#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cctype>

namespace LaminaCAS {

    // 基础表达式类
    class Expr {
    public:
        virtual ~Expr() = default;
        virtual std::string toString() const = 0;
        virtual std::unique_ptr<Expr> clone() const = 0;
        virtual std::unique_ptr<Expr> simplify() const = 0;
        virtual std::unique_ptr<Expr> differentiate(const std::string& var) const = 0;
        virtual double evaluate(const std::map<std::string, double>& vars = {}) const = 0;
    };

    using ExprPtr = std::unique_ptr<Expr>;

    // 数字表达式
    class Number : public Expr {
        double value;

    public:
        Number(double v) : value(v) {}

        std::string toString() const override {
            if (value == (int) value) {
                return std::to_string((int) value);
            }
            return std::to_string(value);
        }

        ExprPtr clone() const override {
            return std::make_unique<Number>(value);
        }

        ExprPtr simplify() const override {
            return clone();
        }

        ExprPtr differentiate(const std::string& var) const override {
            return std::make_unique<Number>(0);
        }

        double evaluate(const std::map<std::string, double>& vars = {}) const override {
            return value;
        }

        double getValue() const { return value; }
        bool isZero() const { return value == 0.0; }
        bool isOne() const { return value == 1.0; }
    };

    // 变量表达式
    class Variable : public Expr {
        std::string name;

    public:
        Variable(const std::string& n) : name(n) {}

        std::string toString() const override {
            return name;
        }

        ExprPtr clone() const override {
            return std::make_unique<Variable>(name);
        }

        ExprPtr simplify() const override {
            return clone();
        }

        ExprPtr differentiate(const std::string& var) const override {
            if (name == var) {
                return std::make_unique<Number>(1);
            }
            return std::make_unique<Number>(0);
        }

        double evaluate(const std::map<std::string, double>& vars = {}) const override {
            auto it = vars.find(name);
            if (it != vars.end()) {
                return it->second;
            }
            throw std::runtime_error("Variable " + name + " not found");
        }

        const std::string& getName() const { return name; }
    };

    // 加法表达式
    class Add : public Expr {
        ExprPtr left, right;

    public:
        Add(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}

        std::string toString() const override {
            return "(" + left->toString() + " + " + right->toString() + ")";
        }

        ExprPtr clone() const override {
            return std::make_unique<Add>(left->clone(), right->clone());
        }

        ExprPtr simplify() const override {
            auto l = left->simplify();
            auto r = right->simplify();

            // 0 + x = x
            if (auto ln = dynamic_cast<Number*>(l.get())) {
                if (ln->isZero()) return r;
            }
            // x + 0 = x
            if (auto rn = dynamic_cast<Number*>(r.get())) {
                if (rn->isZero()) return l;
            }
            // num + num = num
            if (auto ln = dynamic_cast<Number*>(l.get())) {
                if (auto rn = dynamic_cast<Number*>(r.get())) {
                    return std::make_unique<Number>(ln->getValue() + rn->getValue());
                }
            }

            return std::make_unique<Add>(std::move(l), std::move(r));
        }

        ExprPtr differentiate(const std::string& var) const override {
            return std::make_unique<Add>(left->differentiate(var), right->differentiate(var));
        }

        double evaluate(const std::map<std::string, double>& vars = {}) const override {
            return left->evaluate(vars) + right->evaluate(vars);
        }
    };

    // 乘法表达式
    class Multiply : public Expr {
        ExprPtr left, right;

    public:
        Multiply(ExprPtr l, ExprPtr r) : left(std::move(l)), right(std::move(r)) {}

        std::string toString() const override {
            return "(" + left->toString() + " * " + right->toString() + ")";
        }

        ExprPtr clone() const override {
            return std::make_unique<Multiply>(left->clone(), right->clone());
        }

        ExprPtr simplify() const override {
            auto l = left->simplify();
            auto r = right->simplify();

            // 0 * x = 0
            if (auto ln = dynamic_cast<Number*>(l.get())) {
                if (ln->isZero()) return std::make_unique<Number>(0);
                if (ln->isOne()) return r;  // 1 * x = x
            }
            // x * 0 = 0
            if (auto rn = dynamic_cast<Number*>(r.get())) {
                if (rn->isZero()) return std::make_unique<Number>(0);
                if (rn->isOne()) return l;  // x * 1 = x
            }
            // num * num = num
            if (auto ln = dynamic_cast<Number*>(l.get())) {
                if (auto rn = dynamic_cast<Number*>(r.get())) {
                    return std::make_unique<Number>(ln->getValue() * rn->getValue());
                }
            }

            return std::make_unique<Multiply>(std::move(l), std::move(r));
        }

        ExprPtr differentiate(const std::string& var) const override {
            // 乘积法则: (fg)' = f'g + fg'
            auto f_prime = left->differentiate(var);
            auto g_prime = right->differentiate(var);
            auto fg_prime = std::make_unique<Multiply>(std::move(f_prime), right->clone());
            auto gf_prime = std::make_unique<Multiply>(left->clone(), std::move(g_prime));
            return std::make_unique<Add>(std::move(fg_prime), std::move(gf_prime));
        }

        double evaluate(const std::map<std::string, double>& vars = {}) const override {
            return left->evaluate(vars) * right->evaluate(vars);
        }
    };

    // 幂表达式
    class Power : public Expr {
        ExprPtr base, exponent;

    public:
        Power(ExprPtr b, ExprPtr e) : base(std::move(b)), exponent(std::move(e)) {}

        std::string toString() const override {
            return "(" + base->toString() + " ^ " + exponent->toString() + ")";
        }

        ExprPtr clone() const override {
            return std::make_unique<Power>(base->clone(), exponent->clone());
        }

        ExprPtr simplify() const override {
            auto b = base->simplify();
            auto e = exponent->simplify();

            // x^0 = 1
            if (auto en = dynamic_cast<Number*>(e.get())) {
                if (en->isZero()) return std::make_unique<Number>(1);
                if (en->isOne()) return b;  // x^1 = x
            }
            // 0^x = 0 (x != 0)
            if (auto bn = dynamic_cast<Number*>(b.get())) {
                if (bn->isZero()) {
                    if (auto en = dynamic_cast<Number*>(e.get())) {
                        if (!en->isZero()) return std::make_unique<Number>(0);
                    }
                }
                if (bn->isOne()) return std::make_unique<Number>(1);    // 1^x = 1
            }

            return std::make_unique<Power>(std::move(b), std::move(e));
        }

        ExprPtr differentiate(const std::string& var) const override {
            // 如果指数是常数: (x^n)' = n*x^(n-1)*x'
            if (auto en = dynamic_cast<Number*>(exponent.get())) {
                auto n = en->getValue();
                auto new_exp = std::make_unique<Number>(n - 1);
                auto new_base = std::make_unique<Power>(base->clone(), std::move(new_exp));
                auto coeff = std::make_unique<Number>(n);
                auto base_deriv = base->differentiate(var);

                auto result = std::make_unique<Multiply>(std::move(coeff), std::move(new_base));
                return std::make_unique<Multiply>(std::move(result), std::move(base_deriv));
            }

            // 一般情况太复杂，先返回错误
            throw std::runtime_error("Differentiation of general exponentials not implemented");
        }

        double evaluate(const std::map<std::string, double>& vars = {}) const override {
            return std::pow(base->evaluate(vars), exponent->evaluate(vars));
        }
    };

    // 简单的解析器
    class Parser {
        std::string expr;
        size_t pos = 0;

        char current() const {
            if (pos >= expr.length()) return '\0';
            return expr[pos];
        }

        void skip_whitespace() {
            while (pos < expr.length() && std::isspace(expr[pos])) {
                pos++;
            }
        }

        ExprPtr parse_number() {
            std::string num;
            while (pos < expr.length() && (std::isdigit(expr[pos]) || expr[pos] == '.')) {
                num += expr[pos++];
            }
            return std::make_unique<Number>(std::stod(num));
        }

        ExprPtr parse_variable() {
            std::string name;
            while (pos < expr.length() && (std::isalnum(expr[pos]) || expr[pos] == '_')) {
                name += expr[pos++];
            }
            return std::make_unique<Variable>(name);
        }

        ExprPtr parse_factor() {
            skip_whitespace();

            if (current() == '(') {
                pos++;  // skip '('
                auto result = parse_expression();
                skip_whitespace();
                if (current() == ')') pos++;    // skip ')'
                return result;
            }

            if (std::isdigit(current())) {
                return parse_number();
            }

            if (std::isalpha(current()) || current() == '_') {
                return parse_variable();
            }

            throw std::runtime_error("Invalid expression");
        }

        ExprPtr parse_power() {
            auto left = parse_factor();

            skip_whitespace();
            if (current() == '^') {
                pos++;                      // skip '^'
                auto right = parse_power(); // 右结合
                return std::make_unique<Power>(std::move(left), std::move(right));
            }

            return left;
        }

        ExprPtr parse_term() {
            auto left = parse_power();

            while (true) {
                skip_whitespace();
                if (current() == '*') {
                    pos++;  // skip '*'
                    auto right = parse_power();
                    left = std::make_unique<Multiply>(std::move(left), std::move(right));
                } else {
                    break;
                }
            }

            return left;
        }

        ExprPtr parse_expression() {
            auto left = parse_term();

            while (true) {
                skip_whitespace();
                if (current() == '+') {
                    pos++;  // skip '+'
                    auto right = parse_term();
                    left = std::make_unique<Add>(std::move(left), std::move(right));
                } else {
                    break;
                }
            }

            return left;
        }

    public:
        Parser(const std::string& expression) : expr(expression) {}

        ExprPtr parse() {
            pos = 0;
            return parse_expression();
        }
    };

}   // namespace LaminaCAS
