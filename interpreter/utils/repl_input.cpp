#include "repl_input.hpp"
#include <iostream>
#include <signal.h>
#include <vector>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#ifdef _WIN32
void CtrlCExit() {
    std::cout << "^C" << std::endl;
    throw CtrlCException();
}
#else
struct termios oldt;
volatile sig_atomic_t g_running = 1;    // 按下Ctrl+C后变为0
// void CtrlCExit() {
//     tcsetattr(STDIN_FILENO, TCSANOW, &oldt);// 恢复终端设置
//     std::cout << "^C" << std::endl;
//     throw CtrlCException();
// }
bool input_available() {    // 防止getchar阻塞
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval timeout = {0, 0};
    return (select(1, &fds, nullptr, nullptr, &timeout) > 0);
}
#endif

std::string repl_readline(const std::string& prompt) {
    // ToDo: ...
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int getchar() {
    // ToDo: ...
}