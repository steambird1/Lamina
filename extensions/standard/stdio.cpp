#include <fstream>

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
        }
    }

    std::cout << std::endl;
    return Value();
}

#include <filesystem>

inline Value file_put_content(const std::vector<Value>& args) {
    if (args.size() < 2) {
        L_ERR("file_put_content requires at least 2 arguments: filename and content");
        return Value(false);
    }

    if (!args[0].is_string()) {
        L_ERR("The first argument of file_put_content must be a string (filename).");
        return Value(false);
    }

    std::string filename = args[0].to_string();

    // 检查文件是否存在
    if (!std::filesystem::exists(filename)) {
        L_ERR("File does not exist: " + filename);
        return Value(false);
    }

    if (!std::filesystem::is_regular_file(filename)) {
        L_ERR("Path is not a file: " + filename);
        return Value(false);
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        L_ERR("Failed to open existing file for writing: " + filename);
        return Value(false);
    }

    std::string content = args[1].to_string();
    file.write(content.data(), content.size());

    if (file.fail()) {
        L_ERR("Failed to write to existing file: " + filename);
        return Value(false);
    }

    return Value(static_cast<int>(content.size()));
}

inline Value file_get_content(const std::vector<Value>& args) {
    if (args.empty()) {
        L_ERR("file_get_content requires 1 argument: filename");
    }

    if (!args[0].is_string()) {
        L_ERR("The first argument of file_get_content must be a string (filename).");
    }

    std::ifstream file(args[0].to_string().c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        L_ERR("Failed to open file for reading: " + args[0].to_string());
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content;
    content.resize(static_cast<size_t>(size));

    if (!file.read(&content[0], size)) {
        L_ERR("Failed to read from file: " + args[0].to_string());
    }

    return Value(content);
}

namespace lamina {
    LAMINA_FUNC_MULTI_ARGS("input", input, 1);
    LAMINA_FUNC_WIT_ANY_ARGS("print", print);
    LAMINA_FUNC_MULTI_ARGS("file_put_content", file_put_content, 2);
    LAMINA_FUNC_MULTI_ARGS("file_get_content", file_get_content, 1);
}// namespace lamina
