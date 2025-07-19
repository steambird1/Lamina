#pragma once

#include <string>
#include "interpreter.hpp"
#include "value.hpp"

/*
    对LAMINA核心资源操作的头文件
 */

#define LAMINA_BOOL(value) Value((bool) value)
#define LAMINA_INT(value) Value((int) value)
#define LAMINA_DOUBLE(value) Value((double) value)
#define LAMINA_STRING(value) Value((const char*) value)
#define LAMINA_BIGINT(value) Value((const ::BigInt&)value)
#define LAMINA_RATIONAL(value) Value((const ::Rational&)value)
#define LAMINA_IRRATIONAL(value) Value((const ::Irrational&)value)
#define LAMINA_ARR(value) Value(value)
#define LAMINA_MATRIX(value) Value(value)

#define LAMINA_FUNC_WIT_ANY_ARGS(func_name, func) \
void func##_any_args_entry(Interpreter& interpreter); \
namespace { \
struct func##_any_args_registrar { \
    func##_any_args_registrar() { \
        Interpreter::register_entry(&func##_any_args_entry); \
    } \
} func##_any_args_instance; \
} \
void func##_any_args_entry(Interpreter& interpreter) { \
    interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value { \
        return func(args); \
    }; \
}

#define LAMINA_FUNC(func_name, func, arg_count) \
void func##_entry(Interpreter& interpreter); \
namespace { \
struct func##_registrar { \
    func##_registrar() { \
        Interpreter::register_entry(&func##_entry); \
    } \
} func##_instance; \
} \
void func##_entry(Interpreter& interpreter) { \
    interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value { \
        if (args.size() != arg_count) { \
            std::cerr << "Error: " << func_name << "() requires " << arg_count <<" arguments\n"; \
            return Value(); \
        } \
        return func(args); \
    }; \
}

#define LAMINA_FUNC_MULTI_ARGS(func_name, func, arg_count) \
void func##_entry(Interpreter& interpreter); \
namespace { \
struct func##_registrar { \
    func##_registrar() { \
        Interpreter::register_entry(&func##_entry); \
    } \
} func##_instance; \
} \
void func##_entry(Interpreter& interpreter) { \
    interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value { \
        if (args.size() > arg_count) { \
            std::cerr << "Error: " << func_name << "() takes 0 to " << arg_count << " arguments\n"; \
            return Value(); \
        } \
        return func(args); \
    }; \
}


#define var(name, value) \
    do { \
        auto _v = (value); \
        Value name##_value; \
        if constexpr (std::is_integral_v<decltype(_v)>) { \
            if constexpr (std::is_same_v<decltype(_v), int>) { \
                name##_value = Value(static_cast<int>(_v)); \
            } else { \
                name##_value = Value(::BigInt(_v)); \
            } \
        } else if constexpr (std::is_floating_point_v<decltype(_v)>) { \
            name##_value = Value(static_cast<double>(_v)); \
        } else if constexpr (std::is_convertible_v<decltype(_v), std::string>) { \
            name##_value = Value(static_cast<std::string>(_v)); \
        } \
        interpreter.set_variable(#name, name##_value); \
    } while(0)



