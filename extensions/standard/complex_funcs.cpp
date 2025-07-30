#include "lamina.hpp"
#include "../extensions/standard/complex.hpp"

// 创建复数的内置函数
Value builtin_complex(const std::vector<Value>& args) {
    if (args.size() != 2) {
        error_and_exit("complex() requires 2 arguments (real, imag)");
        return Value();
    }

    if (!args[0].is_numeric() || !args[1].is_numeric()) {
        error_and_exit("complex() arguments must be numeric");
        return Value();
    }

    double real = args[0].as_number();
    double imag = args[1].as_number();

    return LAMINA_COMPLEX(real, imag);
}

// 计算复数共轭的内置函数
Value builtin_conjugate(const std::vector<Value>& args) {
    if (args.size() != 1) {
        error_and_exit("conjugate() requires 1 argument");
        return Value();
    }

    if (!args[0].is_complex()) {
        error_and_exit("conjugate() argument must be a complex number");
        return Value();
    }

    ::lamina::Complex c = std::get<::lamina::Complex>(args[0].data);
    return LAMINA_COMPLEX(c.conjugate().real, c.conjugate().imag);
}

// 计算复数模长的内置函数
Value builtin_magnitude(const std::vector<Value>& args) {
    if (args.size() != 1) {
        error_and_exit("magnitude() requires 1 argument");
        return Value();
    }

    if (!args[0].is_complex()) {
        error_and_exit("magnitude() argument must be a complex number");
        return Value();
    }

    ::lamina::Complex c = std::get<::lamina::Complex>(args[0].data);
    return LAMINA_DOUBLE(c.magnitude());
}

// 计算复数相位角的内置函数
Value builtin_phase(const std::vector<Value>& args) {
    if (args.size() != 1) {
        error_and_exit("phase() requires 1 argument");
        return Value();
    }

    if (!args[0].is_complex()) {
        error_and_exit("phase() argument must be a complex number");
        return Value();
    }

    ::lamina::Complex c = std::get<::lamina::Complex>(args[0].data);
    return LAMINA_DOUBLE(c.phase());
}

// 注册复数相关的内置函数
LAMINA_FUNC("complex", builtin_complex, 2);
LAMINA_FUNC("conjugate", builtin_conjugate, 1);
LAMINA_FUNC("magnitude", builtin_magnitude, 1);
LAMINA_FUNC("phase", builtin_phase, 1);
