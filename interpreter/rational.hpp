#pragma once
#include "bigint.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

class Rational {
private:
    BigInt numerator;
    BigInt denominator;

    // 求最大公约数
    static BigInt gcd(const BigInt& a, const BigInt& b) {
        BigInt abs_a = a;
        abs_a.negative = false;
        BigInt abs_b = b;
        abs_b.negative = false;

        while (!abs_b.is_zero()) {
            BigInt temp = abs_b;
            abs_b = abs_a % abs_b;
            abs_a = temp;
        }
        return abs_a;
    }

    // 化简分数
    void simplify() {
        if (denominator.is_zero()) {
            throw std::runtime_error("Denominator cannot be zero");
        }

        // 确保分母为正数
        if (denominator.negative) {
            numerator.negative = !numerator.negative;
            denominator.negative = false;
        }

        // 化简
        BigInt g = gcd(numerator, denominator);
        if (!g.is_zero() && !(g.digits.size() == 1 && g.digits[0] == 1)) {
            numerator = numerator / g;
            denominator = denominator / g;
        }
    }

public:
    // 构造函数
    Rational() : numerator(0), denominator(1) {}

    Rational(const BigInt& num) : numerator(num), denominator(1) {}

    Rational(const BigInt& num, const BigInt& den) : numerator(num), denominator(den) {
        simplify();
    }

    Rational(int num) : numerator(num), denominator(1) {}

    Rational(int num, int den) : numerator(num), denominator(den) {
        simplify();
    }

    // 由double构造
    static Rational from_double(double value) {
        if (value == 0.0) {
            return Rational();
        }
        if (std::floor(value) == value) {// 是整数，分母设为1
            return Rational(BigInt(std::to_string(value)));
        }
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(15) << value;
        std::string str = oss.str();

        // 解析科学记数法
        size_t e_pos = str.find('e');
        std::string base = str.substr(0, e_pos);
        bool negative = false;
        if (base[0] == '-') {
            negative = true;
            base.erase(0, 1);
        }
        base.erase(1, 1);           // 删除小数点
        while (base.back() == '0') {// 去除末尾0，不可能全为0
            base.pop_back();
        }
        int exponent = std::stoi(str.substr(e_pos + 1));

        size_t decimal_places = std::max(0, static_cast<int>(base.length()) - exponent - 1);// 有几位小数

        BigInt num(base);
        if (negative) num = num.negate();
        BigInt den("1" + std::string(decimal_places, '0'));

        Rational r(num, den);
        r.simplify();
        return r;
    }

    // 获取分子和分母
    BigInt get_numerator() const { return numerator; }
    BigInt get_denominator() const { return denominator; }

    // 判断是否为整数
    bool is_integer() const {
        return denominator.digits.size() == 1 && denominator.digits[0] == 1;
    }

    // 判断是否为零
    bool is_zero() const {
        return numerator.is_zero();
    }

    // 转换为字符串
    std::string to_string() const {
        if (is_integer()) {
            return numerator.to_string();
        }
        return numerator.to_string() + "/" + denominator.to_string();
    }

    // 转换为 BigInt（如果是整数）
    BigInt to_bigint() const {
        if (!is_integer()) {
            throw std::runtime_error("Cannot convert non-integer fraction to BigInt");
        }
        return numerator;
    }

    // 转换为 double（近似值）
    double to_double() const {
        return static_cast<double>(numerator.to_int()) / static_cast<double>(denominator.to_int());
    }

    // 加法
    Rational operator+(const Rational& other) const {
        BigInt new_num = numerator * other.denominator + other.numerator * denominator;
        BigInt new_den = denominator * other.denominator;
        return Rational(new_num, new_den);
    }

    // 减法
    Rational operator-(const Rational& other) const {
        BigInt new_num = numerator * other.denominator - other.numerator * denominator;
        BigInt new_den = denominator * other.denominator;
        return Rational(new_num, new_den);
    }

    // 乘法
    Rational operator*(const Rational& other) const {
        BigInt new_num = numerator * other.numerator;
        BigInt new_den = denominator * other.denominator;
        return Rational(new_num, new_den);
    }

    // 除法
    Rational operator/(const Rational& other) const {
        if (other.is_zero()) {
            throw std::runtime_error("Division by zero");
        }
        BigInt new_num = numerator * other.denominator;
        BigInt new_den = denominator * other.numerator;
        return Rational(new_num, new_den);
    }

    // 幂运算
    Rational power(const BigInt& exponent) const {
        if (exponent.negative) {
            // 负指数：(a/b)^(-n) = (b/a)^n
            if (is_zero()) {
                throw std::runtime_error("Cannot raise zero to negative power");
            }
            BigInt pos_exp = exponent;
            pos_exp.negative = false;
            return Rational(denominator.power(pos_exp), numerator.power(pos_exp));
        }

        if (exponent.is_zero()) {
            if (is_zero()) {
                throw std::runtime_error("0^0 is undefined");
            }
            return Rational(1);
        }

        return Rational(numerator.power(exponent), denominator.power(exponent));
    }

    // 比较运算符
    bool operator==(const Rational& other) const {
        return numerator * other.denominator == other.numerator * denominator;
    }

    bool operator!=(const Rational& other) const {
        return !(*this == other);
    }

    bool operator<(const Rational& other) const {
        BigInt left = numerator * other.denominator;
        BigInt right = other.numerator * denominator;
        return left < right;
    }

    bool operator<=(const Rational& other) const {
        return *this < other || *this == other;
    }

    bool operator>(const Rational& other) const {
        return !(*this <= other);
    }

    bool operator>=(const Rational& other) const {
        return !(*this < other);
    }

    // 取倒数
    Rational reciprocal() const {
        if (is_zero()) {
            throw std::runtime_error("Cannot take reciprocal of zero");
        }
        return Rational(denominator, numerator);
    }

    // 取绝对值
    Rational abs() const {
        Rational result = *this;
        result.numerator.negative = false;
        return result;
    }

    // 取负值
    Rational operator-() const {
        Rational result = *this;
        result.numerator.negative = !result.numerator.negative;
        return result;
    }
};
