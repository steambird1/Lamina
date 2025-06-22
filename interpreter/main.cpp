#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "trackback.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        // 进入交互式终端（REPL）
        std::cout << "Lamina Terminal. Press Ctrl+C to exit.\n";
        Interpreter interpreter;
        int lineno = 1;
        while (true) {
            std::cout << "> ";
            std::string line;
            if (!std::getline(std::cin, line)) break;
            if (line.empty()) { ++lineno; continue; }
            auto tokens = Lexer::tokenize(line);
            auto ast = Parser::parse(tokens);
            if (!ast) {
                print_traceback("<stdin>", lineno);
            } else {
                interpreter.execute(ast);
            }
            ++lineno;
        }
        return 0;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "无法打开文件: " << argv[1] << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    auto tokens = Lexer::tokenize(source);
    auto ast = Parser::parse(tokens);
    Interpreter interpreter;
    if (!ast) {
        print_traceback(argv[1], 1);
        return 2;
    }
    interpreter.execute(ast);
    return 0;
}
