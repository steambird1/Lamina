cmake_minimum_required(VERSION 3.16.3)
project(Lamina)

set(CMAKE_CXX_STANDARD 20)
set(LIBUV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external/libuv)

include_directories(${LIBUV_ROOT}/include)
link_directories(${LIBUV_ROOT}/lib)

include_directories(interpreter)
add_library(lamina_core SHARED
        interpreter/ast.hpp
        interpreter/bigint.hpp
        interpreter/interpreter.cpp
        interpreter/interpreter.hpp
        interpreter/irrational.hpp
        interpreter/lexer.cpp
        interpreter/lexer.hpp
        interpreter/main.cpp
        interpreter/parser.cpp
        interpreter/parser.hpp
        interpreter/rational.hpp
        interpreter/trackback.hpp
        interpreter/value.hpp
        interpreter/lamina.hpp
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

if(MSVC)
    # MSVC specific options
    target_compile_options(lamina_core PRIVATE /W4)
else()
    # GCC/Clang options
    target_compile_options(lamina_core PRIVATE -fvisibility=hidden)
endif()

add_executable(Lamina
        interpreter/ast.hpp
        interpreter/bigint.hpp
        interpreter/interpreter.cpp
        interpreter/interpreter.hpp
        interpreter/irrational.hpp
        interpreter/lexer.cpp
        interpreter/lexer.hpp
        interpreter/main.cpp
        interpreter/parser.cpp
        interpreter/parser.hpp
        interpreter/rational.hpp
        interpreter/trackback.hpp
        interpreter/value.hpp
        interpreter/lamina.hpp
        interpreter/module.cpp
        interpreter/module.hpp
        extensions/standard/math.cpp
        extensions/standard/array.cpp
        extensions/standard/stdio.cpp
        extensions/standard/random.cpp
        extensions/standard/random.hpp
        extensions/standard/times.cpp
        extensions/standard/times.hpp
        extensions/standard/sockets.hpp
        extensions/standard/sockets.cpp
)

target_include_directories(lamina_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/interpreter
)

if(UNIX)
    target_link_libraries(Lamina PRIVATE
        lamina_core
        dl
    )
else()
    target_link_libraries(Lamina PRIVATE
        lamina_core
    )
endif()

if(UNIX AND NOT APPLE)
    message(STATUS "Building on Linux system.")

    file(GLOB_RECURSE EXTENSION_SOURCE_FILES
            "extensions/*.cpp"
            "extensions/*/*.cpp"
            "extensions/*/*/*.cpp"
    )

    foreach(SOURCE_FILE ${EXTENSION_SOURCE_FILES})
        if(NOT SOURCE_FILE MATCHES "extensions/standard")
            get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME_WE)
            add_library(${SOURCE_NAME} SHARED ${SOURCE_FILE})
            target_link_libraries(${SOURCE_NAME} lamina_core)
            set_target_properties(${SOURCE_NAME} PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/extensions
                    POSITION_INDEPENDENT_CODE ON
            )
            message(STATUS "Building extension: ${SOURCE_NAME} with core library")
        endif()
    endforeach()
endif()
