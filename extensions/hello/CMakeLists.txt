cmake_minimum_required(VERSION 3.16.3)
project(hello_plugin VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 lamina_core
find_path(LAMINA_INCLUDE_DIR 
    NAMES interpreter.hpp
    PATHS ../../interpreter
    REQUIRED
)

# 添加头文件目录
include_directories(${LAMINA_INCLUDE_DIR})

# 添加库搜索路径
link_directories(../../build)

# 创建插件动态库
add_library(hello SHARED hello.cpp)

# 链接到 lamina_core
target_link_libraries(hello PRIVATE lamina_core)

# 设置输出属性
set_target_properties(hello PROPERTIES 
    PREFIX ""           # 去掉 lib 前缀
    CXX_VISIBILITY_PRESET hidden
    POSITION_INDEPENDENT_CODE ON
)

# 平台特定设置
if(WIN32)
    set_target_properties(hello PROPERTIES SUFFIX ".dll")
elseif(APPLE)
    set_target_properties(hello PROPERTIES SUFFIX ".dylib")
else()
    set_target_properties(hello PROPERTIES SUFFIX ".so")
endif()

# 编译器警告
if(MSVC)
    target_compile_options(hello PRIVATE /W4)
else()
    target_compile_options(hello PRIVATE -Wall -Wextra)
endif()
