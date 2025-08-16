#include "symbolic.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <functional>

// 符号表达式的化简实现
std::shared_ptr<SymbolicExpr> SymbolicExpr::simplify() const {
    switch (type) {
        case Type::Number:
        case Type::Variable:
            return std::make_shared<SymbolicExpr>(*this);
            
        case Type::Sqrt:
            return simplify_sqrt();
            
        case Type::Multiply:
            return simplify_multiply();
            
        case Type::Add:
            return simplify_add();

        case Type::Power:
            return simplify_power();
        
        default:
            return std::make_shared<SymbolicExpr>(*this);
    }
}

std::shared_ptr<SymbolicExpr> SymbolicExpr::simplify_sqrt() const {
    if (operands.empty()) return std::make_shared<SymbolicExpr>(*this);
    
    auto simplified_operand = operands[0]->simplify();
    
    // 如果操作数是数字，尝试化简
    if (simplified_operand->is_number()) {
        auto num_val = simplified_operand->get_number();
        
        // 处理不同的数字类型
        if (std::holds_alternative<int>(num_val)) {
            int n = std::get<int>(num_val);
            if (n < 0) {
                throw std::runtime_error("Square root of negative number");
            }
            if (n == 0 || n == 1) {
                return SymbolicExpr::number(n);
            }
            
            // 检查是否为完全平方数
            int sqrt_n = static_cast<int>(std::sqrt(n));
            if (sqrt_n * sqrt_n == n) {
                return SymbolicExpr::number(sqrt_n);
            }
            
            // 提取完全平方因子
            int factor = 1;
            int remaining = n;
            for (int i = 2; i * i <= remaining; ++i) {
                while (remaining % (i * i) == 0) {
                    factor *= i;
                    remaining /= (i * i);
                }
            }
            
            if (factor > 1) {
                if (remaining == 1) {
                    return SymbolicExpr::number(factor);
                } else {
                    // 返回 factor * √remaining
                    return SymbolicExpr::multiply(
                        SymbolicExpr::number(factor),
                        SymbolicExpr::sqrt(SymbolicExpr::number(remaining))
                    );
                }
            }
        }
        
        // 处理BigInt
        if (std::holds_alternative<::BigInt>(num_val)) {
            const auto& bi = std::get<::BigInt>(num_val);
            if (bi.is_perfect_square()) {
                return SymbolicExpr::number(bi.sqrt());
            }
            // 对于非完全平方的BigInt，保持符号形式
        }
    }
    
    // 无法进一步化简，返回 √(simplified_operand)
    return SymbolicExpr::sqrt(simplified_operand);
}

std::shared_ptr<SymbolicExpr> SymbolicExpr::simplify_multiply() const {
    if (operands.size() != 2) return std::make_shared<SymbolicExpr>(*this);
    
    auto left = operands[0]->simplify();
    auto right = operands[1]->simplify();
    
    // 如果两个操作数都是数字，直接相乘
    if (left->is_number() && right->is_number()) {
        auto left_num = left->get_number();
        auto right_num = right->get_number();
        
        // 简化：只处理整数乘法
        if (std::holds_alternative<int>(left_num) && std::holds_alternative<int>(right_num)) {
            int result = std::get<int>(left_num) * std::get<int>(right_num);
            return SymbolicExpr::number(result);
        }
    }
    
    return SymbolicExpr::multiply(left, right);
}

