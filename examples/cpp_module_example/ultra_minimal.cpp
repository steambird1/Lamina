/*
 * 极简测试模块 - ultra_minimal.cpp
 * 这是一个超级简化的模块实现，专门用于解决调用约定问题
 */

#include "lamina_api/lamina.hpp"
#include "lamina_api/value.hpp"

#include <vector>

Value pi = Value(3.1415);
Value greet(const std::vector<Value>& args) {
    /* 问好函数
     * greet(name: string)
     */
    if (!args.empty()) {
        return LAMINA_NULL;
    }
    std::cout << "Hello, " << args[0].to_string() << "" << std::endl;
    return LAMINA_NULL;
}

Value lamina_protect_function([[maybe_unused]] const std::vector<Value>& args) {
    /*
     * 测试被保护的函数
     */
    std::cout << "Hello World" << std::endl;
    return LAMINA_NULL;
}