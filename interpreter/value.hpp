#pragma once
#include <string>
#include <variant>
#include <vector>

class Value {
public:
    enum class Type { String, Int, Array };
    Type type;
    std::variant<std::string, int, std::vector<Value>> data;
    Value() : type(Type::String), data("") {}
    Value(const std::string& s) : type(Type::String), data(s) {}
    Value(int i) : type(Type::Int), data(i) {}
    Value(const std::vector<Value>& arr) : type(Type::Array), data(arr) {}
    std::string to_string() const {
        if (type == Type::String) return std::get<std::string>(data);
        if (type == Type::Int) return std::to_string(std::get<int>(data));
        if (type == Type::Array) {
            std::string res = "[";
            const auto& arr = std::get<std::vector<Value>>(data);
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i) res += ", ";
                res += arr[i].to_string();
            }
            res += "]";
            return res;
        }
        return "<unknown>";
    }
};
