cmake_minimum_required(VERSION 3.31)
project(Lamina)

set(CMAKE_CXX_STANDARD 20)

include_directories(interpreter)

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
        extensions/standard/math.cpp
        extensions/standard/stdio.cpp
        extensions/standard/random.cpp
        extensions/standard/random.hpp
        extensions/standard/times.cpp
        extensions/standard/times.hpp
)