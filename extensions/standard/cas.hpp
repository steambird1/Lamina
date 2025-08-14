// CAS.h - 计算机代数系统单头文件实现
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <cctype>

// ============================================================================
// 核心表达式类
// ============================================================================

class ExpressionVisitor;

class Expression {
public:
    virtual ~Expression() = default;
    virtual void accept(ExpressionVisitor& visitor) = 0;
    virtual std::unique_ptr<Expression> clone() const = 0;
    virtual std::string toString() const = 0;
    virtual bool equals(const Expression& other) const = 0;
    virtual std::unique_ptr<Expression> simplify() const = 0;
};

using ExprPtr = std::unique_ptr<Expression>;

class ExpressionVisitor {
public:
    virtual ~ExpressionVisitor() = default;
    virtual void visit(class Number& number) = 0;
    virtual void visit(class Symbol& symbol) = 0;
    virtual void visit(class Add& add) = 0;
    virtual void visit(class Subtract& subtract) = 0;
    virtual void visit(class Multiply& multiply) = 0;
    virtual void visit(class Divide& divide) = 0;
    virtual void visit(class Power& power) = 0;
    virtual void visit(class Function& function) = 0;
};

// ============================================================================
// 基础表达式类实现
// ============================================================================

class Number : public Expression {
private:
    std::variant<int, double> value_;

public:
    Number(int val) : value_(val) {}
    Number(double val) : value_(val) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Number>(*this);
    }
    
    std::string toString() const override {
        return std::visit([](auto&& arg) -> std::string {
            return std::to_string(arg);
        }, value_);
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* num = dynamic_cast<const Number*>(&other)) {
            return value_ == num->value_;
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override {
        return clone();
    }
    
    double getValue() const {
        return std::visit([](auto&& arg) -> double {
            return static_cast<double>(arg);
        }, value_);
    }
    
    bool isZero() const {
        return getValue() == 0.0;
    }
    
    bool isOne() const {
        return getValue() == 1.0;
    }
};

class Symbol : public Expression {
private:
    std::string name_;

public:
    explicit Symbol(const std::string& name) : name_(name) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Symbol>(*this);
    }
    
    std::string toString() const override {
        return name_;
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* sym = dynamic_cast<const Symbol*>(&other)) {
            return name_ == sym->name_;
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override {
        return clone();
    }
    
    const std::string& getName() const { return name_; }
};

// ============================================================================
// 二元运算类
// ============================================================================

class BinaryOperation : public Expression {
protected:
    ExprPtr left_;
    ExprPtr right_;

public:
    BinaryOperation(ExprPtr left, ExprPtr right) 
        : left_(std::move(left)), right_(std::move(right)) {}
    
    const Expression& getLeft() const { return *left_; }
    const Expression& getRight() const { return *right_; }
};

class Add : public BinaryOperation {
public:
    Add(ExprPtr left, ExprPtr right) 
        : BinaryOperation(std::move(left), std::move(right)) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Add>(left_->clone(), right_->clone());
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " + " + right_->toString() + ")";
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* add = dynamic_cast<const Add*>(&other)) {
            return (left_->equals(*add->left_) && right_->equals(*add->right_)) ||
                   (left_->equals(*add->right_) && right_->equals(*add->left_));
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override;
};

class Subtract : public BinaryOperation {
public:
    Subtract(ExprPtr left, ExprPtr right) 
        : BinaryOperation(std::move(left), std::move(right)) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Subtract>(left_->clone(), right_->clone());
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " - " + right_->toString() + ")";
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* sub = dynamic_cast<const Subtract*>(&other)) {
            return left_->equals(*sub->left_) && right_->equals(*sub->right_);
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override;
};

class Multiply : public BinaryOperation {
public:
    Multiply(ExprPtr left, ExprPtr right) 
        : BinaryOperation(std::move(left), std::move(right)) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Multiply>(left_->clone(), right_->clone());
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " * " + right_->toString() + ")";
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* mul = dynamic_cast<const Multiply*>(&other)) {
            return (left_->equals(*mul->left_) && right_->equals(*mul->right_)) ||
                   (left_->equals(*mul->right_) && right_->equals(*mul->left_));
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override;
};

