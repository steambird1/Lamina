/*
 * 极简测试模块 - ultra_minimal.cpp
 * 这是一个超级简化的模块实现，专门用于解决调用约定问题
 */

#include "../../interpreter/module_api.hpp"
#include <cstdio>
#include <cstring>

// 使用LAMINA_CALL宏确保正确的调用约定
LAMINA_EXPORT LaminaValue LAMINA_CALL ultra_test_func(const LaminaValue* args, int argc) {
    LaminaValue result = LAMINA_MAKE_INT(0);// 零初始化
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = 42;

    printf("ultra_test_func called successfully!\n");
    return result;
}

// 模块导出的函数列表 - 只保留一个最简单的函数
static LaminaFunctionEntry ultra_functions[] = {
        {"test", ultra_test_func, "Ultra minimal test function"}};

// 模块导出结构
static LaminaModuleExports ultra_exports = {
        {
                "ultra_minimal",           // namespace_name - 修改为minimal以匹配DLL文件名
                "1.0",                     // version
                "Ultra minimal test module"// description
        },
        ultra_functions,// functions
        1,              // function_count
        nullptr,        // variables
        0               // variable_count
};

// 模块初始化函数 - 使用LAMINA_CALL宏
LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    printf("lamina_module_init called!\n");
    return &ultra_exports;
}

// 模块签名函数 - 使用LAMINA_CALL宏
LAMINA_EXPORT const char* LAMINA_CALL lamina_module_signature() {
    return LAMINA_MODULE_SIGNATURE;
}
