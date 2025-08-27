#pragma once
#include "interpreter.hpp"
#include <string>

inline void print_help() {
    std::cout << "          -- [ Lamina Help ] --             "  << std::endl;
    std::cout << "lamina <path>      ||  run the file at path" << std::endl;
    std::cout << "lamina run <path>  || run the file at path" << std::endl;
    std::cout << "lamina version     ||  show the version of lamina" << std::endl;
    std::cout << "lamina repl        ||  start repl" << std::endl;
    std::cout << "lamina help        ||  show help" << std::endl;
}

int exec_block(const BlockStmt* block);

void enable_ansi_escape();

int run_file(const std::string& path);

int argv_parser(int argc, const char* const argv[]);

int repl();