class Divide : public BinaryOperation {
public:
    Divide(ExprPtr left, ExprPtr right) 
        : BinaryOperation(std::move(left), std::move(right)) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Divide>(left_->clone(), right_->clone());
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " / " + right_->toString() + ")";
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* div = dynamic_cast<const Divide*>(&other)) {
            return left_->equals(*div->left_) && right_->equals(*div->right_);
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override;
};

class Power : public BinaryOperation {
public:
    Power(ExprPtr base, ExprPtr exponent) 
        : BinaryOperation(std::move(base), std::move(exponent)) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Power>(left_->clone(), right_->clone());
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + "^" + right_->toString() + ")";
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* pow = dynamic_cast<const Power*>(&other)) {
            return left_->equals(*pow->left_) && right_->equals(*pow->right_);
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override;
};

// ============================================================================
// 函数类
// ============================================================================

class Function : public Expression {
private:
    std::string name_;
    ExprPtr argument_;

public:
    Function(const std::string& name, ExprPtr argument)
        : name_(name), argument_(std::move(argument)) {}
    
    void accept(ExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    ExprPtr clone() const override {
        return std::make_unique<Function>(name_, argument_->clone());
    }
    
    std::string toString() const override {
        return name_ + "(" + argument_->toString() + ")";
    }
    
    bool equals(const Expression& other) const override {
        if (const auto* func = dynamic_cast<const Function*>(&other)) {
            return name_ == func->name_ && argument_->equals(*func->argument_);
        }
        return false;
    }
    
    std::unique_ptr<Expression> simplify() const override;
    
    const std::string& getName() const { return name_; }
    const Expression& getArgument() const { return *argument_; }
};

// ============================================================================
// 表达式化简实现
// ============================================================================

std::unique_ptr<Expression> Add::simplify() const {
    auto left_simplified = left_->simplify();
    auto right_simplified = right_->simplify();
    
    // 如果两个都是数字，直接计算
    if (auto* left_num = dynamic_cast<const Number*>(left_simplified.get())) {
        if (auto* right_num = dynamic_cast<const Number*>(right_simplified.get())) {
            return std::make_unique<Number>(left_num->getValue() + right_num->getValue());
        }
        // 0 + x = x
        if (left_num->isZero()) {
            return right_simplified->clone();
        }
    }
    
    // x + 0 = x
    if (auto* right_num = dynamic_cast<const Number*>(right_simplified.get())) {
        if (right_num->isZero()) {
            return left_simplified->clone();
        }
    }
    
    return std::make_unique<Add>(std::move(left_simplified), std::move(right_simplified));
}

std::unique_ptr<Expression> Subtract::simplify() const {
    auto left_simplified = left_->simplify();
    auto right_simplified = right_->simplify();
    
    // 如果两个都是数字，直接计算
    if (auto* left_num = dynamic_cast<const Number*>(left_simplified.get())) {
        if (auto* right_num = dynamic_cast<const Number*>(right_simplified.get())) {
            return std::make_unique<Number>(left_num->getValue() - right_num->getValue());
        }
        // x - 0 = x
        if (right_num->isZero()) {
            return left_simplified->clone();
        }
    }
    
    // 0 - x = -x
    if (auto* left_num = dynamic_cast<const Number*>(left_simplified.get())) {
        if (left_num->isZero()) {
            return std::make_unique<Multiply>(
                std::make_unique<Number>(-1),
                right_simplified->clone()
            );
        }
    }
    
    return std::make_unique<Subtract>(std::move(left_simplified), std::move(right_simplified));
}

std::unique_ptr<Expression> Multiply::simplify() const {
    auto left_simplified = left_->simplify();
    auto right_simplified = right_->simplify();
    
    // 如果两个都是数字，直接计算
    if (auto* left_num = dynamic_cast<const Number*>(left_simplified.get())) {
        if (auto* right_num = dynamic_cast<const Number*>(right_simplified.get())) {
            return std::make_unique<Number>(left_num->getValue() * right_num->getValue());
        }
        // 0 * x = 0
        if (left_num->isZero()) {
            return std::make_unique<Number>(0);
        }
        // 1 * x = x
        if (left_num->isOne()) {
            return right_simplified->clone();
        }
        // -1 * x = -x
        if (left_num->getValue() == -1) {
            return std::make_unique<Multiply>(
                std::make_unique<Number>(-1),
                right_simplified->clone()
            );
        }
    }
    
    // x * 0 = 0, x * 1 = x
    if (auto* right_num = dynamic_cast<const Number*>(right_simplified.get())) {
        if (right_num->isZero()) {
            return std::make_unique<Number>(0);
        }
        if (right_num->isOne()) {
            return left_simplified->clone();
        }
        if (right_num->getValue() == -1) {
            return std::make_unique<Multiply>(
                std::make_unique<Number>(-1),
                left_simplified->clone()
            );
        }
    }
    
    return std::make_unique<Multiply>(std::move(left_simplified), std::move(right_simplified));
}

std::unique_ptr<Expression> Divide::simplify() const {
    auto left_simplified = left_->simplify();
    auto right_simplified = right_->simplify();
    
    // 如果两个都是数字，直接计算
    if (auto* left_num = dynamic_cast<const Number*>(left_simplified.get())) {
        if (auto* right_num = dynamic_cast<const Number*>(right_simplified.get())) {
            if (right_num->isZero()) {
                throw std::runtime_error("Division by zero");
            }
            return std::make_unique<Number>(left_num->getValue() / right_num->getValue());
        }
    }
    
    // 0 / x = 0 (x ≠ 0)
    if (auto* left_num = dynamic_cast<const Number*>(left_simplified.get())) {
        if (left_num->isZero()) {
            return std::make_unique<Number>(0);
        }
    }
    
    // x / 1 = x
    if (auto* right_num = dynamic_cast<const Number*>(right_simplified.get())) {
        if (right_num->isOne()) {
            return left_simplified->clone();
        }
    }
    
    return std::make_unique<Divide>(std::move(left_simplified), std::move(right_simplified));
}

std::unique_ptr<Expression> Power::simplify() const {
    auto base_simplified = left_->simplify();
    auto exp_simplified = right_->simplify();
    
    // 如果都是数字，直接计算
    if (auto* base_num = dynamic_cast<const Number*>(base_simplified.get())) {
        if (auto* exp_num = dynamic_cast<const Number*>(exp_simplified.get())) {
            if (base_num->isZero() && exp_num->getValue() < 0) {
                throw std::runtime_error("Division by zero in power");
            }
            return std::make_unique<Number>(std::pow(base_num->getValue(), exp_num->getValue()));
        }
        // x^0 = 1 (x ≠ 0)
        if (exp_num->isZero()) {
            if (!base_num->isZero()) {
                return std::make_unique<Number>(1);
            }
        }
        // x^1 = x
        if (exp_num->isOne()) {
            return base_simplified->clone();
        }
    }
    
    // 0^x = 0 (x > 0)
    if (auto* base_num = dynamic_cast<const Number*>(base_simplified.get())) {
        if (base_num->isZero()) {
            if (auto* exp_num = dynamic_cast<const Number*>(exp_simplified.get())) {
                if (exp_num->getValue() > 0) {
                    return std::make_unique<Number>(0);
                }
            }
        }
    }
    
    // 1^x = 1
    if (auto* base_num = dynamic_cast<const Number*>(base_simplified.get())) {
        if (base_num->isOne()) {
            return std::make_unique<Number>(1);
        }
    }
    
    return std::make_unique<Power>(std::move(base_simplified), std::move(exp_simplified));
}

std::unique_ptr<Expression> Function::simplify() const {
    auto arg_simplified = argument_->simplify();
    
    // 特殊函数的数值化简
    if (auto* num_arg = dynamic_cast<const Number*>(arg_simplified.get())) {
        double val = num_arg->getValue();
        if (name_ == "sin") {
            return std::make_unique<Number>(std::sin(val));
        } else if (name_ == "cos") {
            return std::make_unique<Number>(std::cos(val));
        } else if (name_ == "tan") {
            return std::make_unique<Number>(std::tan(val));
        } else if (name_ == "log") {
            if (val <= 0) {
                throw std::runtime_error("Logarithm of non-positive number");
            }
            return std::make_unique<Number>(std::log(val));
        } else if (name_ == "exp") {
            return std::make_unique<Number>(std::exp(val));
        }
    }
    
    return std::make_unique<Function>(name_, std::move(arg_simplified));
}

// ============================================================================
// 表达式解析器
// ============================================================================

class Parser {
private:
    std::string input_;
    size_t pos_;