std::shared_ptr<SymbolicExpr> SymbolicExpr::simplify_add() const {
    if (operands.size() != 2) return std::make_shared<SymbolicExpr>(*this);

    auto left = operands[0]->simplify();
    auto right = operands[1]->simplify();

    // 如果两个操作数都是数字，直接相加
    if (left->is_number() && right->is_number()) {
        auto left_num = left->get_number();
        auto right_num = right->get_number();
        if (std::holds_alternative<int>(left_num) && std::holds_alternative<int>(right_num)) {
            int result = std::get<int>(left_num) + std::get<int>(right_num);
            return SymbolicExpr::number(result);
        }
        // Rational 加法
        if (std::holds_alternative<::Rational>(left_num) && std::holds_alternative<::Rational>(right_num)) {
            ::Rational result = std::get<::Rational>(left_num) + std::get<::Rational>(right_num);
            return SymbolicExpr::number(result);
        }
    }

    // 根式同类项合并：如 a√n + b√n = (a+b)√n
    auto extract_sqrt = [](const std::shared_ptr<SymbolicExpr>& expr, ::Rational& coeff, int& radicand) -> bool {
        if (expr->type == SymbolicExpr::Type::Sqrt && expr->operands.size() == 1 && expr->operands[0]->is_number()) {
            coeff = ::Rational(1);
            radicand = std::get<int>(expr->operands[0]->get_number());
            return true;
        }
        if (expr->type == SymbolicExpr::Type::Multiply && expr->operands.size() == 2) {
            // 左系数，右根式
            if (expr->operands[0]->is_number() && expr->operands[1]->type == SymbolicExpr::Type::Sqrt && expr->operands[1]->operands.size() == 1 && expr->operands[1]->operands[0]->is_number()) {
                coeff = std::holds_alternative<::Rational>(expr->operands[0]->get_number()) ? std::get<::Rational>(expr->operands[0]->get_number()) : ::Rational(std::get<int>(expr->operands[0]->get_number()));
                radicand = std::get<int>(expr->operands[1]->operands[0]->get_number());
                return true;
            }
        }
        return false;
    };

    // 递归合并同类项：如 √2+2√2+3√2 合并为 6√2
    std::vector<std::shared_ptr<SymbolicExpr>> terms;
    std::function<void(const std::shared_ptr<SymbolicExpr>&)> flatten_add;
    flatten_add = [&](const std::shared_ptr<SymbolicExpr>& expr) {
        if (expr->type == SymbolicExpr::Type::Add && expr->operands.size() == 2) {
            flatten_add(expr->operands[0]);
            flatten_add(expr->operands[1]);
        } else {
            terms.push_back(expr);
        }
    };
    flatten_add(left);
    flatten_add(right);

    // 合并所有根式同类项
    std::map<int, ::Rational> sqrt_terms; // radicand -> sum of coeffs
    std::vector<std::shared_ptr<SymbolicExpr>> others;
    for (const auto& term : terms) {
        ::Rational coeff;
        int radicand = 0;
        if (extract_sqrt(term, coeff, radicand)) {
            sqrt_terms[radicand] = sqrt_terms[radicand] + coeff;
        } else {
            others.push_back(term);
        }
    }
    std::vector<std::shared_ptr<SymbolicExpr>> result_terms;
    for (const auto& [radicand, coeff] : sqrt_terms) {
        if (coeff.is_integer() && coeff.get_numerator().to_int() == 0) continue;
        if (coeff == ::Rational(1)) {
            result_terms.push_back(SymbolicExpr::sqrt(SymbolicExpr::number(radicand)));
        } else {
            result_terms.push_back(SymbolicExpr::multiply(SymbolicExpr::number(coeff), SymbolicExpr::sqrt(SymbolicExpr::number(radicand))));
        }
    }
    result_terms.insert(result_terms.end(), others.begin(), others.end());
    if (result_terms.empty()) return SymbolicExpr::number(0);
    if (result_terms.size() == 1) return result_terms[0];
    // 多项式加法
    std::shared_ptr<SymbolicExpr> sum = result_terms[0];
    for (size_t i = 1; i < result_terms.size(); ++i) {
        sum = SymbolicExpr::add(sum, result_terms[i]);
    }
    return sum->simplify();
}

std::shared_ptr<SymbolicExpr> SymbolicExpr::simplify_power() const {
    // 如果底数是数字，且指数是整数，用数字储存
    auto base = operands[0]->simplify();
    auto exponent = operands[1]->simplify();
    if (base->is_number() && (exponent->is_int() || exponent->is_big_int())) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Number);
        // 底数是分数，结果为分数
        if (base->is_rational() || exponent->is_rational()) {
            if (exponent->is_int()) {
            expr->number_value = (base->get_rational()).power(BigInt(exponent->get_int()));
            } else if (exponent->is_big_int()) {
            expr->number_value = (base->get_rational()).power(exponent->get_big_int());
            }
        } else {// 否则结果为大整数
            BigInt b;
            if (base->is_int()) {
                b = BigInt(std::get<int>(base->get_number()));
            } else {
                b = std::get<BigInt>(base->get_number());
            }
            BigInt e;
            if (exponent->is_int()) {
                e = BigInt(std::get<int>(exponent->get_number()));
            } else {
                e = std::get<BigInt>(exponent->get_number());
            }
            if (e.to_int() >= 0) {
                expr->number_value = b.power(e);
            } else {
                
                expr->number_value = Rational(BigInt(1), b.power(e.negate()));
            }
        }

        return expr;
    } else if (base->is_number() && exponent->is_rational()) {
        // 底数是数字，指数是分数
        // TODO:暂时用符号储存
    }

    return SymbolicExpr::power(base, exponent);
}


