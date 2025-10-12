#pragma once
#include "interpreter.hpp"
#include "help_text.hpp"
#include <string>

class Interpreter;

inline void print_help() {
    std::cout << HELP_TEXT << std::endl;
}

int exec_block(const BlockStmt* block);

void enable_ansi_escape();

int run_file(const std::string& path);

int argv_parser(int argc, const char* const argv[]);

int repl();
