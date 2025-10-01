#pragma once
#include <stdexcept>
#include <string>

// Ctrl+C 中断异常
class CtrlCException final : public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "Ctrl+C interrupt";
    }
};

// 读取一行输入，支持方向键编辑，兼容 Windows 和 Unix
std::string repl_readline(const std::string& prompt);

int getchar();
