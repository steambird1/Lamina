# Lamina 模块开发指南 v2.0

## 概述
Lamina 支持通过动态库（DLL/SO）模块扩展功能。模块系统基于命名空间设计，提供类型安全的跨平台API。每个模块可以向解释器添加命名空间、函数和变量。

## 模块目录结构
```
extensions/
├── minimal/              # 最小示例模块
│   ├── minimal.cpp       # 模块源代码
│   └── CMakeLists.txt    # 构建配置
├── json_v2/              # JSON处理模块
│   ├── json_v2.cpp       # JSON功能实现
│   └── CMakeLists.txt    # 构建配置
└── standard/             # 标准库（内置到 lamina_core 中）
    ├── array.cpp         # 数组操作
    ├── math.cpp          # 数学函数
    ├── random.cpp        # 随机数生成
    ├── stdio.cpp         # 输入输出
    ├── times.cpp         # 时间处理
    └── sockets.cpp       # 网络功能
```

## 新模块API (v2.0)

### 核心特性
- **命名空间支持**：模块函数自动注册到指定命名空间
- **类型安全**：统一的 `LaminaValue` 类型系统
- **跨平台**：使用 `LAMINA_EXPORT` 和 `LAMINA_CALL` 宏确保兼容性
- **版本控制**：内置模块签名验证机制

### 1. 模块源代码模板
```cpp
#include "../../interpreter/module_api.hpp"
#include <cstdio>

// 模块函数实现 - 使用标准调用约定
LAMINA_EXPORT LaminaValue LAMINA_CALL my_function(const LaminaValue* args, int argc) {
    // 参数检查
    if (argc < 1) {
        return LAMINA_MAKE_STRING("Error: No arguments provided");
    }
    
    // 类型检查
    if (args[0].type != LAMINA_TYPE_STRING) {
        return LAMINA_MAKE_STRING("Error: Expected string argument");
    }
    
    // 处理逻辑
    static char result[256];
    snprintf(result, sizeof(result), "Processed: %s", args[0].data.string_val);
    
    return LAMINA_MAKE_STRING(result);
}

LAMINA_EXPORT LaminaValue LAMINA_CALL my_math_add(const LaminaValue* args, int argc) {
    if (argc < 2) {
        return LAMINA_MAKE_NUMBER(0.0);
    }
    
    double a = (args[0].type == LAMINA_TYPE_NUMBER) ? args[0].data.number_val : 0.0;
    double b = (args[1].type == LAMINA_TYPE_NUMBER) ? args[1].data.number_val : 0.0;
    
    return LAMINA_MAKE_NUMBER(a + b);
}

// 模块导出的函数列表
static LaminaFunctionEntry my_functions[] = {
    {"process", my_function, "Process a string value"},
    {"add", my_math_add, "Add two numbers"}
};

// 模块导出结构
static LaminaModuleExports my_exports = {
    {
        "mymodule",          // namespace_name
        "1.0",               // version
        "My custom module"   // description
    },
    my_functions,            // functions
    2,                       // function_count
    nullptr,                 // variables (暂未实现)
    0                        // variable_count
};

// 必需的导出函数
LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    return &my_exports;
}

LAMINA_EXPORT const char* LAMINA_CALL lamina_module_signature() {
    return LAMINA_MODULE_SIGNATURE;
}
```

### 2. CMakeLists.txt 模板
```cmake
cmake_minimum_required(VERSION 3.16)

# 创建模块库
add_library(my_module SHARED my_module.cpp)

# 链接 lamina_core
target_link_libraries(my_module PRIVATE lamina_core)

# 设置包含目录
target_include_directories(my_module PRIVATE 
    ${CMAKE_SOURCE_DIR}/interpreter
)

# 跨平台设置
set_target_properties(my_module PROPERTIES
    PREFIX ""
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 平台特定设置
if(WIN32)
    set_target_properties(my_module PROPERTIES SUFFIX ".dll")
else()
    set_target_properties(my_module PROPERTIES SUFFIX ".so")
endif()

# 调试输出
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(my_module PROPERTIES
        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    )
endif()
```

## 数据类型系统

### LaminaValue 类型
```cpp
typedef enum {
    LAMINA_TYPE_NULL,
    LAMINA_TYPE_BOOL,
    LAMINA_TYPE_NUMBER,
    LAMINA_TYPE_STRING
} LaminaValueType;

typedef struct {
    LaminaValueType type;
    union {
        bool bool_val;
        double number_val;
        const char* string_val;
    } data;
} LaminaValue;
```

### 便利宏
```cpp
// 创建值
LaminaValue null_val = LAMINA_MAKE_NULL();
LaminaValue bool_val = LAMINA_MAKE_BOOL(true);
LaminaValue num_val = LAMINA_MAKE_NUMBER(42.0);
LaminaValue str_val = LAMINA_MAKE_STRING("Hello");

// 类型检查
if (args[0].type == LAMINA_TYPE_STRING) {
    const char* str = args[0].data.string_val;
    // 处理字符串
}
```

## 构建与使用

### 构建模块
```bash
# 主项目构建（自动构建所有模块）
cmake -B build -DCMAKE_BUILD_TYPE=Debug .
cmake --build build --config Debug --parallel

# 模块输出位置：build/Debug/my_module.dll
```

### 在脚本中使用
```lamina
// 加载模块
include "my_module";

// 使用命名空间函数
var result = mymodule::process("Hello World");
print("结果:", result);

var sum = mymodule::add(10, 20);
print("求和:", sum);
```

### 命令行使用
```bash
# 确保模块DLL与Lamina.exe在同一目录
cd build/Debug
./Lamina.exe script.lm
```

## 高级特性

### 错误处理
```cpp
LAMINA_EXPORT LaminaValue LAMINA_CALL safe_divide(const LaminaValue* args, int argc) {
    if (argc < 2) {
        return LAMINA_MAKE_STRING("Error: Need two arguments");
    }
    
    if (args[0].type != LAMINA_TYPE_NUMBER || args[1].type != LAMINA_TYPE_NUMBER) {
        return LAMINA_MAKE_STRING("Error: Arguments must be numbers");
    }
    
    double a = args[0].data.number_val;
    double b = args[1].data.number_val;
    
    if (b == 0.0) {
        return LAMINA_MAKE_STRING("Error: Division by zero");
    }
    
    return LAMINA_MAKE_NUMBER(a / b);
}
```

### 内存管理
- 字符串返回值使用静态缓冲区或确保内存生命周期
- 避免返回局部变量的指针
- 使用 `snprintf` 等安全函数防止缓冲区溢出

## 最佳实践

1. **命名空间设计**：使用简洁且描述性的命名空间名称
2. **参数验证**：始终验证参数数量和类型
3. **错误处理**：返回有意义的错误消息
4. **文档注释**：在函数条目中提供清晰的描述
5. **版本管理**：使用语义化版本号
6. **跨平台**：使用提供的宏确保平台兼容性

## 故障排除

### 常见问题
- **模块无法加载**：检查DLL路径和依赖项
- **函数未找到**：确认命名空间和函数名正确
- **签名不匹配**：检查模块API版本兼容性
- **运行时错误**：验证参数类型和内存管理

### 调试技巧
```cpp
// 在模块函数中添加调试输出
#ifdef _DEBUG
    printf("Debug: Function called with %d args\n", argc);
#endif
```

## 示例模块
- **minimal**：`extensions/minimal/` - 最简单的功能演示
- **json_v2**：`extensions/json_v2/` - JSON处理功能
- 查看这些目录获取完整的实现示例
