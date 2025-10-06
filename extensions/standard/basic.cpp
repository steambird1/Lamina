#include <fstream>

#include "../../interpreter/lamina_api/lamina.hpp"
#include "../../interpreter/lamina_api/value.hpp"
#include "std.hpp"

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

/**
 * 终端内执行系统指令
 */
inline Value exec(const std::vector<Value>& args) {
    if (args.empty()) {
        L_ERR("exec requires 1 argument: command");
    }

    if (!args[0].is_string()) {
        L_ERR("The first argument of exec must be a string (command).");
    }

    std::string command = args[0].to_string();
    int result = std::system(command.c_str());

    if (result != 0) {
        L_ERR("Command execution failed: " + command);
    }

    return Value(result);
}

inline Value exist(const std::vector<Value>& args) {
    if (args.empty()) {
        L_ERR("exist() requires 1 argument: filename");
        return Value(false);
    }

    if (!args[0].is_string()) {
        L_ERR("The first argument of exist must be a string (filename).");
        return Value(false);
    }

    std::string filename = args[0].to_string();
    bool exists = std::filesystem::exists(filename);
    return Value(exists);
}

inline Value touch_file(const std::vector<Value>& args) {
    if (args.empty()) {
        L_ERR("touch_file() requires 1 argument: filename");
        return Value(false);
    }

    if (!args[0].is_string()) {
        L_ERR("The first argument of touch_file must be a string (filename).");
        return Value(false);
    }

    std::string filename = args[0].to_string();

    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        L_ERR("Failed to touch file: " + filename);
        return Value(false);
    }

    std::filesystem::last_write_time(filename, std::filesystem::file_time_type::clock::now());

    return Value(true);
}

inline Value assert(const std::vector<Value>& args) {
    const bool cond = !args.empty()
                              ? args[0].as_bool()
                              : false;
    const auto msg = args.size() > 1
                             ? args[1].to_string()
                             : "None";
    if (!cond) {
        L_ERR("Assertion: " + msg);
    }

    return LAMINA_NULL;
}

inline Value typeof_(const std::vector<Value>& args) {
    if (args.empty()) {
        return LAMINA_NULL;
    }
    switch (args[0].type) {
        case Value::Type::Lambda:    return LAMINA_STRING("lambda");
        case Value::Type::lmStruct:  return LAMINA_STRING("struct");
        case Value::Type::Symbolic:  return LAMINA_STRING("symbolic");
        case Value::Type::Null:      return LAMINA_STRING("null");
        case Value::Type::Bool:      return LAMINA_STRING("bool");
        case Value::Type::Int:       return LAMINA_STRING("int");
        case Value::Type::Float:     return LAMINA_STRING("float");
        case Value::Type::BigInt:    return LAMINA_STRING("bigint");
        case Value::Type::Rational:  return LAMINA_STRING("rational");
        case Value::Type::Irrational:return LAMINA_STRING("irrational");
        case Value::Type::String:    return LAMINA_STRING("string");
        case Value::Type::Array:     return LAMINA_STRING("array");
        case Value::Type::Set:       return LAMINA_STRING("set");
        case Value::Type::Matrix:    return LAMINA_STRING("matrix");
        case Value::Type::lmCppFunction:return LAMINA_STRING("cpp_func");
        case Value::Type::lmModule:  return LAMINA_STRING("module");
        default: return LAMINA_NULL;
    }

}
