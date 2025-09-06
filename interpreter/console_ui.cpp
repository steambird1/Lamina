#include "console_ui.hpp"
#include "interpreter.hpp"
#include "lamina.hpp"
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
#include <winnls.h>
#endif

#include <string>

int exec_block(const BlockStmt* block) {
    Interpreter interpreter;
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
    return 0;
}


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
        return 2;
    }

    // 只支持 BlockStmt
    auto block = dynamic_cast<BlockStmt*>(ast.get());
    if (!block) return 1;

    exec_block(block);
    std::cout << "\nProgram execution completed." << std::endl;
    return 0;
}

void enable_ansi_escape() {
#ifdef _WIN32
    // Windows 平台的颜色支持
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD outMode = 0;
        if (GetConsoleMode(hOut, &outMode)) {
            // 只有在成功获取控制台模式时才尝试设置
            if (!SetConsoleMode(hOut, outMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                // 设置失败只是警告，不影响程序运行
                std::cerr << "Warning: Console does not support ANSI color (STD_OUTPUT)" << std::endl;
            }
        } else {
            // 获取控制台模式失败（可能在测试环境或无控制台情况）
            // 静默失败，不输出错误信息，以免影响测试
        }
    }

    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hErr != INVALID_HANDLE_VALUE) {
        DWORD errMode = 0;
        if (GetConsoleMode(hErr, &errMode)) {
            if (!SetConsoleMode(hErr, errMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                std::cerr << "Warning: Console does not support ANSI color (STD_ERROR)" << std::endl;
            }
        }
        // 获取模式失败时静默处理
    }

    // UTF-8 编码设置
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

int argv_parser(const int argc, const char* const argv[]) {
    if (argc < 2) {
        return repl();
    }

    const std::vector<std::string> introduction = {"run", "version", "help", "repl"};
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; ++i) {
        arguments.emplace_back(argv[i]);// char* => string
    }

    bool is_valid = false;
    for (const auto& cmd: introduction) {
        if (arguments[1] == cmd) {
            is_valid = true;
            break;
        }
    }

    if (!is_valid && arguments.size() == 2) {
        // 如果不匹配任何指令, 默认执行argv[1]
        // if argv[1] not in introduction then run the argv[1](as a path)
        const std::string path = argv[1];
        arguments[1] = "run";
        arguments.emplace_back(path);
    }

    if (arguments[1] == "run") {
        if (arguments.size() != 3) {
            std::cout << "'run' command need 1 arguments but " << arguments.size() << "was given" << std::endl;
        }
        return run_file(arguments[2]);
    }

    if (arguments[1] == "version") {
        std::cout << LAMINA_VERSION << std::endl;
        return 0;
    }

    if (arguments[1] == "help") {
        print_help();
        return 0;
    }

    if (arguments[1] == "repl") {
        try {
            return repl();
        } catch (const CtrlCException&) {
            // 捕获Ctrl+C异常，友好提示后退出
            // Catch Ctrl+C exception and exit gracefully
            std::cout << "\n程序收到 Ctrl+C 信号，已安全退出。"
                      << "\nProgram received Ctrl+C signal, exited safely." << std::endl;
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "程序发生异常: " << e.what()
                      << "\nProgram exception occurred: " << e.what() << std::endl;
            return 1;
        } catch (...) {
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
    std::cout << "Lamina REPL. Press Ctrl+C or :exit to exit." << std::endl;
    std::cout << "Type :help for help." << std::endl;

    Interpreter interpreter;
    int lineno = 1; // 记录当前输入的行号，用于错误提示

    // REPL主循环
    std::string code_buffer;
    int brace_level = 0;

    while (true) {
        try {
            // 根据终端是否支持颜色，设置不同的提示符样式
            // Set prompt style based on whether the terminal supports color
            std::string prompt;
            if (brace_level > 0) {
                prompt = Interpreter::supports_colors()
                                 ? "\033[1;32m. \033[0m "   // 多行输入下等待输入的提示符
                                 : ". ";
            } else {
                prompt = Interpreter::supports_colors()
                                 ? "\033[1;32m> \033[0m "   // 带绿色高亮的提示符（ANSI转义序列）
                                 : "> ";                    // 普通提示符
            }

            std::string line = repl_readline(prompt);

            if (line == "\x03") {   // Ctrl+C interrupt
                if (brace_level > 0) {
                    code_buffer.clear();
                    brace_level = 0;
                    std::cout << "\nKeyboardInterrupt" << std::endl;
                    continue;
                }
                std::cout << "\n";
                break;
            } else if (line == "\x04") {    // Ctrl+D (EOF)
                if (brace_level > 0) {
                    // 在多行模式下，Ctrl+D 意味着输入结束，立即执行代码
                    // In multi-line mode, Ctrl+D means end of input, execute code immediately
                    goto execute_code;
                } else {
                    // 在主提示符的空行上，Ctrl+D 退出
                    // On an empty line at the main prompt, Ctrl+D exits
                    break;
                }
            }

            code_buffer += line + "\n";

            // 简单地通过计算大括号来检查代码块是否完整
            // Simply check if the code block is complete by counting braces
            for (char c: line) {
                if (c == '{') {
                    brace_level++;
                } else if (c == '}') {
                    brace_level--;
                }
            }

            if (brace_level > 0) {
                ++lineno;
                continue;
            }

        execute_code:           // 用于从Ctrl+D跳转 (Goto label for Ctrl+D)
            brace_level = 0;    // 重置 (Reset)

            if (code_buffer.empty() || code_buffer == "\n") {
                ++lineno;
                code_buffer.clear();
                continue;
            }

            std::string line_to_process = code_buffer;
            code_buffer.clear();

            // 只有在单行模式下才处理REPL命令
            // Only process REPL commands in single-line mode
            if (brace_level == 0) {
                std::string trimmed_line = line;
                trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r"));
                trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r") + 1);

                if (trimmed_line == ":exit") {
                    break;
                }
                if (trimmed_line == ":help") {
                    std::cout << "Lamina解释器命令 / Lamina Interpreter Commands:\n";
                    std::cout << "  :exit - 退出解释器 / Exit interpreter\n";
                    std::cout << "  :help - 显示此帮助信息 / Show this help message\n";
                    std::cout << "  :vars - 显示所有变量 / Show all variables\n";
                    std::cout << "  :clear - 清空屏幕 / Clear screen\n";
                    ++lineno;
                    continue;
                }
                if (trimmed_line == ":vars") {
                    interpreter.printVariables();
                    ++lineno;
                    continue;
                }
                if (trimmed_line == ":clear") {
#ifdef _WIN32
                    auto result = system("cls");    // Windows下使用cls命令
#else
                    auto result = system("clear");  // Linux/macOS下使用clear命令
#endif
                    (void) result;  // 显式忽略system函数的返回值，避免编译器警告
                    ++lineno;
                    continue;
                }
                if (trimmed_line == ":fns") {
                    ++lineno;
                    continue;
                }
            }

            // frontend
            auto tokens = Lexer::tokenize(line_to_process);
            auto ast = Parser::parse(tokens);

            // 检查AST是否生成成功
            // Check if AST generation succeeded
            if (!ast) {
                print_traceback("<stdin>", lineno);
                return 1;
            }
            // 仅支持BlockStmt类型的AST（代码块语句）
            // Only support AST of type BlockStmt (block statement)
            auto* block = dynamic_cast<BlockStmt*>(ast.get());
            if (!block) return 1;
            // 保存AST以保持函数指针有效
            interpreter.save_repl_ast(std::move(ast));

            try {
                // 执行代码块中的每个语句
                // Execute each statement in the block
                for (auto& stmt: block->statements) {
                    try {
                        interpreter.execute(stmt);
                    } catch (const RuntimeError& re) {
                        interpreter.print_stack_trace(re, true);
                        break;
                    } catch (const ReturnException&) {
                        Interpreter::print_warning(
                                "Return语句不能在函数外使用（第" + std::to_string(lineno) + "行）" +
                                        "Return statement used outside function (line " +
                                        std::to_string(lineno) + ")",
                                true);
                        break;
                    }
                    // 捕获break语句在循环外使用的异常
                    // Catch break statement used outside loop
                    catch (const BreakException&) {
                        Interpreter::print_warning(
                                "Break语句不能在循环外使用（第" + std::to_string(lineno) + "行）" +
                                        "Break statement used outside loop (line " + std::to_string(lineno) + ")",
                                true);
                        break;
                    }
                    // 捕获continue语句在循环外使用的异常
                    // Catch continue statement used outside loop
                    catch (const ContinueException&) {
                        Interpreter::print_warning(
                                "Continue语句不能在循环外使用（第" + std::to_string(lineno) + "行）" + "Continue statement used outside loop (line " +
                                        std::to_string(lineno) + ")",
                                true);
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
                        true);
            }
            // 捕获所有其他未预料的异常
            // Catch all other unexpected exceptions
            catch (...) {
                Interpreter::print_error("发生未知错误\nUnknown error occurred", true);
            }
        }
        // 捕获并处理REPL循环中的所有异常，确保REPL不崩溃
        // Catch and handle all exceptions in REPL loop to prevent crashes
        catch (...) {
            Interpreter::print_error(
                    "REPL环境捕获到异常，但已恢复\nREPL environment caught exception, but recovered",
                    true);
        }


        ++lineno;   // 行号递增
    }

    return 0;
}