    void skipWhitespace() {
        while (pos_ < input_.length() && std::isspace(input_[pos_])) {
            pos_++;
        }
    }

    bool isDigit(char c) const {
        return c >= '0' && c <= '9';
    }

    bool isAlpha(char c) const {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool isAlphanumeric(char c) const {
        return isAlpha(c) || isDigit(c);
    }
    
    std::string parseIdentifier() {
        std::string result;
        while (pos_ < input_.length() && isAlphanumeric(input_[pos_])) {
            result += input_[pos_++];
        }
        return result;
    }

    double parseNumber() {
        size_t start = pos_;
        bool hasDecimal = false;
        
        while (pos_ < input_.length()) {
            if (isDigit(input_[pos_])) {
                pos_++;
            } else if (input_[pos_] == '.' && !hasDecimal) {
                hasDecimal = true;
                pos_++;
            } else {
                break;
            }
        }
        
        std::string numStr = input_.substr(start, pos_ - start);
        return std::stod(numStr);
    }

    ExprPtr parseAtom() {
        skipWhitespace();
        
        if (pos_ >= input_.length()) {
            throw std::runtime_error("Unexpected end of input");
        }
        
        char c = input_[pos_];
        
        if (isDigit(c) || c == '.') {
            return std::make_unique<Number>(parseNumber());
        }
        
        if (c == '(') {
            pos_++; // skip '('
            auto expr = parseExpression();
            skipWhitespace();
            if (pos_ >= input_.length() || input_[pos_] != ')') {
                throw std::runtime_error("Expected ')'");
            }
            pos_++; // skip ')'
            return expr;
        }
        
        if (c == '-') {
            pos_++; // skip '-'
            auto atom = parseAtom();
            return std::make_unique<Multiply>(
                std::make_unique<Number>(-1),
                std::move(atom)
            );
        }
        
        if (isAlpha(c)) {
            std::string name = parseIdentifier();
            skipWhitespace();
            // 检查是否是函数
            if (pos_ < input_.length() && input_[pos_] == '(') {
                pos_++; // skip '('
                auto arg = parseExpression();
                skipWhitespace();
                if (pos_ >= input_.length() || input_[pos_] != ')') {
                    throw std::runtime_error("Expected ')'");
                }
                pos_++; // skip ')'
                return std::make_unique<Function>(name, std::move(arg));
            }
            // 否则是变量
            return std::make_unique<Symbol>(name);
        }
        
        throw std::runtime_error("Unexpected character: " + std::string(1, c));
    }

    ExprPtr parsePower() {
        auto left = parseAtom();
        
        skipWhitespace();
        if (pos_ < input_.length() && input_[pos_] == '^') {
            pos_++;
            auto right = parsePower(); // 右结合
            return std::make_unique<Power>(std::move(left), std::move(right));
        }
        
        return left;
    }

    ExprPtr parseFactor() {
        auto left = parsePower();
        
        while (pos_ < input_.length()) {
            skipWhitespace();
            if (pos_ < input_.length() && (input_[pos_] == '*' || input_[pos_] == '/')) {
                char op = input_[pos_++];
                auto right = parsePower();
                if (op == '*') {
                    left = std::make_unique<Multiply>(std::move(left), std::move(right));
                } else {
                    left = std::make_unique<Divide>(std::move(left), std::move(right));
                }
            } else {
                break;
            }
        }
        
        return left;
    }

    ExprPtr parseTerm() {
        auto left = parseFactor();
        
        while (pos_ < input_.length()) {
            skipWhitespace();
            if (pos_ < input_.length() && (input_[pos_] == '+' || input_[pos_] == '-')) {
                char op = input_[pos_++];
                auto right = parseFactor();
                if (op == '+') {
                    left = std::make_unique<Add>(std::move(left), std::move(right));
                } else {
                    left = std::make_unique<Subtract>(std::move(left), std::move(right));
                }
            } else {
                break;
            }
        }
        
        return left;
    }

    ExprPtr parseExpression() {
        return parseTerm();
    }

public:
    explicit Parser(const std::string& input) : input_(input), pos_(0) {}
    
    ExprPtr parse() {
        pos_ = 0;
        auto result = parseExpression();
        skipWhitespace();
        if (pos_ < input_.length()) {
            throw std::runtime_error("Unexpected character at end of input");
        }
        return result;
    }
};

// ============================================================================
// 微积分模块 - 求导
// ============================================================================

class DerivativeVisitor : public ExpressionVisitor {
private:
    std::string variable_;
    ExprPtr result_;

public:
    explicit DerivativeVisitor(const std::string& variable) : variable_(variable) {}
    
