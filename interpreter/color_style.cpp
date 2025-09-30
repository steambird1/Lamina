
#include "color_style.hpp"

#include "interpreter.hpp"
void ConClr::init(const bool use_color) {
    RESET = use_color ? "\033[0m" : "";
    BLACK = use_color ? "\033[30m" : "";
    RED = use_color ? "\033[31m" : "";
    GREEN = use_color ? "\033[32m" : "";
    YELLOW = use_color ? "\033[33m" : "";
    BLUE = use_color ? "\033[34m" : "";
    MAGENTA = use_color ? "\033[35m" : "";
    CYAN = use_color ? "\033[36m" : "";
    WHITE = use_color ? "\033[37m" : "";
    LIGHT_BLACK = use_color ? "\033[90m" : "";
    LIGHT_RED = use_color ? "\033[91m" : "";
    LIGHT_GREEN = use_color ? "\033[92m" : "";
    LIGHT_YELLOW = use_color ? "\033[93m" : "";
    LIGHT_BLUE = use_color ? "\033[94m" : "";
    LIGHT_MAGENTA = use_color ? "\033[95m" : "";
    LIGHT_CYAN = use_color ? "\033[96m" : "";
    LIGHT_WHITE = use_color ? "\033[97m" : "";
}
