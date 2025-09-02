#include "console_ui.hpp"
#include "repl_input.hpp"

int main(const int argc, const char* const argv[]) {
    try {
        enable_ansi_escape();          // 设置编码
        return argv_parser(argc, argv);// 主要执行这里

    } catch (const CtrlCException&) {
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
