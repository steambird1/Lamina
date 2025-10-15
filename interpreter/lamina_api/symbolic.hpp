#pragma once
#include "bigint.hpp"
#include "rational.hpp"
#include <memory>
#include <string>
#include <variant>
#include <cmath>
#include <limits>
#include <algorithm>
#include <map>
#include <functional>
#include <iostream>

#define _SYMBOLIC_DEBUG 1

#ifdef _WIN32
#ifdef LAMINA_CORE_EXPORTS
#define LAMINA_API __declspec(dllexport)
#else
#define LAMINA_API __declspec(dllimport)
#endif
#else
#define LAMINA_API
#endif

#if _SYMBOLIC_DEBUG
#define err_stream std::cerr
#else
#define err_stream if (0) std::cerr
/*
class _NullBuffer : public std::streambuf {
	public:
		virtual int overflow(int c) {
			return c;
		}
};
_NullBuffer nbf;
std::ostream nullstream(&nbf);
#define err_stream nullstream
*/
#endif

// 符号表达式系统
// 支持精确的数学表达式，不进行数值近似

class LAMINA_API SymbolicExpr {
public:
    enum class Type {
        Number,      // 数字 (BigInt, Rational, int)
        Sqrt,        // 平方根 √
        Root,        // n次方根 √[n]，未使用
        Power,       // 幂次 ^
        Multiply,    // 乘法 *
        Add,         // 加法 +
        Subtract,    // 减法 -，未使用
        Infinity,      // 无限大（number_value 中的 int 决定是正或负）
        Variable     // 变量 (如 π, e)

    };
	
	/*
	哈希要保证的一些因素：
	- 乘法交换律（乘法：奇数二进制位参与运算）
	- 加法交换律（加法：偶数二进制位参与运算）
	- 加法和乘法区分
	这个函数在加法化简时使用。
	*/
	struct HashData {
#define _HASH_PARAMS		ODDBIT, EVENBIT, SQRBIT, HALFBIT
		using HashType = unsigned long long;
		// TODO: 允许多个 HashData 对象之间的不同以减少哈希冲突概率
		// 下方变量的名字不重要。
#define ODDBIT_D 0x555555555555555ull
#define EVENBIT_D 0xAAAAAAAAAAAAAAAull
#define SQRBIT_D 0xBDEEBD77BDEEBD7ull
#define HALFBIT_D 0x7EDB7EDB7EDB7EDBull
#define EMPTY 0ull
#define INFINITY_D 0xFFF7FFFFDEADBEEFull
#define PI_H 0x1451419810C0000ull
#define E_H 0x9198101145C0000ull
#define UNKNOWN_H 0xAD0AA0BEEFC0000ull
		
		HashType ODDBIT;
		HashType EVENBIT;
		HashType SQRBIT;
		HashType HALFBIT;
		
		::Rational k = ::Rational(1), ksqrt = ::Rational(1);
		HashType hash = EMPTY;
		std::shared_ptr<SymbolicExpr> hash_obj = SymbolicExpr::number(1);	// 因为是乘法，1为默认状态。此处存储被 hash 的项目对应的值
		
		static HashType bigint_hash(const BigInt& rt) {
			// 直接哈希所有 digits
			SymbolicExpr::HashData::HashType weight = 1ull, ans = 0ull;
			for (auto &i : rt.digits) {
				ans = ans * weight + (i + 3ull);
				weight *= 17ull;			// 不用 10 减少哈希冲突
			}
			if (rt.negative) return ~ans;
			return ans;
		}
		
		static HashType rational_hash(const Rational& rt) {
			return bigint_hash(rt.get_numerator()) ^ bigint_hash(rt.get_denominator());
		}
		
		HashType to_single_hash();
		
		// TODO: 考虑优化
		std::shared_ptr<SymbolicExpr> get_combined_k() {
			return SymbolicExpr::multiply(SymbolicExpr::number(k), SymbolicExpr::sqrt(SymbolicExpr::number(ksqrt)))->simplify();
		}
		
		HashData() {
			
		}
		
		HashData(std::shared_ptr<SymbolicExpr> obj, 
			HashType ODDBIT = ODDBIT_D, HashType EVENBIT = EVENBIT_D, HashType SQRBIT = SQRBIT_D, HashType HALFBIT = HALFBIT_D);
	
	};

    Type type;

    // 数值存储
    std::variant<int, ::BigInt, ::Rational> number_value;

    // 表达式参数
    std::vector<std::shared_ptr<SymbolicExpr>> operands;

