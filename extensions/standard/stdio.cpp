#include "interpreter.hpp"
#include "lamina.hpp"
#include "value.hpp"

inline Value input(const std::vector<Value>& args) {
    std::string input_line;

    if (args.size() == 1) {
        std::cout << args[0].to_string();
    }

    if (std::getline(std::cin, input_line)) {
        // Try to parse as number first
        try {
            // Check if it contains a decimal point for float
            if (input_line.find('.') != std::string::npos) {
                double d = std::stod(input_line);
                return Value(d);
            } else {
                int i = std::stoi(input_line);
                return Value(i);
            }
        } catch (...) {
            // Return as string if not a number
            return Value(input_line);
        }
    }

    // Return empty string if input failed
    return Value("");
}

inline Value print(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << args[i].to_string();
        if (i != args.size() - 1) {
            std::cout << " ";
            LAMINA_INT(1);
        }
    }
    std::cout << std::endl;
    return LAMINA_INT(1);
}

namespace lamina {
    LAMINA_FUNC_MULTI_ARGS("input", input, 1);
    LAMINA_FUNC_WIT_ANY_ARGS("print", print);
}
