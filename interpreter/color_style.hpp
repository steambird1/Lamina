#pragma once
#include <string>
#include <mutex>
#include "interpreter.hpp"

// CC => Console Color
class ConsoleColor {
public:
    ConsoleColor(const ConsoleColor&) = delete;
    ConsoleColor& operator=(const ConsoleColor&) = delete;

    static std::string RESET;
    static std::string BLACK;
    static std::string RED;
    static std::string GREEN;
    static std::string YELLOW;
    static std::string BLUE;
    static std::string MAGENTA;
    static std::string CYAN;
    static std::string WHITE;
    static std::string LIGHT_BLACK;
    static std::string LIGHT_RED;
    static std::string LIGHT_GREEN;
    static std::string LIGHT_YELLOW;
    static std::string LIGHT_BLUE;
    static std::string LIGHT_MAGENTA;
    static std::string LIGHT_CYAN;
    static std::string LIGHT_WHITE;

    static void init() {
        static std::once_flag flag;
        std::call_once(flag, []() {
            const bool has_color = Interpreter::supports_colors();

            RESET = has_color ? "\033[0m" : "";
            BLACK = has_color ? "\033[30m" : "";
            RED = has_color ? "\033[31m" : "";
            GREEN = has_color ? "\033[32m" : "";
            YELLOW = has_color ? "\033[33m" : "";
            BLUE = has_color ? "\033[34m" : "";
            MAGENTA = has_color ? "\033[35m" : "";
            CYAN = has_color ? "\033[36m" : "";
            WHITE = has_color ? "\033[37m" : "";
            LIGHT_BLACK = has_color ? "\033[90m" : "";
            LIGHT_RED = has_color ? "\033[91m" : "";
            LIGHT_GREEN = has_color ? "\033[92m" : "";
            LIGHT_YELLOW = has_color ? "\033[93m" : "";
            LIGHT_BLUE = has_color ? "\033[94m" : "";
            LIGHT_MAGENTA = has_color ? "\033[95m" : "";
            LIGHT_CYAN = has_color ? "\033[96m" : "";
            LIGHT_WHITE = has_color ? "\033[97m" : "";
        });
    }

private:
    // 私有构造函数：防止创建实例
    ConsoleColor() = default;
};

std::string ConsoleColor::RESET;
std::string ConsoleColor::BLACK;
std::string ConsoleColor::RED;
std::string ConsoleColor::GREEN;
std::string ConsoleColor::YELLOW;
std::string ConsoleColor::BLUE;
std::string ConsoleColor::MAGENTA;
std::string ConsoleColor::CYAN;
std::string ConsoleColor::WHITE;
std::string ConsoleColor::LIGHT_BLACK;
std::string ConsoleColor::LIGHT_RED;
std::string ConsoleColor::LIGHT_GREEN;
std::string ConsoleColor::LIGHT_YELLOW;
std::string ConsoleColor::LIGHT_BLUE;
std::string ConsoleColor::LIGHT_MAGENTA;
std::string ConsoleColor::LIGHT_CYAN;
std::string ConsoleColor::LIGHT_WHITE;

using ConClr = ConsoleColor;
