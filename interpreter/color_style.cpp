#include "color_style.hpp"

LAMINA_API std::string ConsoleColor::RESET;
LAMINA_API std::string ConsoleColor::BLACK;
LAMINA_API std::string ConsoleColor::RED;
LAMINA_API std::string ConsoleColor::GREEN;
LAMINA_API std::string ConsoleColor::YELLOW;
LAMINA_API std::string ConsoleColor::BLUE;
LAMINA_API std::string ConsoleColor::MAGENTA;
LAMINA_API std::string ConsoleColor::CYAN;
LAMINA_API std::string ConsoleColor::WHITE;
LAMINA_API std::string ConsoleColor::LIGHT_BLACK;
LAMINA_API std::string ConsoleColor::LIGHT_RED;
LAMINA_API std::string ConsoleColor::LIGHT_GREEN;
LAMINA_API std::string ConsoleColor::LIGHT_YELLOW;
LAMINA_API std::string ConsoleColor::LIGHT_BLUE;
LAMINA_API std::string ConsoleColor::LIGHT_MAGENTA;
LAMINA_API std::string ConsoleColor::LIGHT_CYAN;
LAMINA_API std::string ConsoleColor::LIGHT_WHITE;

namespace {
    const bool init_console_color = []() {
        ConsoleColor::init();
        return true;
    }();
}