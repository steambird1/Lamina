#pragma once
#include <iostream>
#include <string>

inline void print_traceback(const std::string& filename, int lineno, const std::string& msg = "invalid syntax") {
    std::cerr << "Traceback (most recent call last):\n";
    std::cerr << "  File \"" << filename << "\", line " << lineno << "\n";
    std::cerr << "SyntaxError: " << msg << "\n";
}
