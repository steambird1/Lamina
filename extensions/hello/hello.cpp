#include "../../interpreter/interpreter.hpp"

extern "C" __declspec(dllexport) void _entry(Interpreter& interpreter) {
    interpreter.set_global_variable("hello_from_dll", Value("Hello from dynamic library!"));
}