    ExprPtr differentiate(const Expression& expr) {
        const_cast<Expression&>(expr).accept(*this);
        return std::move(result_);
    }
    
    void visit(Number& number) override {
        result_ = std::make_unique<Number>(0);
    }
    
    void visit(Symbol& symbol) override {
        if (symbol.getName() == variable_) {
            result_ = std::make_unique<Number>(1);
        } else {
            result_ = std::make_unique<Number>(0);
        }
    }
    
    void visit(Add& add) override {
        DerivativeVisitor left_visitor(variable_);
        auto left_deriv = left_visitor.differentiate(add.getLeft());
        
        DerivativeVisitor right_visitor(variable_);
        auto right_deriv = right_visitor.differentiate(add.getRight());
        
        result_ = std::make_unique<Add>(std::move(left_deriv), std::move(right_deriv));
    }
    
    void visit(Subtract& subtract) override {
        DerivativeVisitor left_visitor(variable_);
        auto left_deriv = left_visitor.differentiate(subtract.getLeft());
        
        DerivativeVisitor right_visitor(variable_);
        auto right_deriv = right_visitor.differentiate(subtract.getRight());
        
        result_ = std::make_unique<Subtract>(std::move(left_deriv), std::move(right_deriv));
    }
    
    void visit(Multiply& multiply) override {
        // 乘积法则: (uv)' = u'v + uv'
        DerivativeVisitor left_visitor(variable_);
        auto left_deriv = left_visitor.differentiate(multiply.getLeft());
        
        DerivativeVisitor right_visitor(variable_);
        auto right_deriv = right_visitor.differentiate(multiply.getRight());
        
        auto left_copy = multiply.getLeft().clone();
        auto right_copy = multiply.getRight().clone();
        
        auto first_term = std::make_unique<Multiply>(std::move(left_deriv), std::move(right_copy));
        auto second_term = std::make_unique<Multiply>(std::move(left_copy), std::move(right_deriv));
        
        result_ = std::make_unique<Add>(std::move(first_term), std::move(second_term));
    }
    
