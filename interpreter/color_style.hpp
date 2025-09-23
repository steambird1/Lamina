#pragma once
#include <string>
#include <mutex>
#include "interpreter.hpp"

class LAMINA_API ConsoleColor {
public:
    ConsoleColor(const ConsoleColor&) = delete;
    ConsoleColor& operator=(const ConsoleColor&) = delete;

    static LAMINA_API std::string RESET;
    static LAMINA_API std::string BLACK;
    static LAMINA_API std::string RED;
    static LAMINA_API std::string GREEN;
    static LAMINA_API std::string YELLOW;
    static LAMINA_API std::string BLUE;
    static LAMINA_API std::string MAGENTA;
    static LAMINA_API std::string CYAN;
    static LAMINA_API std::string WHITE;
    static LAMINA_API std::string LIGHT_BLACK;
    static LAMINA_API std::string LIGHT_RED;
    static LAMINA_API std::string LIGHT_GREEN;
    static LAMINA_API std::string LIGHT_YELLOW;
    static LAMINA_API std::string LIGHT_BLUE;
    static LAMINA_API std::string LIGHT_MAGENTA;
    static LAMINA_API std::string LIGHT_CYAN;
    static LAMINA_API std::string LIGHT_WHITE;

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
    ConsoleColor() = default;
};

using ConClr = ConsoleColor;