#pragma once
#include "color_style.hpp"
#include "lamina_api/value.hpp"
#include "src_manger.hpp"
#include "version.hpp"

#include <iostream>

enum class WarningLevel: unsigned char {
    Low, Medium, High
};

LAMINA_API struct LmError {
    const std::string name;
    const std::string content;
    int err_code;
};

inline std::pair<std::string, std::string> get_warning_info(const WarningLevel level) {
    switch (level) {
        case WarningLevel::Low:
            return {"LOW", ConClr::CYAN};
        case WarningLevel::Medium:
            return {"MEDIUM", ConClr::YELLOW};
        case WarningLevel::High:
            return {"HIGH", ConClr::RED};
        default:
            return {"UNKNOWN", ConClr::RESET};
    }
}

inline std::string generate_separator(const int col_start, const int col_end, const int line_end) {
    std::stringstream ss;
    ss << std::to_string(line_end).size(); // 对齐行号
    for (int i = 1; i < col_start; ++i) ss << " ";
    const int length = std::max(1, col_end - col_start + 1);
    for (int i = 0; i < length; ++i) ss << "^";
    return ss.str();
}

LAMINA_API inline void error_reporter(
    const std::string& src_path,
    const int& src_line_start,
    const int& src_line_end,
    const int& src_col_start,
    const int& src_col_end,
    const LmError& error
) {
    // 获取错误位置的代码
    std::string error_line = get_slice(src_path, src_line_start, src_line_end);
    if (error_line.empty()) {
        error_line = "[Can't slice the source file]";
    }

    // 格式化错误信息
    std::cerr << "\n" << ConClr::RED << "Lamina Interpreter v" << LAMINA_VERSION << " raise this error"
        << ConClr::RESET <<std::endl;
    std::cerr << "\n" << ConClr::RESET << ConClr::RED << "ERROR [" << error.err_code << "]: "
              << error.name << ConClr::RESET << "\n";
    std::cerr << "  file: " << src_path << "\n";
    std::cerr << "  pos: ln " << src_line_start;
    if (src_line_start != src_line_end) {
        std::cerr << "-" << src_line_end;
    }
    std::cerr << ", ln " << src_col_start << "-" << src_col_end << "\n";
    std::cerr << "  [Info]: " << error.content << "\n\n";

    // 显示错误行代码
    std::cerr << "    " << src_line_start << " | " << error_line << "\n";
    std::cerr << "      " << generate_separator(src_col_start, src_col_end, src_line_end) << "\n\n";

    std::exit(error.err_code);
}

LAMINA_API inline void warning_reporter(
    const std::string& src_path,
    const WarningLevel& level,
    const int& src_line_start,
    const int& src_line_end,
    const int& src_col_start,
    const int& src_col_end,
    const std::string& message
) {
    // 获取错误位置的代码
    auto [level_str, color] = get_warning_info(level);
    std::string warning_line = get_slice(src_path, src_line_start, src_line_end);
    if (warning_line.empty()) {
        warning_line = "[Can't slice the source file]";
    }

    // 格式化警告信息
    std::cerr << "\n" << ConClr::RESET << color << "WARNING [" << level_str << "]: "
              << ConClr::RESET << message << "\n";
    std::cerr << "  file: " << src_path << "\n";
    std::cerr << "  pos: ln " << src_line_start;
    if (src_line_start != src_line_end) {
        std::cerr << "-" << src_line_end;
    }
    std::cerr << ", col " << src_col_start << "-" << src_col_end << "\n\n";

    // 显示警告行代码
    std::cerr << "    " << src_line_start << " | " << warning_line << "\n";
    std::cerr << "      " << generate_separator(src_col_start, src_col_end, src_line_end) << "\n\n";
}