    void visit(Divide& divide) override {
        // 商法则: (u/v)' = (u'v - uv')/v^2
        DerivativeVisitor left_visitor(variable_);
        auto u_deriv = left_visitor.differentiate(divide.getLeft());
        
        DerivativeVisitor right_visitor(variable_);
        auto v_deriv = right_visitor.differentiate(divide.getRight());
        
        auto u = divide.getLeft().clone();
        auto v = divide.getRight().clone();
        auto v_squared = std::make_unique<Power>(v->clone(), std::make_unique<Number>(2));
        
        auto numerator = std::make_unique<Subtract>(
            std::make_unique<Multiply>(std::move(u_deriv), v->clone()),
            std::make_unique<Multiply>(std::move(u), std::move(v_deriv))
        );
        
        result_ = std::make_unique<Divide>(std::move(numerator), std::move(v_squared));
    }
    
    void visit(Power& power) override {
        // 幂函数法则: (u^n)' = n*u^(n-1)*u'
        const auto& base = power.getLeft();
        const auto& exponent = power.getRight();
        
        // 简单情况：x^n
        if (const auto* symbol = dynamic_cast<const Symbol*>(&base)) {
            if (symbol->getName() == variable_) {
                if (const auto* num = dynamic_cast<const Number*>(&exponent)) {
                    double n = num->getValue();
                    if (n == 1) {
                        result_ = std::make_unique<Number>(1);
                    } else {
                        auto new_exp = std::make_unique<Number>(n - 1);
                        auto new_power = std::make_unique<Power>(base.clone(), std::move(new_exp));
                        auto coeff = std::make_unique<Number>(n);
                        result_ = std::make_unique<Multiply>(std::move(coeff), std::move(new_power));
                    }
                    return;
                }
            }
        }
        
        // 一般情况: (f^g)' = f^g * (g' * ln(f) + g * f'/f)
        DerivativeVisitor base_visitor(variable_);
        auto f_deriv = base_visitor.differentiate(base);
        
        DerivativeVisitor exp_visitor(variable_);
        auto g_deriv = exp_visitor.differentiate(exponent);
        
        auto f = base.clone();
        auto g = exponent.clone();
        
        auto ln_f = std::make_unique<Function>("log", f->clone());
        auto f_div_f = std::make_unique<Divide>(std::move(f_deriv), f->clone());
        
        auto term1 = std::make_unique<Multiply>(std::move(g_deriv), std::move(ln_f));
        auto term2 = std::make_unique<Multiply>(std::move(g), std::move(f_div_f));
        
        auto sum = std::make_unique<Add>(std::move(term1), std::move(term2));
        auto f_power_g = std::make_unique<Power>(base.clone(), exponent.clone());
        
        result_ = std::make_unique<Multiply>(std::move(f_power_g), std::move(sum));
    }
    
