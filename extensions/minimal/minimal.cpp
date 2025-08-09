/*
 * 最小测试模块 - 跨平台版本
 */

#include "../../interpreter/module_api.hpp"
#include <cstdio>

// 使用新的跨平台API - 标准C调用约定
LAMINA_EXPORT LaminaValue LAMINA_CALL minimal_test_func(const LaminaValue* args, int argc) {
    return LAMINA_MAKE_NULL();
}

// 带参数测试的函数
LAMINA_EXPORT LaminaValue LAMINA_CALL minimal_hello(const LaminaValue* args, int argc) {
    // 使用静态buffer保证内存生命周期
    static char result[256];
    
    if (argc > 0 && args[0].type == LAMINA_TYPE_STRING) {
        // 构造动态的返回消息
        snprintf(result, sizeof(result), "Hello from %s!", args[0].data.string_val);
        return LAMINA_MAKE_STRING(result);
    }
    
    snprintf(result, sizeof(result), "Hello from minimal!");
    return LAMINA_MAKE_STRING(result);
}

// 模块导出的函数列表
static LaminaFunctionEntry minimal_functions[] = {
    {"test", minimal_test_func, "Minimal test function"},
    {"hello", minimal_hello, "Hello function with parameter support"}
};

// 模块导出结构
static LaminaModuleExports minimal_exports = {
    {
        "minimal",        // namespace_name
        "1.0",           // version
        "Minimal test module" // description
    },
    minimal_functions,   // functions
    2,                   // function_count (updated)
    nullptr,             // variables
    0                    // variable_count
};

// 模块初始化函数 - 使用新的跨平台导出宏
LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    return &minimal_exports;
}

LAMINA_EXPORT const char* LAMINA_CALL lamina_module_signature() {
    return LAMINA_MODULE_SIGNATURE;
}
