#pragma once
#include <numeric>
#include <string>
#include <iostream>

class Rational {
private:
    long long numerator;
    long long denominator;
    
    // 最大公约数 (Euclidean algorithm)
    static long long gcd(long long a, long long b) {
        a = std::abs(a);
        b = std::abs(b);
        while (b != 0) {
            long long temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    // 化简分数
    void simplify() {
        if (denominator == 0) {
            throw std::runtime_error("Rational: denominator cannot be zero");
        }
        
        // 确保分母为正
        if (denominator < 0) {
            numerator = -numerator;
            denominator = -denominator;
        }
        
        // 化简
        long long g = gcd(numerator, denominator);
        if (g > 0) {
            numerator /= g;
            denominator /= g;
        }
    }
    
public:
    // 构造函数
    Rational() : numerator(0), denominator(1) {}
    Rational(long long num) : numerator(num), denominator(1) {}
    Rational(long long num, long long den) : numerator(num), denominator(den) {
        simplify();
    }
    
    // 从double创建有理数（近似）
    static Rational from_double(double value, long long max_denominator = 1000000) {
        if (value == 0.0) return Rational(0, 1);
        
        bool negative = value < 0;
        if (negative) value = -value;
        
        long long integer_part = static_cast<long long>(value);
        double fractional_part = value - integer_part;
        
        if (fractional_part < 1e-15) {
            return negative ? Rational(-integer_part, 1) : Rational(integer_part, 1);
        }
        
        // 简单的十进制转换法
        // 先尝试常见的简单分数
        static const std::pair<double, std::pair<long long, long long>> common_fractions[] = {
            {0.5, {1, 2}},
            {0.25, {1, 4}},
            {0.75, {3, 4}},
            {0.125, {1, 8}},
            {0.375, {3, 8}},
            {0.625, {5, 8}},
            {0.875, {7, 8}},
            {0.1, {1, 10}},
            {0.2, {1, 5}},
            {0.3, {3, 10}},
            {0.4, {2, 5}},
            {0.6, {3, 5}},
            {0.7, {7, 10}},
            {0.8, {4, 5}},
            {0.9, {9, 10}},
            {0.333333, {1, 3}},
            {0.666667, {2, 3}}
        };
        
        const double tolerance = 1e-6;
        for (const auto& frac : common_fractions) {
            if (std::abs(fractional_part - frac.first) < tolerance) {
                long long num = integer_part * frac.second.second + frac.second.first;
                return negative ? Rational(-num, frac.second.second) : Rational(num, frac.second.second);
            }
        }
        
        // 如果不是常见分数，使用连分数展开
        double x = fractional_part;
        long long num = 0, den = 1;
        long long prev_num = 1, prev_den = 0;
        
        for (int i = 0; i < 20 && den <= max_denominator; i++) {
            if (x < 1e-15) break;
            
            long long a = static_cast<long long>(1.0 / x);
            long long temp_num = a * num + prev_num;
            long long temp_den = a * den + prev_den;
            
            if (temp_den > max_denominator) break;
            
            prev_num = num;
            prev_den = den;
            num = temp_num;
            den = temp_den;
            
            double new_x = 1.0 / x - a;
            if (std::abs(new_x) < 1e-15) break;
            x = new_x;
        }
        
        num = integer_part * den + num;
        return negative ? Rational(-num, den) : Rational(num, den);
    }
    
    // 访问器
    long long get_numerator() const { return numerator; }
    long long get_denominator() const { return denominator; }
    
    // 判断是否为整数
    bool is_integer() const { return denominator == 1; }
    
    // 转换为double
    double to_double() const {
        return static_cast<double>(numerator) / static_cast<double>(denominator);
    }
    
    // 转换为字符串
    std::string to_string() const {
        if (denominator == 1) {
            return std::to_string(numerator);
        }
        return std::to_string(numerator) + "/" + std::to_string(denominator);
    }
    
    // 算术运算
    Rational operator+(const Rational& other) const {
        return Rational(
            numerator * other.denominator + other.numerator * denominator,
            denominator * other.denominator
        );
    }
    
    Rational operator-(const Rational& other) const {
        return Rational(
            numerator * other.denominator - other.numerator * denominator,
            denominator * other.denominator
        );
    }
    
    Rational operator*(const Rational& other) const {
        return Rational(
            numerator * other.numerator,
            denominator * other.denominator
        );
    }
    
    Rational operator/(const Rational& other) const {
        if (other.numerator == 0) {
            throw std::runtime_error("Rational: division by zero");
        }
        return Rational(
            numerator * other.denominator,
            denominator * other.numerator
        );
    }
    
    Rational operator-() const {
        return Rational(-numerator, denominator);
    }
    
    // 比较运算
    bool operator==(const Rational& other) const {
        return numerator == other.numerator && denominator == other.denominator;
    }
    
    bool operator!=(const Rational& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Rational& other) const {
        return numerator * other.denominator < other.numerator * denominator;
    }
    
    bool operator<=(const Rational& other) const {
        return *this < other || *this == other;
    }
    
    bool operator>(const Rational& other) const {
        return other < *this;
    }
    
    bool operator>=(const Rational& other) const {
        return *this > other || *this == other;
    }
    
    // 幂运算
    Rational pow(int exponent) const {
        if (exponent == 0) return Rational(1, 1);
        if (exponent > 0) {
            long long num = 1, den = 1;
            for (int i = 0; i < exponent; ++i) {
                num *= numerator;
                den *= denominator;
            }
            return Rational(num, den);
        } else {
            // 负指数
            if (numerator == 0) {
                throw std::runtime_error("Rational: 0 to negative power");
            }
            long long num = 1, den = 1;
            for (int i = 0; i < -exponent; ++i) {
                num *= denominator;
                den *= numerator;
            }
            return Rational(num, den);
        }
    }
    
    // 绝对值
    Rational abs() const {
        return Rational(std::abs(numerator), denominator);
    }
    
    // 判断是否为零
    bool is_zero() const {
        return numerator == 0;
    }
    
    // 判断是否为正数
    bool is_positive() const {
        return numerator > 0;
    }
    
    // 判断是否为负数
    bool is_negative() const {
        return numerator < 0;
    }
    
    // 转换为最简分数的字符串表示（带括号用于复杂表达式）
    std::string to_string_parenthesized() const {
        if (denominator == 1) {
            return std::to_string(numerator);
        }
        if (numerator < 0 || denominator < 0) {
            return "(" + std::to_string(numerator) + "/" + std::to_string(denominator) + ")";
        }
        return std::to_string(numerator) + "/" + std::to_string(denominator);
    }
    
    // 取倒数
    Rational reciprocal() const {
        if (numerator == 0) {
            throw std::runtime_error("Rational: reciprocal of zero");
        }
        return Rational(denominator, numerator);
    }
    
    // 求最大公约数（静态函数，供外部调用）
    static long long compute_gcd(long long a, long long b) {
        return gcd(a, b);
    }
    
    // 求最小公倍数
    static long long lcm(long long a, long long b) {
        return std::abs(a * b) / gcd(a, b);
    }
    
    // 输出流重载
    friend std::ostream& operator<<(std::ostream& os, const Rational& r) {
        os << r.to_string();
        return os;
    }
};