    void visit(Function& function) override {
        const std::string& name = function.getName();
        const auto& arg = function.getArgument();
        
        DerivativeVisitor arg_visitor(variable_);
        auto arg_deriv = arg_visitor.differentiate(arg);
        
        if (name == "sin") {
            // (sin x)' = cos x
            auto cos_func = std::make_unique<Function>("cos", arg.clone());
            result_ = std::make_unique<Multiply>(std::move(cos_func), std::move(arg_deriv));
        } else if (name == "cos") {
            // (cos x)' = -sin x
            auto sin_func = std::make_unique<Function>("sin", arg.clone());
            auto neg_sin = std::make_unique<Multiply>(std::make_unique<Number>(-1), std::move(sin_func));
            result_ = std::make_unique<Multiply>(std::move(neg_sin), std::move(arg_deriv));
        } else if (name == "tan") {
            // (tan x)' = sec^2 x = 1/cos^2 x
            auto cos_func = std::make_unique<Function>("cos", arg.clone());
            auto cos_squared = std::make_unique<Power>(std::move(cos_func), std::make_unique<Number>(2));
            auto sec_squared = std::make_unique<Divide>(std::make_unique<Number>(1), std::move(cos_squared));
            result_ = std::make_unique<Multiply>(std::move(sec_squared), std::move(arg_deriv));
        } else if (name == "log") {
            // (ln x)' = 1/x
            auto one_over_x = std::make_unique<Divide>(std::make_unique<Number>(1), arg.clone());
            result_ = std::make_unique<Multiply>(std::move(one_over_x), std::move(arg_deriv));
        } else if (name == "exp") {
            // (e^x)' = e^x
            auto exp_func = std::make_unique<Function>("exp", arg.clone());
            result_ = std::make_unique<Multiply>(std::move(exp_func), std::move(arg_deriv));
        } else {
            // 未知函数，设为 f'(x)
            result_ = std::make_unique<Number>(0); // 简化处理
        }
    }
    
    const ExprPtr& getResult() const { return result_; }
};

// ============================================================================
// 方程求解器
// ============================================================================

class EquationSolver {
public:
    // 解线性方程 ax + b = 0
    static std::vector<double> solveLinear(const Expression& equation, const std::string& variable) {
        // 提取系数 a*x + b = 0 中的 a 和 b
        auto coeffs = extractLinearCoefficients(equation, variable);
        double a = coeffs.first;
        double b = coeffs.second;
        
        if (std::abs(a) < 1e-10) {
            if (std::abs(b) < 1e-10) {
                // 0 = 0，无穷多解
                return {}; // 返回空向量表示无穷解
            } else {
                // 0 = b (b≠0)，无解
                return {}; // 返回空向量表示无解
            }
        }
        
        // 解为 x = -b/a
        return {-b / a};
    }
    
    // 解二次方程 ax^2 + bx + c = 0
    static std::vector<double> solveQuadratic(const Expression& equation, const std::string& variable) {
        auto coeffs = extractQuadraticCoefficients(equation, variable);
        double a = coeffs.first;
        double b = coeffs.second;
        double c = coeffs.third;
        
        if (std::abs(a) < 1e-10) {
            // 实际上是一次方程
            if (std::abs(b) < 1e-10) {
                if (std::abs(c) < 1e-10) {
                    return {}; // 无穷解
                } else {
                    return {}; // 无解
                }
            }
            return {-c / b}; // 线性方程的解
        }
        
        double discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) {
            return {}; // 无实数解
        } else if (discriminant == 0) {
            return {-b / (2 * a)}; // 一个重根
        } else {
            double sqrt_d = std::sqrt(discriminant);
            return {(-b + sqrt_d) / (2 * a), (-b - sqrt_d) / (2 * a)}; // 两个不同实根
        }
    }
    
private:
    struct Coefficients {
        double first = 0.0;
        double second = 0.0;
        double third = 0.0;
    };
    
