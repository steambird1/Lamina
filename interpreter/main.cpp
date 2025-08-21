#include "console_ui.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
#include <winnls.h>

int main(const int argc, const char* const argv[]) {
    // Windows平台下设置控制台编码为UTF-8，确保中文等字符正常显示
    // Set console encoding to UTF-8 on Windows for proper display of Chinese characters
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // 设置控制台输出编码
    SetConsoleCP(CP_UTF8);        // 设置控制台输入编码
#endif
    return argv_parser(argc, argv);
}
