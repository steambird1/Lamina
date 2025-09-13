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
        auto num = rat.get_numerator();
        auto den = rat.get_denominator();

        if (num.is_perfect_square() && den.is_perfect_square()) {
            // Both are perfect squares
            return Value(::Rational(num.sqrt(), den.sqrt()));
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

/**
 * @brief 获取数组、矩阵或字符串的大小
 * 
 * @param args 参数列表，要求包含一个数组、矩阵或字符串类型的参数
 * @return Value 大小结果（整数）
 */
inline Value size(const std::vector<Value>& args) {
    if (args[0].is_array()) {
        const auto& arr = std::get<std::vector<Value>>(args[0].data);
        return Value(static_cast<int>(arr.size()));
    } else if (args[0].is_matrix()) {
        const auto& mat = std::get<std::vector<std::vector<Value>>>(args[0].data);
        return Value(static_cast<int>(mat.size()));
    } else if (args[0].is_string()) {
        const auto& str = std::get<std::string>(args[0].data);
        return Value(static_cast<int>(str.length()));
    }
    return Value(1);// Scalar values have size 1
}

/**
 * @brief 整数除法
 * 
 * @param args 参数列表，要求包含两个数值类型的参数
 * @return Value 整数除法结果（整数）
 */
inline Value idiv(const std::vector<Value>& args) {
    if (!args[0].is_numeric() || !args[1].is_numeric()) {
        error_and_exit("idiv() requires numeric arguments");
    }
    double divisor = args[1].as_number();
    if (divisor == 0.0) {
        error_and_exit("Integer division by zero");
    }
    double dividend = args[0].as_number();
    return Value(static_cast<int>(dividend / divisor));
}

/**
 * @brief 将数值转换为分数形式
 * 
 * @param args 参数列表，要求包含一个数值类型的参数
 * @return Value 分数形式的结果
 */
inline Value fraction(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        std::cerr << "Error: fraction() requires numeric argument" << std::endl;
        return Value();
    }

    // If already a rational, return as is
    if (args[0].is_rational()) {
        return args[0];
    }

    // Convert double to rational approximation
    double val = args[0].as_number();
    try {
        Rational rat = Rational::from_double(val);
        return Value(rat);
    } catch (const std::exception& e) {
        std::cerr << "Error: Cannot convert to fraction: " << e.what() << std::endl;
        return Value();
    }
}

/**
 * @brief 将数值转换为小数形式
 * 
 * @param args 参数列表，要求包含一个数值类型的参数
 * @return Value 小数形式的结果
 */
inline Value decimal(const std::vector<Value>& args) {
    if (!args[0].is_numeric()) {
        std::cerr << "Error: decimal() requires numeric argument" << std::endl;
        return Value();
    }

    // Convert to double representation
    double val = args[0].as_number();
    return Value(val);
}

/**
 * @brief 计算幂运算
 * 
 * @param args 参数列表，要求包含两个数值类型的参数（底数和指数）
 * @return Value 幂运算结果
 */
inline Value pow(const std::vector<Value>& args) {
    if (args.size() < 2) {
        std::cerr << "Error: pow() requires two arguments (base, exponent)" << std::endl;
        return Value();
    }
    
    if (!args[0].is_numeric() || !args[1].is_numeric()) {
        std::cerr << "Error: pow() requires numeric arguments" << std::endl;
        return Value();
    }
    
    // Handle BigInt base with integer exponent
    if (args[0].is_bigint() && (args[1].is_int() || args[1].is_bigint())) {
        const auto& base = std::get<::BigInt>(args[0].data);
        
        ::BigInt exponent;
        if (args[1].is_int()) {
            exponent = ::BigInt(std::get<int>(args[1].data));
        } else {
            exponent = std::get<::BigInt>(args[1].data);
        }
        
        try {
            return Value(base.power(exponent));
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return Value();
        }
    }
    
    // For non-BigInt cases or when precision loss is acceptable
    double base_val = args[0].as_number();
    double exp_val = args[1].as_number();
    
    // Warn if we're converting BigInt to double
    if (args[0].is_bigint() || args[1].is_bigint()) {
        Interpreter::print_warning("pow() with BigInt converted to floating point, precision may be lost");
    }
    
    return Value(std::pow(base_val, exp_val));
}

/**
 * @brief 计算两个数的最大公约数
 * 
 * @param args 参数列表，要求包含两个数值类型的参数
 * @return Value 最大公约数结果
 */
inline Value gcd(const std::vector<Value>& args) {
    if (args.size() < 2) {
        std::cerr << "Error: gcd() requires two arguments" << std::endl;
        return Value();
    }
    
    if (!args[0].is_numeric() || !args[1].is_numeric()) {
        std::cerr << "Error: gcd() requires numeric arguments" << std::endl;
        return Value();
    }
    
    // Handle BigInt case
    if (args[0].is_bigint() || args[1].is_bigint()) {
        ::BigInt a = args[0].is_bigint() ? 
            std::get<::BigInt>(args[0].data) : ::BigInt(std::get<int>(args[0].data));
        ::BigInt b = args[1].is_bigint() ? 
            std::get<::BigInt>(args[1].data) : ::BigInt(std::get<int>(args[1].data));
        
        return Value(::BigInt::gcd(a, b));
    }
    
    // Handle regular integer case
    if (args[0].is_int() && args[1].is_int()) {
        int a = std::get<int>(args[0].data);
        int b = std::get<int>(args[1].data);
        
        // Simple GCD algorithm for integers
        a = std::abs(a);
        b = std::abs(b);
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return Value(a);
    }
    
    // For floating point, warn about precision loss
    Interpreter::print_warning("gcd() with floating point numbers may have precision issues");
    long long a = static_cast<long long>(std::abs(args[0].as_number()));
    long long b = static_cast<long long>(std::abs(args[1].as_number()));
    
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return Value(static_cast<int>(a));
}