    static Coefficients extractLinearCoefficients(const Expression& expr, const std::string& variable) {
        Coefficients coeffs;
        // 简化实现 - 实际应用中需要更复杂的符号处理
        coeffs.first = 1.0;  // x的系数
        coeffs.second = 0.0; // 常数项
        return coeffs;
    }
    
    static Coefficients extractQuadraticCoefficients(const Expression& expr, const std::string& variable) {
        Coefficients coeffs;
        // 简化实现 - 实际应用中需要更复杂的符号处理
        coeffs.first = 1.0;  // x^2的系数
        coeffs.second = 0.0; // x的系数
        coeffs.third = 0.0;  // 常数项
        return coeffs;
    }
};

// ============================================================================
// 表达式求值器
// ============================================================================

class Evaluator : public ExpressionVisitor {
private:
    std::map<std::string, double> variables_;
    double result_;

public:
    explicit Evaluator(const std::map<std::string, double>& variables) 
        : variables_(variables) {}
    
    double evaluate(const Expression& expr) {
        const_cast<Expression&>(expr).accept(*this);
        return result_;
    }
    
    void visit(Number& number) override {
        result_ = number.getValue();
    }
    
    void visit(Symbol& symbol) override {
        auto it = variables_.find(symbol.getName());
        if (it != variables_.end()) {
            result_ = it->second;
        } else {
            throw std::runtime_error("Undefined variable: " + symbol.getName());
        }
    }
    
    void visit(Add& add) override {
        Evaluator left_eval(variables_);
        double left_val = left_eval.evaluate(add.getLeft());
        
        Evaluator right_eval(variables_);
        double right_val = right_eval.evaluate(add.getRight());
        
        result_ = left_val + right_val;
    }
    
    void visit(Subtract& subtract) override {
        Evaluator left_eval(variables_);
        double left_val = left_eval.evaluate(subtract.getLeft());
        
        Evaluator right_eval(variables_);
        double right_val = right_eval.evaluate(subtract.getRight());
        
        result_ = left_val - right_val;
    }
    
    void visit(Multiply& multiply) override {
        Evaluator left_eval(variables_);
        double left_val = left_eval.evaluate(multiply.getLeft());
        
        Evaluator right_eval(variables_);
        double right_val = right_eval.evaluate(multiply.getRight());
        
        result_ = left_val * right_val;
    }
    
    void visit(Divide& divide) override {
        Evaluator left_eval(variables_);
        double left_val = left_eval.evaluate(divide.getLeft());
        
        Evaluator right_eval(variables_);
        double right_val = right_eval.evaluate(divide.getRight());
        
        if (std::abs(right_val) < 1e-10) {
            throw std::runtime_error("Division by zero");
        }
        
        result_ = left_val / right_val;
    }
    
    void visit(Power& power) override {
        Evaluator left_eval(variables_);
        double left_val = left_eval.evaluate(power.getLeft());
        
        Evaluator right_eval(variables_);
        double right_val = right_eval.evaluate(power.getRight());
        
        result_ = std::pow(left_val, right_val);
    }
    
    void visit(Function& function) override {
        Evaluator arg_eval(variables_);
        double arg_val = arg_eval.evaluate(function.getArgument());
        
        const std::string& name = function.getName();
        if (name == "sin") {
            result_ = std::sin(arg_val);
        } else if (name == "cos") {
            result_ = std::cos(arg_val);
        } else if (name == "tan") {
            result_ = std::tan(arg_val);
        } else if (name == "log") {
            if (arg_val <= 0) {
                throw std::runtime_error("Logarithm of non-positive number");
            }
            result_ = std::log(arg_val);
        } else if (name == "exp") {
            result_ = std::exp(arg_val);
        } else if (name == "sqrt") {
            if (arg_val < 0) {
                throw std::runtime_error("Square root of negative number");
            }
            result_ = std::sqrt(arg_val);
        } else {
            throw std::runtime_error("Unknown function: " + name);
        }
    }
};

// ============================================================================
// 主要CAS接口类
// ============================================================================

class CAS {
public:
    // 解析表达式
    static ExprPtr parse(const std::string& expression) {
        Parser parser(expression);
        return parser.parse();
    }
    
    // 化简表达式
    static ExprPtr simplify(const Expression& expr) {
        return expr.simplify();
    }
    
