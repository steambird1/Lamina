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
#ifdef _WIN32
    // Windows 平台：直接调用 _getch（避免与标准库 getch 冲突）
    return _getch();
#else
    // Linux/macOS 平台：通过 termios 临时关闭终端回显和缓冲
    struct termios old_attr, new_attr;
    int ch;

    // 1. 获取当前终端属性并保存（用于后续恢复）
    if (tcgetattr(STDIN_FILENO, &old_attr) != 0) {
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }
    new_attr = old_attr;  // 复制属性作为修改基础

    // 2. 修改属性：关闭回显（ICANON）和缓冲（ECHO）
    new_attr.c_lflag &= ~(ICANON | ECHO);
    new_attr.c_cc[VMIN] = 1;  // 至少读取 1 个字符才返回
    new_attr.c_cc[VTIME] = 0; // 不设置超时，立即返回

    // 3. 应用新的终端属性
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_attr) != 0) {
        perror("tcsetattr failed");
        exit(EXIT_FAILURE);
    }

    // 4. 读取字符（此时无回显、无缓冲）
    ch = getchar();

    // 5. 恢复终端原始属性（关键：避免影响后续终端行为）
    tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);

    return ch;
#endif
}