    // 字符串标识（用于变量名或操作符）
    std::string identifier;

	// 是否已经化简完成
	bool already_simplified = false;

    // 构造函数
    SymbolicExpr(Type t) : type(t) {}

    // 数字构造函数
    static std::shared_ptr<SymbolicExpr> number(int n) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Number);
        expr->number_value = n;
        return expr;
    }

    static std::shared_ptr<SymbolicExpr> number(const ::BigInt& bi) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Number);
        expr->number_value = bi;
        return expr;
    }

    static std::shared_ptr<SymbolicExpr> number(const ::Rational& r) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Number);
        expr->number_value = r;
        return expr;
    }

	static std::shared_ptr<SymbolicExpr> infinity(int k = 1) {
		auto expr = std::make_shared<SymbolicExpr>(Type::Infinity);
		expr->number_value = k;
		return expr;
	}

    // 平方根构造函数
    static std::shared_ptr<SymbolicExpr> sqrt(std::shared_ptr<SymbolicExpr> operands) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Sqrt);
        expr->operands.push_back(operands);
        return expr;
    }

    // 乘法构造函数
    static std::shared_ptr<SymbolicExpr> multiply(std::shared_ptr<SymbolicExpr> left, std::shared_ptr<SymbolicExpr> right) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Multiply);
        if (right->is_number()) {
            // 将数字移至前端
            expr->operands.push_back(right);
            expr->operands.push_back(left);
            return expr;
        }
        expr->operands.push_back(left);
        expr->operands.push_back(right);
        return expr;
    }

    // 加法构造函数
    static std::shared_ptr<SymbolicExpr> add(std::shared_ptr<SymbolicExpr> left, std::shared_ptr<SymbolicExpr> right) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Add);
        expr->operands.push_back(left);
        expr->operands.push_back(right);
        return expr;
    }

    // 幂次构造函数
    static std::shared_ptr<SymbolicExpr> power(std::shared_ptr<SymbolicExpr> base, std::shared_ptr<SymbolicExpr> exponent) {

        // 直接符号储存
        auto expr = std::make_shared<SymbolicExpr>(Type::Power);
        expr->operands.push_back(base);
        expr->operands.push_back(exponent);
        return expr;
    }

    // 变量构造函数
    static std::shared_ptr<SymbolicExpr> variable(const std::string& name) {
        auto expr = std::make_shared<SymbolicExpr>(Type::Variable);
        expr->identifier = name;
        return expr;
    }

    // 化简表达式
    std::shared_ptr<SymbolicExpr> simplify() const;

    // 转换为字符串表示
    std::string to_string() const;

    // 检查是否为数字
    bool is_number() const { return type == Type::Number; }

    // 检查是否为大整数
    bool is_big_int() const { return is_number() && std::holds_alternative<::BigInt>(number_value); }

    // 检查是否为分数（有理数）
    bool is_rational() const { return is_number() && std::holds_alternative<::Rational>(number_value); }

    // 检查是否为整数
    bool is_int() const { return is_number() && std::holds_alternative<int>(number_value); }

    // 获取数字值（如果是数字的话）
    std::variant<int, ::BigInt, ::Rational> get_number() const {
        if (is_number()) {
            return number_value;
        }
        throw std::runtime_error("Expression is not a number");
    }

    int get_int() const {
        if (is_int()) {
            return std::get<int>(get_number());
        }
        throw std::runtime_error("Expression is not a int");
    }
    ::BigInt get_big_int() const {
        if (is_big_int()) {
            return std::get<BigInt>(get_number());
        }
        throw std::runtime_error("Expression is not a BigInt");
    }
    ::Rational get_rational() const {
        if (is_rational()) {
            return std::get<Rational>(get_number());
        }
        throw std::runtime_error("Expression is not a Rational");
    }
    ::Rational convert_rational() const {
		if (!is_number()) {
			throw std::runtime_error("Expression cannot be converted into Rational");
		}
		if (is_rational()) return get_rational();
		else if (is_big_int()) return ::Rational(get_big_int());
		else return ::Rational(get_int());
	}
	

    // 尝试计算数值（如果可能的话）
    double to_double() const;

private:
    // 内部化简函数
    std::shared_ptr<SymbolicExpr> simplify_sqrt() const;
    std::shared_ptr<SymbolicExpr> simplify_multiply() const;
    std::shared_ptr<SymbolicExpr> simplify_add() const;
    std::shared_ptr<SymbolicExpr> simplify_power() const;
};
