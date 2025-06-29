#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <climits>

class BigInt {
private:
    std::vector<int> digits;  // 存储数字，低位在前
    bool negative;

public:
    // 构造函数
    BigInt() : negative(false) { digits.push_back(0); }
    
    BigInt(int n) : negative(n < 0) {
        if (n == 0) {
            digits.push_back(0);
            return;
        }
        
        // 处理 INT_MIN 的特殊情况
        if (n == INT_MIN) {
            // INT_MIN 的绝对值超出 int 范围，直接处理
            long long ln = static_cast<long long>(n);
            ln = -ln;  // 现在安全了
            while (ln > 0) {
                digits.push_back(ln % 10);
                ln /= 10;
            }
        } else {
            n = std::abs(n);
            while (n > 0) {
                digits.push_back(n % 10);
                n /= 10;
            }
        }
    }
    
    BigInt(const std::string& str) : negative(false) {
        if (str.empty() || str == "0") {
            digits.push_back(0);
            return;
        }
        
        int start = 0;
        if (str[0] == '-') {
            negative = true;
            start = 1;
        } else if (str[0] == '+') {
            start = 1;
        }
        
        for (int i = str.length() - 1; i >= start; i--) {
            if (str[i] >= '0' && str[i] <= '9') {
                digits.push_back(str[i] - '0');
            }
        }
        
        if (digits.empty()) {
            digits.push_back(0);
        }
        
        remove_leading_zeros();
    }
    
    // 移除前导零
    void remove_leading_zeros() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.size() == 1 && digits[0] == 0) {
            negative = false;
        }
    }
    
    // 转换为字符串
    std::string to_string() const {
        if (digits.size() == 1 && digits[0] == 0) {
            return "0";
        }
        
        std::string result;
        if (negative) result += "-";
        
        for (int i = digits.size() - 1; i >= 0; i--) {
            result += char('0' + digits[i]);
        }
        
        return result;
    }
    
    // 乘法
    BigInt operator*(const BigInt& other) const {
        BigInt result;
        result.digits.assign(digits.size() + other.digits.size(), 0);
        result.negative = (negative != other.negative);
        
        for (size_t i = 0; i < digits.size(); i++) {
            for (size_t j = 0; j < other.digits.size(); j++) {
                result.digits[i + j] += digits[i] * other.digits[j];
                if (result.digits[i + j] >= 10) {
                    result.digits[i + j + 1] += result.digits[i + j] / 10;
                    result.digits[i + j] %= 10;
                }
            }
        }
        
        result.remove_leading_zeros();
        return result;
    }
    
    // 转换为int（如果可能）
    int to_int() const {
        if (digits.size() > 10) {  // 太大了
            return negative ? INT_MIN : INT_MAX;
        }
        
        long long result = 0;
        long long multiplier = 1;
        
        for (int digit : digits) {
            result += digit * multiplier;
            multiplier *= 10;
            if (result > INT_MAX) {
                return negative ? INT_MIN : INT_MAX;
            }
        }
        
        return negative ? -static_cast<int>(result) : static_cast<int>(result);
    }
    
    // 检查是否为零
    bool is_zero() const {
        return digits.size() == 1 && digits[0] == 0;
    }
    
    // 比较运算符
    bool operator==(const BigInt& other) const {
        return negative == other.negative && digits == other.digits;
    }
    
    bool operator!=(const BigInt& other) const {
        return !(*this == other);
    }
};