    // 求导
    static ExprPtr differentiate(const Expression& expr, const std::string& variable) {
        DerivativeVisitor visitor(variable);
        return visitor.differentiate(expr);
    }
    
    // 求值（代入数值）
    static double evaluate(const Expression& expr, const std::map<std::string, double>& values) {
        Evaluator evaluator(values);
        return evaluator.evaluate(expr);
    }
    
    // 解方程
    static std::vector<double> solveEquation(const Expression& equation, const std::string& variable) {
        // 简化实现 - 实际中需要分析方程类型
        try {
            return EquationSolver::solveQuadratic(equation, variable);
        } catch (...) {
            return EquationSolver::solveLinear(equation, variable);
        }
    }
    
    // 格式化输出
    static std::string toString(const Expression& expr) {
        return expr.toString();
    }
    
    // LaTeX输出
    static std::string toLaTeX(const Expression& expr) {
        // 简化实现 - 实际中需要完整的LaTeX转换
        std::string str = expr.toString();
        // 替换一些特殊字符
        size_t pos = 0;
        while ((pos = str.find("*", pos)) != std::string::npos) {
            str.replace(pos, 1, " \\cdot ");
            pos += 7;
        }
        return str;
    }
    
    // 创建基本表达式
    static ExprPtr number(double value) {
        return std::make_unique<Number>(value);
    }
    
    static ExprPtr symbol(const std::string& name) {
        return std::make_unique<Symbol>(name);
    }
    
    static ExprPtr add(ExprPtr left, ExprPtr right) {
        return std::make_unique<Add>(std::move(left), std::move(right));
    }
    
    static ExprPtr subtract(ExprPtr left, ExprPtr right) {
        return std::make_unique<Subtract>(std::move(left), std::move(right));
    }
    
    static ExprPtr multiply(ExprPtr left, ExprPtr right) {
        return std::make_unique<Multiply>(std::move(left), std::move(right));
    }
    
    static ExprPtr divide(ExprPtr left, ExprPtr right) {
        return std::make_unique<Divide>(std::move(left), std::move(right));
    }
    
    static ExprPtr power(ExprPtr base, ExprPtr exponent) {
        return std::make_unique<Power>(std::move(base), std::move(exponent));
    }
    
    static ExprPtr function(const std::string& name, ExprPtr argument) {
        return std::make_unique<Function>(name, std::move(argument));
    }
};

// ============================================================================
// 重载运算符以方便使用
// ============================================================================

inline ExprPtr operator+(ExprPtr left, ExprPtr right) {
    return std::make_unique<Add>(std::move(left), std::move(right));
}

inline ExprPtr operator-(ExprPtr left, ExprPtr right) {
    return std::make_unique<Subtract>(std::move(left), std::move(right));
}

inline ExprPtr operator*(ExprPtr left, ExprPtr right) {
    return std::make_unique<Multiply>(std::move(left), std::move(right));
}

inline ExprPtr operator/(ExprPtr left, ExprPtr right) {
    return std::make_unique<Divide>(std::move(left), std::move(right));
}

// 用于数字与表达式的运算
inline ExprPtr operator+(double left, const ExprPtr& right) {
    return std::make_unique<Add>(std::make_unique<Number>(left), right->clone());
}

inline ExprPtr operator+(const ExprPtr& left, double right) {
    return std::make_unique<Add>(left->clone(), std::make_unique<Number>(right));
}

inline ExprPtr operator-(double left, const ExprPtr& right) {
    return std::make_unique<Subtract>(std::make_unique<Number>(left), right->clone());
}

inline ExprPtr operator-(const ExprPtr& left, double right) {
    return std::make_unique<Subtract>(left->clone(), std::make_unique<Number>(right));
}

inline ExprPtr operator*(double left, const ExprPtr& right) {
    return std::make_unique<Multiply>(std::make_unique<Number>(left), right->clone());
}

inline ExprPtr operator*(const ExprPtr& left, double right) {
    return std::make_unique<Multiply>(left->clone(), std::make_unique<Number>(right));
}

inline ExprPtr operator/(double left, const ExprPtr& right) {
    return std::make_unique<Divide>(std::make_unique<Number>(left), right->clone());
}

inline ExprPtr operator/(const ExprPtr& left, double right) {
    return std::make_unique<Divide>(left->clone(), std::make_unique<Number>(right));
}