# Minimal 测试模块
cmake_minimum_required(VERSION 3.10)

# 设置模块名称
set(MODULE_NAME minimal)

# 创建共享库
add_library(${MODULE_NAME} SHARED
    minimal.cpp
)

# 设置输出目录
set_target_properties(${MODULE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
)

# 跨平台编译设置
if(WIN32)
    # Windows 特定设置
    target_compile_definitions(${MODULE_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
    # 确保使用正确的调用约定
    target_compile_options(${MODULE_NAME} PRIVATE
        /std:c++17
    )
elseif(UNIX AND NOT APPLE)
    # Linux 特定设置
    target_compile_options(${MODULE_NAME} PRIVATE
        -fPIC
        -std=c++17
        -fvisibility=hidden
    )
    # 确保符号可见性
    target_compile_definitions(${MODULE_NAME} PRIVATE
        -DLAMINA_EXPORT=__attribute__\(\(visibility\(\"default\"\)\)\)
    )
elseif(APPLE)
    # macOS 特定设置
    target_compile_options(${MODULE_NAME} PRIVATE
        -fPIC
        -std=c++17
        -fvisibility=hidden
    )
    target_compile_definitions(${MODULE_NAME} PRIVATE
        -DLAMINA_EXPORT=__attribute__\(\(visibility\(\"default\"\)\)\)
    )
endif()

# 包含解释器头文件
target_include_directories(${MODULE_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/interpreter
)
