#pragma once
#ifndef LAMINA_EXPORT
#ifdef __GNUC__
#define LAMINA_EXPORT __attribute__((visibility("default")))
#else
#define LAMINA_EXPORT
#endif
#endif

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

// Lamina 版本号
#include "version.hpp"
// Lamina 帮助文本
#include "help_text.hpp"

/*
    对LAMINA核心资源操作的头文件
 */

// 声明错误处理函数
LAMINA_EXPORT void error_and_exit(const std::string& msg);

template<class>
constexpr bool always_false = false;


#define LAMINA_BOOL(value) Value((bool) value)
#define LAMINA_INT(value) Value((int) value)
#define LAMINA_DOUBLE(value) Value((double) value)
#define LAMINA_STRING(value) Value((const char*) value)
#define LAMINA_BIGINT(value) Value((const ::BigInt&) value)
#define LAMINA_RATIONAL(value) Value((const ::Rational&) value)
#define LAMINA_IRRATIONAL(value) Value((const ::Irrational&) value)
#define LAMINA_COMPLEX(real, imag) Value(::lamina::Complex(real, imag))
#define LAMINA_ARR(value) Value(value)
#define LAMINA_MATRIX(value) Value(value)
#define LAMINA_NULL Value()

// #define LAMINA_FUNC_WIT_ANY_ARGS(func_name, func)                                                \
//     void func##_any_args_entry(Interpreter& interpreter);                                        \
//     namespace {                                                                                  \
//         struct func##_any_args_registrar {                                                       \
//             func##_any_args_registrar() {                                                        \
//                 Interpreter::register_entry(&func##_any_args_entry);                             \
//             }                                                                                    \
//         } func##_any_args_instance;                                                              \
//     }                                                                                            \
//     inline void func##_any_args_entry(Interpreter& interpreter) {                                \
//         interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value { \
//             return func(args);                                                                   \
//         };                                                                                       \
//     }
//
// #define LAMINA_FUNC(func_name, func, arg_count)                                                       \
//     LAMINA_EXPORT void func##_entry(Interpreter& interpreter);                                        \
//     namespace {                                                                                       \
//         struct func##_registrar {                                                                     \
//             func##_registrar() {                                                                      \
//                 Interpreter::register_entry(&func##_entry);                                           \
//             }                                                                                         \
//         } func##_instance;                                                                            \
//     }                                                                                                 \
//     inline void func##_entry(Interpreter& interpreter) {                                              \
//         interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value {      \
//             if (args.size() != arg_count) {                                                           \
//                 std::cerr << "Error: " << func_name << "() requires " << arg_count << " arguments\n"; \
//                 return Value();                                                                       \
//             }                                                                                         \
//             return func(args);                                                                        \
//         };                                                                                            \
//     }
//
// #define LAMINA_FUNC_MULTI_ARGS(func_name, func, arg_count)                                              \
//     void func##_entry(Interpreter& interpreter);                                                        \
//     namespace {                                                                                         \
//         struct func##_registrar {                                                                       \
//             func##_registrar() {                                                                        \
//                 Interpreter::register_entry(&func##_entry);                                             \
//             }                                                                                           \
//         } func##_instance;                                                                              \
//     }                                                                                                   \
//     inline void func##_entry(Interpreter& interpreter) {                                                \
//         interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value {        \
//             if (args.size() > arg_count) {                                                              \
//                 std::cerr << "Error: " << func_name << "() takes 0 to " << arg_count << " arguments\n"; \
//                 return Value();                                                                         \
//             }                                                                                           \
//             return func(args);                                                                          \
//         };                                                                                              \
//     }
//
//
//
// #define LAMINA_CALL_FUNC(interpreter, function, args)                  \
//     interpreter.call_function(function, args);
//
// #define LAMINA_GET_LOCALS(interpreter) \
//     interpreter.get_variable()
// #define LAMINA_GET_GLOBALS(interpreter) \
//     interpreter.get_globals()
//
// // subitems必须是 std::unordered_map<std::string, Value>类型
// #define LAMINA_MODULE(interpreter, name, id,  subitems) \
//     interpreter.entry_module(std::make_unique<LmModule>(name, id, subitems))
//
// #define LAMINA_ENTRY_FUNC(interpreter, name,function) \
//     interpreter.entry_function(name, function)

#define LAMINA_BUILTINS_FUNC(name, func)                                     \
    {name, Value(                                                            \
        std::make_shared<LmCppFunction>(func)                                \
    )}

#define LAMINA_BUILTINS_MODULE(name, version, subitems)                      \
    {name, Value(std::make_shared<LmModule>(name, version, subitems) )}       \


// L_ERR
class StdLibException : public std::exception {
public:
    std::string message;
    StdLibException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};