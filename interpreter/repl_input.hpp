#pragma once
#include <string>

// 读取一行输入，支持方向键编辑，兼容 Windows 和 Unix
std::string repl_readline(const std::string& prompt);
