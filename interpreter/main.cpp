#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "trackback.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <typeinfo>  // 用于typeid操作符

int main(int argc, char* argv[]) {
    if (argc < 2) {
        // 进入交互式终端（REPL）
        std::cout << "Lamina REPL. Press Ctrl+C or :exit to exit.\n";
        std::cout << "Type :help for help.\n";
        Interpreter interpreter;
        int lineno = 1;
        while (true) {
            try {
                // Use color-safe prompt
                if (Interpreter::supports_colors()) {
                    std::cout << "\033[1;32m>\033[0m "; // Green prompt with colors
                } else {
                    std::cout << "> "; // Plain prompt without colors
                }
                std::string line;
                if (!std::getline(std::cin, line)) break;
                
                if (line.empty()) { 
                    ++lineno; 
                    continue; 
                }
                
                // 处理特殊命令
                if (line == ":exit") {
                    break;
                }
                
                if (line == ":help") {
                    std::cout << "Lamina Interpreter Commands:\n";
                    std::cout << "  :exit - Exit interpreter\n";
                    std::cout << "  :help - Show this help message\n";
                    std::cout << "  :vars - Show all variables\n";
                    std::cout << "  :clear - Clear screen\n";
                    ++lineno;
                    continue;
                }
                
                if (line == ":vars") {
                    interpreter.printVariables();
                    ++lineno;
                    continue;
                }
                
                if (line == ":clear") {
                    #ifdef _WIN32
                    (void)system("cls");
                    #else
                    (void)system("clear");
                    #endif
                    ++lineno;
                    continue;
                }
                
                auto tokens = Lexer::tokenize(line);
                auto ast = Parser::parse(tokens);
                if (!ast) {
                    print_traceback("<stdin>", lineno);
                } else {
                    // Only support BlockStmt
                    auto* block = dynamic_cast<BlockStmt*>(ast.get());
                    if (block) {
                        try {
                            for (auto& stmt : block->statements) {
                                try {
                                    interpreter.execute(stmt);
                                } catch (const RuntimeError& re) {
                                    interpreter.print_stack_trace(re, true);
                                    break;
                                } catch (const std::exception& e) {
                                    Interpreter::print_error(e.what(), true);
                                    break;
                                }
                            }
                        } catch (const RuntimeError& re) {
                            interpreter.print_stack_trace(re, true);
                        } catch (...) {
                            Interpreter::print_error("Unknown error occurred", true);
                        }
                    }
                }
            } catch (...) {
                Interpreter::print_error("REPL environment caught exception, but recovered", true);
            }
            ++lineno;
        }
        return 0;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Unable to open file: " << argv[1] << std::endl;
        return 1;
    }
    std::cout << "Executing file: " << argv[1] << std::endl;
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
    
    // 只支持 BlockStmt
    auto* block = dynamic_cast<BlockStmt*>(ast.get());
    if (block) {
        // 每个语句独立执行并捕获异常，但保持解释器状态
        int currentLine = 0;
        for (auto& stmt : block->statements) {
            currentLine++;
            try {
                interpreter.execute(stmt);
            } catch (const RuntimeError& re) {
                // Use stack trace for runtime errors
                interpreter.print_stack_trace(re, true);
            } catch (const ReturnException&) {
                // Catch and show meaningful info (return outside function causes this exception)
                Interpreter::print_warning("Return statement used outside function (line " + std::to_string(currentLine) + ")", true);
            } catch (const BreakException&) {
                // Catch and show meaningful info
                Interpreter::print_warning("Break statement used outside loop (line " + std::to_string(currentLine) + ")", true);
            } catch (const ContinueException&) {
                // Catch and show meaningful info
                Interpreter::print_warning("Continue statement used outside loop (line " + std::to_string(currentLine) + ")", true);
            } catch (const std::exception& e) {
                // Other standard exceptions
                Interpreter::print_error(std::string(e.what()) + " (line " + std::to_string(currentLine) + ")", true);
                std::cerr << "Type: " << typeid(e).name() << std::endl;
            } catch (...) {
                // Unknown exceptions
                Interpreter::print_error("Unknown error (line " + std::to_string(currentLine) + ")", true);
            }
        }
        std::cout << "\nProgram execution completed." << std::endl;
    }
    return 0;
}
