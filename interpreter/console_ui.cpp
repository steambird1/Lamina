#include "console_ui.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "repl_input.hpp"
#include "trackback.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <typeinfo>

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>

int run_file(const std::string& path) {
    std::ifstream file(path);
        if (!file) {
            std::cerr << "Unable to open file: " << path << std::endl;
            return 1;
        }
        std::cout << "Executing file: " << path << std::endl;
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();
        auto tokens = Lexer::tokenize(source);
        auto ast = Parser::parse(tokens);
        Interpreter interpreter;

        // 注释掉自动加载minimal模块，改为按需加载
        // std::cout << "Loading minimal module..." << std::endl;
        // try {
        //     // 首先尝试从extensions目录加载
        //     auto moduleLoader = std::make_unique<ModuleLoader>("../extensions/minimal.dll");
        //     if (!moduleLoader->isLoaded()) {
        //         // 如果失败，尝试从当前目录加载
        //         moduleLoader = std::make_unique<ModuleLoader>("minimal.dll");
        //     }
        //     if (moduleLoader->isLoaded()) {
        //         std::cout << "Module loaded successfully, registering to interpreter..." << std::endl;
        //         if (moduleLoader->registerToInterpreter(interpreter)) {
        //             std::cout << "Module registered successfully!" << std::endl;
        //         } else {
        //             std::cerr << "Failed to register module to interpreter!" << std::endl;
        //         }
        //     } else {
        //         std::cerr << "Failed to load module!" << std::endl;
        //     }
        // } catch (const std::exception& e) {
        //     std::cerr << "Exception during module loading: " << e.what() << std::endl;
        // }

        if (!ast) {
            print_traceback(path, 1);
            exit(2);
        }

        // 只支持 BlockStmt
        auto* block = dynamic_cast<BlockStmt*>(ast.get());
        if (block) {
            // 每个语句独立执行并捕获异常，但保持解释器状态
            int currentLine = 0;
            for (auto& stmt: block->statements) {
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

int argv_parser(const int argc, const char* const argv[]) {
    const std::string VERSION = "0.1.0";

    if (argc < 2) {
        try {
            return repl();
        }
        catch (const CtrlCException&) {
            // 捕获Ctrl+C异常，友好提示后退出
            // Catch Ctrl+C exception and exit gracefully
            std::cout << "\n程序收到 Ctrl+C 信号，已安全退出。"
                      << "\nProgram received Ctrl+C signal, exited safely." << std::endl;
            return 0;
        }
        catch (const std::exception& e) {
            std::cerr << "程序发生异常: " << e.what()
                      << "\nProgram exception occurred: " << e.what() << std::endl;
            return 1;
        }
        catch (...) {
            std::cerr << "程序发生未知异常"
                      << "\nProgram encountered an unknown exception." << std::endl;
            return 1;
        }
    }

    const std::vector<std::string> introduction = {"run", "version", "help", "repl"};
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; ++i) {
        arguments.emplace_back(argv[i]); // char* => string
    }

    bool is_valid = false;
    for (const auto & cmd : introduction) {
        if (arguments[1] == cmd) {
            is_valid = true;
            break;
        }
    }

    if (!is_valid && arguments.size()==2) {
        // 如果不匹配任何指令, 默认执行argv[1]
        // if argv[1] not in introduction then run the argv[1](as a path)
        const std::string path = argv[1];
        arguments[1] = "run";
        arguments.emplace_back(path);
    }

    if (arguments[1]=="run") {
        if (arguments.size()!=3) {
            std::cout << "'run' command need 1 arguments but " << arguments.size() << "was given"<< std::endl;
        }
        return run_file(arguments[2]);
    }

    if (arguments[1]=="version") {
        std::cout << VERSION << std::endl;
        return 0;
    }

    if (arguments[1]=="help") {
        print_help();
        return 0;
    }

    if (arguments[1] == "repl") {
        try {
            return repl();
        }
        catch (const CtrlCException&) {
            // 捕获Ctrl+C异常，友好提示后退出
            // Catch Ctrl+C exception and exit gracefully
            std::cout << "\n程序收到 Ctrl+C 信号，已安全退出。"
                      << "\nProgram received Ctrl+C signal, exited safely." << std::endl;
            return 0;
        }
        catch (const std::exception& e) {
            std::cerr << "程序发生异常: " << e.what()
                      << "\nProgram exception occurred: " << e.what() << std::endl;
            return 1;
        }
        catch (...) {
            std::cerr << "程序发生未知异常"
                      << "\nProgram encountered an unknown exception." << std::endl;
            return 1;
        }
    }

    std::cout << "Unknown command: " << argv[1] << std::endl;
    print_help();
    return 1;
}


int repl() {
        // 输出REPL欢迎信息和退出提示
        // Print REPL welcome message and exit instructions
        std::cout << "Lamina REPL. Press Ctrl+C or :exit to exit." << std::endl;
        std::cout << "Type :help for help." << std::endl;

        Interpreter interpreter;  // 创建解释器实例
        int lineno = 1;           // 记录当前输入的行号，用于错误提示

        // REPL主循环：持续读取-执行-输出
        // REPL main loop: continuously read-execute-print
        while (true) {
            try {
                // 根据终端是否支持颜色，设置不同的提示符样式
                // Set prompt style based on whether the terminal supports color
                std::string prompt = Interpreter::supports_colors()
                    ? "\033[1;32m>\033[0m "  // 带绿色高亮的提示符（ANSI转义序列）
                    : "> ";                  // 普通提示符

                std::string line;           // 存储用户输入的一行内容

                // 读取用户输入
                // Read user input
                try {
                    line = repl_readline(prompt);
                } catch (const CtrlCException&) {
                    break;
                }

                // 处理EOF（如用户输入Ctrl+D），退出循环
                // Handle EOF (e.g., user input Ctrl+D) and exit loop
                if (std::cin.eof()) break;

                // 忽略空行输入
                // Ignore empty input lines
                if (line.empty()) {
                    ++lineno;
                    continue;
                }

                // 处理:exit命令：退出REPL
                // Handle :exit command: exit REPL
                if (line == ":exit") {
                    break;
                }

                // 处理:help命令：显示帮助信息
                // Handle :help command: show help message
                if (line == ":help") {
                    std::cout << "Lamina解释器命令 / Lamina Interpreter Commands:\n";
                    std::cout << "  :exit - 退出解释器 / Exit interpreter\n";
                    std::cout << "  :help - 显示此帮助信息 / Show this help message\n";
                    std::cout << "  :vars - 显示所有变量 / Show all variables\n";
                    std::cout << "  :clear - 清空屏幕 / Clear screen\n";
                    ++lineno;
                    continue;
                }

                // 处理:vars命令：打印当前所有变量
                // Handle :vars command: print all current variables
                if (line == ":vars") {
                    interpreter.printVariables();
                    ++lineno;
                    continue;
                }

                // 处理:clear命令：清空屏幕（跨平台实现）
                // Handle :clear command: clear screen (cross-platform implementation)
                if (line == ":clear") {
#ifdef _WIN32
                    auto result = system("cls");  // Windows下使用cls命令
#else
                    auto result = system("clear"); // Linux/macOS下使用clear命令
#endif
                    (void)result;  // 显式忽略system函数的返回值，避免编译器警告
                    ++lineno;
                    continue;
                }

                // 预留:fns命令（暂未实现）
                // Reserved :fns command (not implemented yet)
                if (line == ":fns") {
                    ++lineno;
                    continue;
                }

                // frontend
                auto tokens = Lexer::tokenize(line);
                auto ast = Parser::parse(tokens);

                // 检查AST是否生成成功
                // Check if AST generation succeeded
                if (!ast) {
                    print_traceback("<stdin>", lineno);
                } else {
                    // 仅支持BlockStmt类型的AST（代码块语句）
                    // Only support AST of type BlockStmt (block statement)
                    auto* block = dynamic_cast<BlockStmt*>(ast.get());
                    if (block) {
                        // 保存AST以保持函数指针有效
                        // Save AST to keep function pointers valid
                        interpreter.save_repl_ast(std::move(ast));

                        try {
                            // 执行代码块中的每个语句
                            // Execute each statement in the block
                            for (auto& stmt : block->statements) {
                                try {
                                    interpreter.execute(stmt);
                                }
                                catch (const RuntimeError& re) {
                                    interpreter.print_stack_trace(re, true);
                                    break;
                                }
                                catch (const ReturnException&) {
                                    Interpreter::print_warning(
                                        "Return语句不能在函数外使用（第" + std::to_string(lineno) + "行）"
                                        "Return statement used outside function (line " + std::to_string(lineno) + ")",
                                        true
                                    );
                                    break;
                                }
                                // 捕获break语句在循环外使用的异常
                                // Catch break statement used outside loop
                                catch (const BreakException&) {
                                    Interpreter::print_warning(
                                        "Break语句不能在循环外使用（第" + std::to_string(lineno) + "行）"
                                        "Break statement used outside loop (line " + std::to_string(lineno) + ")",
                                        true
                                    );
                                    break;
                                }
                                // 捕获continue语句在循环外使用的异常
                                // Catch continue statement used outside loop
                                catch (const ContinueException&) {
                                    Interpreter::print_warning(
                                        "Continue语句不能在循环外使用（第" + std::to_string(lineno) + "行）"
                                        "Continue statement used outside loop (line " + std::to_string(lineno) + ")",
                                        true
                                    );
                                    break;
                                }
                                // 捕获其他标准异常
                                // Catch other standard exceptions
                                catch (const std::exception& e) {
                                    Interpreter::print_error(e.what(), true);
                                    break;
                                }
                            }
                        }
                        // 捕获代码块执行过程中的顶层运行时错误
                        // Catch top-level runtime errors during block execution
                        catch (const RuntimeError& re) {
                            interpreter.print_stack_trace(re, true);
                        }
                        // 捕获从内层循环逃逸的return异常
                        // Catch return exceptions escaping the inner loop
                        catch (const ReturnException&) {
                            Interpreter::print_warning(
                                "Return语句不能在函数外使用\nReturn statement used outside function",
                                true
                            );
                        }
                        // 捕获所有其他未预料的异常
                        // Catch all other unexpected exceptions
                        catch (...) {
                            Interpreter::print_error("发生未知错误\nUnknown error occurred", true);
                        }
                    }
                }
            }
            // 捕获并处理REPL循环中的所有异常，确保REPL不崩溃
            // Catch and handle all exceptions in REPL loop to prevent crashes
            catch (...) {
                Interpreter::print_error(
                    "REPL环境捕获到异常，但已恢复\nREPL environment caught exception, but recovered",
                    true
                );
            }

            ++lineno;  // 行号递增
        }

        return 0;
    }