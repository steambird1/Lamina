#pragma once
#include <string>
#include <variant>

class Value {
public:
    enum class Type { String, Int };
    Type type;
    std::variant<std::string, int> data;
    Value() : type(Type::String), data("") {}
    Value(const std::string& s) : type(Type::String), data(s) {}
    Value(int i) : type(Type::Int), data(i) {}
    std::string to_string() const {
        if (type == Type::String) return std::get<std::string>(data);
        if (type == Type::Int) return std::to_string(std::get<int>(data));
        return "<unknown>";
    }
};
