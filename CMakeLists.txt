cmake_minimum_required(VERSION 3.16.3)
project(Lamina VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# 设置 libuv 相关变量
set(LIBUV_LIBRARY uv_a)
set(LIBUV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external/libuv)

# 检查 libuv 子模块是否存在
if(NOT EXISTS ${LIBUV_ROOT}/CMakeLists.txt)
    message(FATAL_ERROR "LibUV submodule not found. Please run 'git submodule update --init --recursive'")
endif()

# 添加 libuv 子目录
add_subdirectory(${LIBUV_ROOT})
# 只针对 libuv 启用 PIC
set_target_properties(uv uv_a PROPERTIES POSITION_INDEPENDENT_CODE ON)

# 设置 libuv 库和包含目录
set(LIBUV_LIBRARIES uv_a)
set(LIBUV_INCLUDE_DIRS ${LIBUV_ROOT}/include)
include_directories(${LIBUV_INCLUDE_DIRS})
include_directories(interpreter)
# 创建 lamina_core 共享库
add_library(lamina_core SHARED
        interpreter/ast.hpp
        interpreter/bigint.hpp
        interpreter/interpreter.cpp
        interpreter/interpreter.hpp
        interpreter/irrational.hpp
        interpreter/lexer.cpp
        interpreter/lexer.hpp
        interpreter/parser.cpp
        interpreter/parser.hpp
        interpreter/rational.hpp
        interpreter/trackback.hpp
        interpreter/value.hpp
        interpreter/lamina.hpp
        interpreter/module.cpp
        interpreter/module.hpp
        extensions/standard/math.cpp
        extensions/standard/stdio.cpp
        extensions/standard/random.cpp
        extensions/standard/random.hpp
        extensions/standard/times.cpp
        extensions/standard/times.hpp
        extensions/standard/array.cpp
        extensions/standard/sockets.hpp
        extensions/standard/sockets.cpp
)

# 设置 lamina_core 属性
set_target_properties(lamina_core PROPERTIES 
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# 定义导出宏
target_compile_definitions(lamina_core PRIVATE LAMINA_CORE_EXPORTS)

# 链接 libuv
target_link_libraries(lamina_core PRIVATE ${LIBUV_LIBRARY} uv)

# 设置编译器特定选项
if(MSVC)
    target_compile_options(lamina_core PRIVATE /W4)
else()
    target_compile_options(lamina_core PRIVATE 
        -Wall -Wextra -Wpedantic
        -fvisibility=hidden
    )
endif()

# 设置 lamina_core 的公共头文件目录
target_include_directories(lamina_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter
)

# 创建主可执行文件
add_executable(Lamina interpreter/main.cpp)

# 链接到 lamina_core
target_link_libraries(Lamina PRIVATE lamina_core)

# 平台特定的链接库
if(UNIX)
    target_link_libraries(Lamina PRIVATE dl)
endif()

# 自动复制依赖的动态库
add_custom_command(TARGET Lamina POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:uv>
        $<TARGET_FILE_DIR:Lamina>
    COMMENT "Copying libuv.dll to output directory"
)

# Windows 下复制 lamina_core.dll
if(WIN32)
    add_custom_command(TARGET Lamina POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:lamina_core>
            $<TARGET_FILE_DIR:Lamina>
        COMMENT "Copying lamina_core.dll to output directory"
    )
endif()

# 扩展/插件系统（仅在 Linux 下自动构建）
if(UNIX AND NOT APPLE)
    message(STATUS "Building on Linux system - auto-building extensions")

    file(GLOB_RECURSE EXTENSION_SOURCE_FILES
            "extensions/*.cpp"
            "extensions/*/*.cpp"
            "extensions/*/*/*.cpp"
    )

    foreach(SOURCE_FILE ${EXTENSION_SOURCE_FILES})
        # 排除标准库扩展
        if(NOT SOURCE_FILE MATCHES "extensions/standard")
            get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME_WE)
            add_library(${SOURCE_NAME} SHARED ${SOURCE_FILE})
            
            # 链接到 lamina_core 和 libuv
            target_link_libraries(${SOURCE_NAME} PRIVATE lamina_core ${LIBUV_LIBRARY})
            
            set_target_properties(${SOURCE_NAME} PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/extensions
                POSITION_INDEPENDENT_CODE ON
                PREFIX ""  # 去掉 lib 前缀
            )
            
            message(STATUS "Building extension: ${SOURCE_NAME}")
        endif()
    endforeach()
endif()

# 安装规则
install(TARGETS Lamina lamina_core
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件（供插件开发使用）
install(DIRECTORY interpreter/
    DESTINATION include/lamina
    FILES_MATCHING PATTERN "*.hpp"
)
