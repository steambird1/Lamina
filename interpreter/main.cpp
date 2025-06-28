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
        std::cout << "Lamina Terminal v1.0. Press Ctrl+C to exit.\n";
        std::cout << "输入 :help 查看帮助信息，输入 :quit 或 :exit 退出\n";
        Interpreter interpreter;
        int lineno = 1;
        while (true) {
            try {
                std::cout << "\033[1;32m>\033[0m "; // 绿色提示符
                std::string line;
                if (!std::getline(std::cin, line)) break;
                
                if (line.empty()) { 
                    ++lineno; 
                    continue; 
                }
                
                // 处理特殊命令
                if (line == ":quit" || line == ":exit") {
                    std::cout << "再见！\n";
                    break;
                }
                
                if (line == ":help") {
                    std::cout << "Lamina 解释器命令：\n";
                    std::cout << "  :quit 或 :exit - 退出解释器\n";
                    std::cout << "  :help - 显示此帮助信息\n";
                    std::cout << "  :vars - 显示所有变量\n";
                    std::cout << "  :clear - 清除屏幕\n";
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
                    system("cls");
                    #else
                    system("clear");
                    #endif
                    ++lineno;
                    continue;
                }
                
                auto tokens = Lexer::tokenize(line);
                auto ast = Parser::parse(tokens);
                if (!ast) {
                    print_traceback("<stdin>", lineno);
                } else {
                    // 只支持 BlockStmt
                    auto* block = dynamic_cast<BlockStmt*>(ast.get());
                    if (block) {
                        try {
                            for (auto& stmt : block->statements) {
                                try {
                                    interpreter.execute(stmt);
                                } catch (const std::exception& e) {
                                    std::cerr << "\033[1;31m错误: \033[0m" << e.what() << std::endl;
                                    break;
                                }
                            }
                        } catch (...) {
                            std::cerr << "\033[1;31m发生未知错误\033[0m" << std::endl;
                        }
                    }
                }
            } catch (...) {
                std::cerr << "\033[1;31mREPL 环境捕获到异常，但已恢复\033[0m" << std::endl;
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
    std::cout << "正在执行文件: " << argv[1] << std::endl;
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
                // 处理自定义运行时错误，提供更详细信息
                std::cerr << "\033[1;31m运行时错误 (行 " << currentLine << "): \033[0m" << re.what() << std::endl;
                std::cerr << "尝试继续执行后续代码..." << std::endl;
            } catch (const ReturnException&) {
                // 捕获并显示有意义的信息（函数外的return会导致这个异常）
                std::cerr << "\033[1;33m警告 (行 " << currentLine << "): \033[0m函数外使用了return语句" << std::endl;
                std::cerr << "尝试继续执行后续代码..." << std::endl;
            } catch (const BreakException&) {
                // 捕获并显示有意义的信息
                std::cerr << "\033[1;33m警告 (行 " << currentLine << "): \033[0m循环外使用了break语句" << std::endl;
                std::cerr << "尝试继续执行后续代码..." << std::endl;
            } catch (const ContinueException&) {
                // 捕获并显示有意义的信息
                std::cerr << "\033[1;33m警告 (行 " << currentLine << "): \033[0m循环外使用了continue语句" << std::endl;
                std::cerr << "尝试继续执行后续代码..." << std::endl;
            } catch (const std::exception& e) {
                // 其他标准异常
                std::cerr << "\033[1;31m错误 (行 " << currentLine << "): \033[0m" << e.what() << std::endl;
                std::cerr << "类型: " << typeid(e).name() << std::endl;
                std::cerr << "尝试继续执行后续代码..." << std::endl;
            } catch (...) {
                // 未知异常
                std::cerr << "\033[1;31m未知错误 (行 " << currentLine << ")\033[0m" << std::endl;
                std::cerr << "尝试继续执行后续代码..." << std::endl;
            }
        }
        std::cout << "\n程序执行完毕。" << std::endl;
    }
    return 0;
}
