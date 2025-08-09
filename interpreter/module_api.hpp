/*
 * Lamina 模块API - 安全的跨平台动态库接口
 * 基于纯C接口，避免复杂C++对象跨DLL边界
 */

#ifndef LAMINA_MODULE_API_HPP
#define LAMINA_MODULE_API_HPP

// 跨平台调用约定定义
#ifdef _WIN32
    #ifdef __cplusplus
        #define LAMINA_CALL __cdecl
        #define LAMINA_EXPORT extern "C" __declspec(dllexport)
    #else
        #define LAMINA_CALL __cdecl
        #define LAMINA_EXPORT __declspec(dllexport)
    #endif
#else
    #ifdef __cplusplus
        #define LAMINA_CALL
        #define LAMINA_EXPORT extern "C" __attribute__((visibility("default")))
    #else
        #define LAMINA_CALL
        #define LAMINA_EXPORT __attribute__((visibility("default")))
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 模块信息结构 - 纯C结构
typedef struct {
    const char* namespace_name;  // 域名，如 "json", "math", "http"
    const char* version;         // 版本号
    const char* description;     // 描述
} LaminaModuleInfo;

// 简化的值类型枚举
typedef enum {
    LAMINA_TYPE_NULL = 0,
    LAMINA_TYPE_BOOL,
    LAMINA_TYPE_INT,
    LAMINA_TYPE_DOUBLE,
    LAMINA_TYPE_STRING
} LaminaValueType;

// 简化的值结构 - 纯C结构
typedef struct {
    LaminaValueType type;
    union {
        int bool_val;
        int int_val;
        double double_val;
        char* string_val;  // 必须由调用者管理内存
    } data;
} LaminaValue;

// 函数调用的回调类型 - 使用标准C调用约定确保跨平台兼容性
typedef LaminaValue (LAMINA_CALL *LaminaFunction)(const LaminaValue* args, int arg_count);

// 函数注册结构
typedef struct {
    const char* name;        // 函数名
    LaminaFunction func;     // 函数指针
    const char* description; // 函数描述
} LaminaFunctionEntry;

// 变量注册结构  
typedef struct {
    const char* name;        // 变量名
    LaminaValue value;       // 变量值
    const char* description; // 变量描述
} LaminaVariableEntry;

// 模块入口函数类型
typedef struct {
    LaminaModuleInfo info;
    const LaminaFunctionEntry* functions;
    int function_count;
    const LaminaVariableEntry* variables;
    int variable_count;
} LaminaModuleExports;

// 模块入口函数 - 每个模块必须实现 (注意返回值不是指针)
typedef LaminaModuleExports* (LAMINA_CALL *LaminaModuleInit)(void);

// 模块签名验证
#define LAMINA_MODULE_SIGNATURE "LAMINA_MODULE_V2"

// 辅助宏
#define LAMINA_MAKE_NULL() {LAMINA_TYPE_NULL, {0}}
#define LAMINA_MAKE_BOOL(b) {LAMINA_TYPE_BOOL, {.bool_val = (b) ? 1 : 0}}
#define LAMINA_MAKE_INT(i) {LAMINA_TYPE_INT, {.int_val = (i)}}
#define LAMINA_MAKE_DOUBLE(d) {LAMINA_TYPE_DOUBLE, {.double_val = (d)}}
#define LAMINA_MAKE_STRING(s) {LAMINA_TYPE_STRING, {.string_val = (char*)(s)}}

#ifdef __cplusplus
}
#endif

#endif // LAMINA_MODULE_API_HPP
