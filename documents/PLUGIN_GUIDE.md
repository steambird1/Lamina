# Lamina 插件开发指南

## 概述
Lamina 支持通过动态库（DLL/SO）扩展功能。插件可以向解释器添加新的变量、函数和功能。

## 插件目录结构
```
extensions/
├── hello/                # 示例插件
│   ├── hello.cpp         # 插件源代码
│   └── CMakeLists.txt    # 构建配置
└── standard/             # 标准库（内置到 lamina_core 中）
    ├── array.cpp
    ├── math.cpp
    ├── random.cpp
    ├── stdio.cpp
    ├── times.cpp
    └── sockets.cpp
```

## 创建新插件

### 1. 插件源代码模板
```cpp
#include "../../interpreter/interpreter.hpp"

// 导出入口函数（Windows 需要 __declspec(dllexport)）
extern "C" __declspec(dllexport) void _entry(Interpreter& interpreter) {
    // 添加全局变量
    interpreter.set_global_variable("my_var", Value("Hello from plugin!"));
    
    // 添加内置函数
    interpreter.add_builtin_function("my_function", [](const std::vector<Value>& args) -> Value {
        // 函数实现
        return Value("Function result");
    });
}
```

### 2. CMakeLists.txt 模板
```cmake
cmake_minimum_required(VERSION 3.16)
project(my_plugin)

set(CMAKE_CXX_STANDARD 17)

# 查找 lamina_core
find_library(LAMINA_CORE 
    NAMES lamina_core
    PATHS ${CMAKE_SOURCE_DIR}/../../build/Debug 
          ${CMAKE_SOURCE_DIR}/../../build/Release
          ${CMAKE_SOURCE_DIR}/../../build
)

# 创建插件
add_library(my_plugin SHARED my_plugin.cpp)

# 链接到 lamina_core
target_link_libraries(my_plugin PRIVATE ${LAMINA_CORE})

# 设置包含目录
target_include_directories(my_plugin PRIVATE 
    ${CMAKE_SOURCE_DIR}/../../interpreter
)

# 设置输出属性
set_target_properties(my_plugin PROPERTIES
    PREFIX ""  # 去掉 lib 前缀
    SUFFIX ".dll"  # Windows 使用 .dll，Linux 使用 .so
)
```

## 构建插件

### 方法一：单独构建
```bash
cd extensions/my_plugin
cmake -B build -S .
cmake --build build
```

### 方法二：主项目构建时自动构建
在主 CMakeLists.txt 中已经配置了自动扫描 `extensions/` 目录并构建所有插件。

## 使用插件

### 在 Lamina 脚本中加载插件
```lamina
// 加载插件
include "my_plugin.dll";

// 使用插件提供的变量和函数
print(my_var);
print(my_function());
```

### 命令行使用
```bash
cd build/Debug  # 或 build/Release
./Lamina.exe script.lm
```

## 注意事项

1. **符号导出**：Windows 下必须使用 `__declspec(dllexport)` 导出 `_entry` 函数
2. **入口函数**：插件必须提供 `extern "C" void _entry(Interpreter& interpreter)` 函数
3. **依赖管理**：插件需要链接到 `lamina_core.dll`
4. **路径问题**：确保插件 DLL 与 Lamina.exe 在同一目录，或在系统 PATH 中
5. **跨平台**：使用条件编译支持不同平台的符号导出

## 示例插件
查看 `extensions/hello/` 目录获取完整的示例插件实现。