/**
 * @brief 计算两个数的最小公倍数
 * 
 * @param args 参数列表，要求包含两个数值类型的参数
 * @return Value 最小公倍数结果
 */
inline Value lcm(const std::vector<Value>& args) {
    if (args.size() < 2) {
        std::cerr << "Error: lcm() requires two arguments" << std::endl;
        return Value();
    }
    
    if (!args[0].is_numeric() || !args[1].is_numeric()) {
        std::cerr << "Error: lcm() requires numeric arguments" << std::endl;
        return Value();
    }
    
    // Handle BigInt case
    if (args[0].is_bigint() || args[1].is_bigint()) {
        ::BigInt a = args[0].is_bigint() ? 
            std::get<::BigInt>(args[0].data) : ::BigInt(std::get<int>(args[0].data));
        ::BigInt b = args[1].is_bigint() ? 
            std::get<::BigInt>(args[1].data) : ::BigInt(std::get<int>(args[1].data));
        
        return Value(::BigInt::lcm(a, b));
    }
    
    // Handle regular integer case
    if (args[0].is_int() && args[1].is_int()) {
        int a = std::abs(std::get<int>(args[0].data));
        int b = std::abs(std::get<int>(args[1].data));
        
        if (a == 0 || b == 0) {
            return Value(0);
        }
        
        // LCM = |a * b| / GCD(a, b)
        int gcd_val = a;
        int temp_b = b;
        while (temp_b != 0) {
            int temp = temp_b;
            temp_b = gcd_val % temp_b;
            gcd_val = temp;
        }
        
        return Value((a / gcd_val) * b);
    }
    
    // For floating point, warn about precision loss
    Interpreter::print_warning("lcm() with floating point numbers may have precision issues");
    long long a = static_cast<long long>(std::abs(args[0].as_number()));
    long long b = static_cast<long long>(std::abs(args[1].as_number()));
    
    if (a == 0 || b == 0) {
        return Value(0);
    }
    
    long long gcd_val = a;
    long long temp_b = b;
    while (temp_b != 0) {
        long long temp = temp_b;
        temp_b = gcd_val % temp_b;
        gcd_val = temp;
    }
    
    return Value(static_cast<int>((a / gcd_val) * b));
}

/** namespace latex {
    ImprovedLaTeXCalculator* improved_la_te_x_calculator = new ImprovedLaTeXCalculator();

    Value Initialize(const std::vector<Value>& args) {
        improved_la_te_x_calculator;
        return Value(nullptr);
    }

    Value set_var(const std::vector<Value>& args) {
        if (!args[0].is_string()) {
            L_ERR("Args 0 must be string");
        }
        if (!args[1].is_numeric()) {
            L_ERR("Args 1 must be number");
        }
        std::string var_name = args[0].to_string();
        double value = args[1].as_number();
        improved_la_te_x_calculator->setVariable(var_name, value);
        return Value(nullptr);
    }

    Value calculate(const std::vector<Value>& args) {
        if (!args[0].is_string()) {
            L_ERR("Args 0 must be string");
        }
        std::string expression = args[0].to_string();
        auto [success, result] = improved_la_te_x_calculator->evaluate(expression);
        if (!success) {
            throw RuntimeError("LaTeX evaluation failed: " + result);
        }
        try {
            size_t pos;
            double num = std::stod(result, &pos);
            if (pos == result.size() && num == std::floor(num)) {
                return Value(static_cast<int>(num));
            }
            return Value(num);
        } catch (const std::exception& e) {
            throw RuntimeError("Result conversion failed: " + result);
        }
    }
    LAMINA_FUNC("init", Initialize, 0);
    LAMINA_FUNC("set_var", set_var, 2);
    LAMINA_FUNC("calculate", calculate, 1);

}// namespace latex**/

namespace lamina {
    LAMINA_FUNC("sqrt", sqrt, 1);
    LAMINA_FUNC("pi", pi, 0);
    LAMINA_FUNC("e", e, 0);
    LAMINA_FUNC("abs", abs, 1);
    LAMINA_FUNC("sin", sin, 1);
    LAMINA_FUNC("cos", cos, 1);
    LAMINA_FUNC("tan", tan, 1);
    LAMINA_FUNC("log", log, 1);
    LAMINA_FUNC("round", round, 1);
    LAMINA_FUNC("floor", floor, 1);
    LAMINA_FUNC("ceil", ceil, 1);
    LAMINA_FUNC("dot", dot, 2);
    LAMINA_FUNC("cross", cross, 2);
    LAMINA_FUNC("norm", norm, 1);
    LAMINA_FUNC("normalize", normalize, 1);
    LAMINA_FUNC("det", det, 1);
    LAMINA_FUNC("size", size, 1);
    LAMINA_FUNC("idiv", idiv, 2);
    LAMINA_FUNC("fraction", fraction, 1);
    LAMINA_FUNC("decimal", decimal, 1);
    LAMINA_FUNC("pow", pow, 2);
    LAMINA_FUNC("gcd", gcd, 2);
    LAMINA_FUNC("lcm", lcm, 2);
}// namespace lamina
