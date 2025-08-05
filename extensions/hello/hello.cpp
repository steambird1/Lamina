#include "../../interpreter/interpreter.hpp"

// 跨平台的导出宏
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

extern "C" EXPORT void _entry(Interpreter& interpreter) {
    interpreter.set_global_variable("hello_from_dll", Value("Hello from dynamic library!"));
}