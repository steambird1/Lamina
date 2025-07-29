#pragma once
#ifndef LAMINA_EXPORT
#ifdef __GNUC__
#define LAMINA_EXPORT __attribute__((visibility("default")))
#else
#define LAMINA_EXPORT
#endif
#endif

#include <string>
#include "interpreter.hpp"
#include "value.hpp"

/*
    对LAMINA核心资源操作的头文件
 */

template<class> constexpr bool always_false = false;


#define LAMINA_BOOL(value) Value((bool) value)
#define LAMINA_INT(value) Value((int) value)
#define LAMINA_DOUBLE(value) Value((double) value)
#define LAMINA_STRING(value) Value((const char*) value)
#define LAMINA_BIGINT(value) Value((const ::BigInt&)value)
#define LAMINA_RATIONAL(value) Value((const ::Rational&)value)
#define LAMINA_IRRATIONAL(value) Value((const ::Irrational&)value)
#define LAMINA_ARR(value) Value(value)
#define LAMINA_MATRIX(value) Value(value)
#define LAMINA_NULL Value()

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
void func##_entry(Interpreter& interpreter) LAMINA_EXPORT; \
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

#define LAMINA_GET_VAR(interpreter, var) \
   interpreter.get_variable(#var)

#define L_ERR(msg)\
    error_and_exit(msg); \

#define LAMINA_GLOBAL_VAR(name, value) \
void global_var_##name##_entry(Interpreter& interpreter) { \
    interpreter.set_global_variable(#name, Value(value)); \
} \
namespace { \
struct global_var_##name##_registrar { \
    global_var_##name##_registrar() { \
        Interpreter::register_entry(&global_var_##name##_entry); \
    } \
} global_var_##name##_instance; \
}
