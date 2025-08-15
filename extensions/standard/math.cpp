#include "interpreter.hpp"
#include "lamina.hpp"
// #include "latex.hpp"
#include "value.hpp"
#include "symbolic.hpp"

/**
 * @brief 计算数值的平方根
 * 
 * 根据输入参数的类型（整数、大整数、有理数或浮点数）进行相应的处理：
 * - 对于整数和大整数，若为完全平方数则返回精确结果，否则返回符号表达式；
 * - 对于有理数，若分子分母均为完全平方数则返回精确结果，否则返回符号表达式；
 * - 对于浮点数等其他类型，使用标准库函数计算近似值。
 * 
 * @param args 参数列表，要求包含一个数值类型的参数
 * @return Value 平方根的结果，可能为数值或符号表达式
 */
inline Value sqrt(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        std::cerr << "Error: sqrt() requires numeric argument" << std::endl;
        return Value();
    }

    // Check for negative values first
    if (args[0].as_number() < 0) {
        std::cerr << "Error: sqrt() of negative number" << std::endl;
        return Value();
    }

    // Handle integer case - return symbolic result
    if (args[0].is_int()) {
        int val = std::get<int>(args[0].data);
        if (val == 0 || val == 1) {
            return Value(val);  // sqrt(0) = 0, sqrt(1) = 1
        }
        
        // Check if it's a perfect square
        int sqrt_val = static_cast<int>(std::sqrt(val));
        if (sqrt_val * sqrt_val == val) {
            return Value(sqrt_val);
        }
        
        // Return symbolic expression for non-perfect squares
        auto symbolic_arg = SymbolicExpr::number(val);
        auto symbolic_sqrt = SymbolicExpr::sqrt(symbolic_arg);
        return Value(symbolic_sqrt->simplify());
    }

    // Handle BigInt case - return symbolic result
    if (args[0].is_bigint()) {
        const auto& bi = std::get<::BigInt>(args[0].data);
        if (bi.is_zero()) {
            return Value(0);
        }
        
        // Check if it's a perfect square
        if (bi.is_perfect_square()) {
            return Value(bi.sqrt());
        }
        
        // Return symbolic expression for non-perfect squares
        auto symbolic_arg = SymbolicExpr::number(bi);
        auto symbolic_sqrt = SymbolicExpr::sqrt(symbolic_arg);
        return Value(symbolic_sqrt->simplify());
    }

    // Handle rational case
    if (args[0].is_rational()) {
        const auto& rat = std::get<::Rational>(args[0].data);
        long long num = rat.get_numerator();
        long long den = rat.get_denominator();
        
        if (num < 0) {
            std::cerr << "Error: sqrt() of negative number" << std::endl;
            return Value();
        }
        
        long long sqrt_num = static_cast<long long>(std::sqrt(num));
        long long sqrt_den = static_cast<long long>(std::sqrt(den));
        
        if (sqrt_num * sqrt_num == num && sqrt_den * sqrt_den == den) {
            // Both are perfect squares
            return Value(::Rational(sqrt_num, sqrt_den));
        }
        
        // Return symbolic expression for non-perfect squares
        auto symbolic_arg = SymbolicExpr::number(rat);
        auto symbolic_sqrt = SymbolicExpr::sqrt(symbolic_arg);
        return Value(symbolic_sqrt->simplify());
    }

    // For float and other types, use floating point (legacy behavior)
    double val = args[0].as_number();
    return Value(std::sqrt(val));
}

/**
 * @brief 返回圆周率π的值
 * 
 * @param args 无参数
 * @return Value 圆周率π的表示
 */
inline Value pi(const std::vector<Value>& /* args */) {
    return Value(::Irrational::pi());
}

/**
 * @brief 返回自然常数e的值
 * 
 * @param args 无参数
 * @return Value 自然常数e的表示
 */
inline Value e(const std::vector<Value>& /* args */) {
    return Value(::Irrational::e());
}

/**
 * @brief 计算数值的绝对值
 * 
 * 支持整数、大整数和浮点数等类型。对于大整数，调用其abs方法；其他类型转换为double后计算。
 * 
 * @param args 参数列表，要求包含一个数值类型的参数
 * @return Value 绝对值结果
 */