std::string SymbolicExpr::to_string() const {
    switch (type) {
        case Type::Number:
            if (std::holds_alternative<int>(number_value)) {
                return std::to_string(std::get<int>(number_value));
            } else if (std::holds_alternative<::BigInt>(number_value)) {
                return std::get<::BigInt>(number_value).to_string();
            } else if (std::holds_alternative<::Rational>(number_value)) {
                return std::get<::Rational>(number_value).to_string();
            }
            return "0";
            
        case Type::Variable:
            return identifier;
            
        case Type::Sqrt:
            if (operands.empty()) return "√()";
            return "√" + operands[0]->to_string();
            
        case Type::Multiply:
            if (operands.size() < 2) return "*(?)";
            // 特殊处理：如果左操作数是数字且右操作数是平方根，使用紧凑形式
            if (operands[0]->is_number() && operands[1]->type == Type::Sqrt) {
                return operands[0]->to_string() + operands[1]->to_string();
            }
            return operands[0]->to_string() + "*" + operands[1]->to_string();
            
        case Type::Add: {
            if (operands.size() < 2) return "+(?)";
            // 展开并合并同类项，输出紧凑表达式
            std::vector<const SymbolicExpr*> terms;
            std::function<void(const SymbolicExpr*)> flatten_add;
            flatten_add = [&](const SymbolicExpr* expr) {
                if (expr->type == Type::Add && expr->operands.size() == 2) {
                    flatten_add(expr->operands[0].get());
                    flatten_add(expr->operands[1].get());
                } else {
                    terms.push_back(expr);
                }
            };
            flatten_add(this);
            // 合并根式同类项
            std::map<int, ::Rational> sqrt_terms;
            std::vector<std::string> others;
            auto extract_sqrt = [](const SymbolicExpr* expr, ::Rational& coeff, int& radicand) -> bool {
                if (expr->type == Type::Sqrt && expr->operands.size() == 1 && expr->operands[0]->is_number()) {
                    coeff = ::Rational(1);
                    radicand = std::get<int>(expr->operands[0]->get_number());
                    return true;
                }
                if (expr->type == Type::Multiply && expr->operands.size() == 2) {
                    if (expr->operands[0]->is_number() && expr->operands[1]->type == Type::Sqrt && expr->operands[1]->operands.size() == 1 && expr->operands[1]->operands[0]->is_number()) {
                        coeff = std::holds_alternative<::Rational>(expr->operands[0]->get_number()) ? std::get<::Rational>(expr->operands[0]->get_number()) : ::Rational(std::get<int>(expr->operands[0]->get_number()));
                        radicand = std::get<int>(expr->operands[1]->operands[0]->get_number());
                        return true;
                    }
                }
                return false;
            };
            for (const auto* term : terms) {
                ::Rational coeff;
                int radicand = 0;
                if (extract_sqrt(term, coeff, radicand)) {
                    sqrt_terms[radicand] = sqrt_terms[radicand] + coeff;
                } else {
                    others.push_back(term->to_string());
                }
            }
            std::vector<std::string> result_terms;
            for (const auto& [radicand, coeff] : sqrt_terms) {
                if (coeff.is_integer() && coeff.get_numerator().to_int() == 0) continue;
                if (coeff == ::Rational(1)) {
                    result_terms.push_back("√" + std::to_string(radicand));
                } else {
                    result_terms.push_back(coeff.to_string() + "√" + std::to_string(radicand));
                }
            }
            result_terms.insert(result_terms.end(), others.begin(), others.end());
            if (result_terms.empty()) return "0";
            std::string res = result_terms[0];
            for (size_t i = 1; i < result_terms.size(); ++i) {
                res += "+" + result_terms[i];
            }
            return res;
        }
            
        case Type::Power:
            if (operands.size() < 2) return "^(?)";
            return operands[0]->to_string() + "^" + operands[1]->to_string();
            
        default:
            return "Unknown";
    }
}

double SymbolicExpr::to_double() const {
    switch (type) {
        case Type::Number:
            if (std::holds_alternative<int>(number_value)) {
                return static_cast<double>(std::get<int>(number_value));
            } else if (std::holds_alternative<::BigInt>(number_value)) {
                return std::get<::BigInt>(number_value).to_double();
            } else if (std::holds_alternative<::Rational>(number_value)) {
                return std::get<::Rational>(number_value).to_double();
            }
            return 0.0;
            
        case Type::Sqrt:
            if (!operands.empty()) {
                return std::sqrt(operands[0]->to_double());
            }
            return 0.0;
            
        case Type::Multiply:
            if (operands.size() >= 2) {
                return operands[0]->to_double() * operands[1]->to_double();
            }
            return 0.0;
            
        default:
            return 0.0;
    }
}
