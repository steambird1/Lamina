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
    std::cout << prompt;
    std::cout.flush();    // 确保提示符立即显示
    std::string input;

    int cursor_pos = 0;  // 跟踪光标位置

    while (true) {
        int c = console_getchar();

        // 处理方向键（Windows使用0xE0作为扩展键前缀）
        #ifdef _WIN32
        if (c == 0xE0) {  // 扩展键前缀
            c = console_getchar();  // 获取实际键值
            if (c == 0x4B){ // 左方向键 <-
                if (cursor_pos > 0) {
                    cursor_pos--;
                    move_cursor(-1);  // 左移光标
                    continue;
                }
            if (c == 0x4D){ // 右方向键 <-
                if (cursor_pos < static_cast<int>(input.size())) {
                    cursor_pos++;
                    move_cursor(1);   // 右移光标
                }
                continue;
            }
        }
        #else
        // Linux/macOS 方向键是ESC序列：左=\033[D，右=\033[C
        if (c == '\033') {  // ESC字符
            if (console_getchar() == '[') {  // 转义序列前缀
                switch (console_getchar()) {
                    case 'D':  // 左方向键
                        if (cursor_pos > 0) {
                            cursor_pos--;
                            move_cursor(-1);
                        }
                        continue;
                    case 'C':  // 右方向键
                        if (cursor_pos < static_cast<int>(input.size())) {
                            cursor_pos++;
                            move_cursor(1);
                        }
                        continue;
                }
            }
        }
        #endif

        // 处理回车/换行
        if (c == '\r' || c == '\n') {
            #ifdef _WIN32
            if (c == '\r') std::cout << '\n';
            #else
            std::cout << '\n';
            #endif
            break;
        }
        // 处理退格
        else if (c == '\b' || c == 127) {
            if (cursor_pos > 0) {
                // 从字符串中删除光标前的字符
                input.erase(cursor_pos - 1, 1);
                cursor_pos--;

                // 更新显示
                move_cursor(-1);
                std::cout << input.substr(cursor_pos) << ' ' << "\b";
                // 移动光标到正确位置
                move_cursor(static_cast<int>(input.size()) - cursor_pos);
                std::cout.flush();
            }
        }
        // 处理可打印字符
        else if (c >= 32 && c <= 126) {
            // 在光标位置插入字符
            input.insert(cursor_pos, 1, static_cast<char>(c));
            // 显示插入的字符和后面的内容
            std::cout << static_cast<char>(c) << input.substr(cursor_pos + 1);
            cursor_pos++;
            // 移动光标
            move_cursor(static_cast<int>(input.size()) - cursor_pos);
            std::cout.flush();
        }
    }
}

int console_getchar() {
#ifdef _WIN32
    // Windows 平台：直接调用 _getch（避免与标准库 getch 冲突）
    return _getch();
#else
    // Linux/macOS 平台：通过 termios 临时关闭终端回显和缓冲
    struct termios old_attr, new_attr;
    int ch;

    // 获取当前终端属性并保存（用于后续恢复）
    if (tcgetattr(STDIN_FILENO, &old_attr) != 0) {
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }
    new_attr = old_attr;  // 复制属性作为修改基础

    // 修改属性：关闭回显（ICANON）和缓冲（ECHO）
    new_attr.c_lflag &= ~(ICANON | ECHO);
    new_attr.c_cc[VMIN] = 1;  // 至少读取 1 个字符才返回
    new_attr.c_cc[VTIME] = 0; // 不设置超时，立即返回

    // 应用新的终端属性
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_attr) != 0) {
        perror("tcsetattr failed");
        exit(EXIT_FAILURE);
    }

    // 读取字符（此时无回显、无缓冲）
    ch = getchar();

    // 恢复终端原始属性（关键：避免影响后续终端行为）
    tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);

    return ch;
#endif
}

// 终端光标移动控制函数
void move_cursor(int steps) {
#ifdef _WIN32
#include <windows.h>
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    COORD new_pos = csbi.dwCursorPosition;
    new_pos.X += steps;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), new_pos);
#else
    // Linux/macOS 平台使用ANSI转义序列
    if (steps > 0) {
        // 右移
        printf("\033[%dC", steps);
    } else if (steps < 0) {
        // 左移
        printf("\033[%dD", -steps);
    }
    fflush(stdout);
#endif
}