inline Value abs(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        std::cerr << "Error: abs() requires numeric argument" << std::endl;
        return Value();
    }
    
    // Handle BigInt specifically
    if (args[0].is_bigint()) {
        const auto& bigint_val = std::get<::BigInt>(args[0].data);
        return Value(bigint_val.abs());
    }
    
    // Handle other numeric types
    double val = args[0].as_number();
    return Value(std::abs(val));
}

/**
 * @brief 计算正弦值
 * 
 * @param args 参数列表，要求包含一个数值类型的参数（弧度）
 * @return Value 正弦值结果
 */
inline Value sin(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        std::cerr << "Error: sin() requires numeric argument" << std::endl;
        return Value();
    }
    return Value(std::sin(args[0].as_number()));
}

/**
 * @brief 计算余弦值
 * 
 * @param args 参数列表，要求包含一个数值类型的参数（弧度）
 * @return Value 余弦值结果
 */
inline Value cos(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        std::cerr << "Error: cos() requires numeric argument" << std::endl;
        return Value();
    }
    return Value(std::cos(args[0].as_number()));
}

/**
 * @brief 计算正切值
 * 
 * @param args 参数列表，要求包含一个数值类型的参数（弧度）
 * @return Value 正切值结果
 */
inline Value tan(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        std::cerr << "Error: tan() requires numeric argument" << std::endl;
        return Value();
    }
    return Value(std::tan(args[0].as_number()));
}

/**
 * @brief 计算自然对数
 * 
 * @param args 参数列表，要求包含一个正数类型的参数
 * @return Value 自然对数值结果
 */
inline Value log(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        error_and_exit("log() requires numeric argument");
    }
    double val = args[0].as_number();
    if (val <= 0) {
        error_and_exit("log() requires positive argument");
    }
    return Value(std::log(val));
}

/**
 * @brief 四舍五入到最近的整数
 * 
 * @param args 参数列表，要求包含一个数值类型的参数
 * @return Value 四舍五入后的整数结果
 */
inline Value round(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        error_and_exit("round() requires numeric argument");
    }
    return Value(static_cast<int>(std::round(args[0].as_number())));
}

/**
 * @brief 向下取整
 * 
 * @param args 参数列表，要求包含一个数值类型的参数
 * @return Value 向下取整后的整数结果
 */
inline Value floor(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        error_and_exit("floor() requires numeric argument");
    }
    return Value(static_cast<int>(std::floor(args[0].as_number())));
}

/**
 * @brief 向上取整
 * 
 * @param args 参数列表，要求包含一个数值类型的参数
 * @return Value 向上取整后的整数结果
 */
inline Value ceil(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        error_and_exit("ceil() requires numeric argument");
    }
    return Value(static_cast<int>(std::ceil(args[0].as_number())));
}

/**
 * @brief 计算两个向量的点积
 * 
 * @param args 参数列表，要求包含两个向量类型的参数
 * @return Value 点积结果
 */
inline Value dot(const std::vector<Value>& args) {
    return args[0].dot_product(args[1]);
}

/**
 * @brief 计算两个向量的叉积
 * 
 * @param args 参数列表，要求包含两个向量类型的参数
 * @return Value 叉积结果
 */
inline Value cross(const std::vector<Value>& args) {
    return args[0].cross_product(args[1]);
}

/**
 * @brief 计算向量的模长
 * 
 * @param args 参数列表，要求包含一个向量类型的参数
 * @return Value 模长结果
 */
inline Value norm(const std::vector<Value>& args) {
    return args[0].magnitude();
}

/**
 * @brief 将向量归一化
 * 
 * @param args 参数列表，要求包含一个向量类型的参数
 * @return Value 归一化后的单位向量
 */
inline Value normalize(const std::vector<Value>& args) {
    return args[0].normalize();
}

/**
 * @brief 计算矩阵的行列式
 * 
 * @param args 参数列表，要求包含一个矩阵类型的参数
 * @return Value 行列式结果
 */
inline Value det(const std::vector<Value>& args) {
    return args[0].determinant();
}
