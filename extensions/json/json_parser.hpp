#pragma once

#include "lamina.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <functional>

enum class json_token {
    JSON_BEGIN,
    JSON_END,
    JSON_ARRAY_BEGIN,   // [
    JSON_ARRAY_END,     // ]
    JSON_OBJECT_BEGIN,  // {
    JSON_OBJECT_END,    // }
    JSON_STRING,        // 字符串
    JSON_INTEGER,       // 整数
    JSON_FLOAT,         // 浮点数
    JSON_TRUE,          // true
    JSON_FALSE,         // false
    JSON_NULL,          // null
    JSON_COLON,         // :
    JSON_COMMA          // ,
};

struct state {
    json_token token;
    std::string value;
};

struct json_value {
    enum class type {
        OBJECT, ARRAY, STRING, INTEGER, FLOAT, BOOLEAN, NULL_VALUE
    };

    type t;
    std::string str_val;
    int int_val;
    double float_val;
    bool bool_val;
    std::vector<json_value> array_val;
    std::unordered_map<std::string, std::unique_ptr<json_value>> object_val;

    // 默认构造函数
    json_value() : t(type::NULL_VALUE) {}
    // 类型构造函数
    json_value(type type) : t(type) {}

    // 移动构造和移动赋值（必须显式默认，因存在自定义成员）
    json_value(json_value&&) = default;
    json_value& operator=(json_value&&) = default;

    // 禁用拷贝（避免智能指针管理的对象被拷贝）
    json_value(const json_value&) = delete;
    json_value& operator=(const json_value&) = delete;
};

// 前向声明
json_value parse_json_data(const std::vector<state>& tokens, size_t& index);

// 加inline避免头文件函数多重定义
inline std::vector<state> tokenize(const std::string& str) {
    std::vector<state> tokens;
    size_t i = 0;
    const size_t n = str.size();

    while (i < n) {
        if (std::isspace(static_cast<unsigned char>(str[i]))) {
            i++;
            continue;
        }

        char c = str[i];

        if (c == '{') {
            tokens.push_back({json_token::JSON_OBJECT_BEGIN, "{"});
            i++;
        } else if (c == '}') {
            tokens.push_back({json_token::JSON_OBJECT_END, "}"});
            i++;
        } else if (c == '[') {
            tokens.push_back({json_token::JSON_ARRAY_BEGIN, "["});
            i++;
        } else if (c == ']') {
            tokens.push_back({json_token::JSON_ARRAY_END, "]"});
            i++;
        } else if (c == ':') {
            tokens.push_back({json_token::JSON_COLON, ":"});
            i++;
        } else if (c == ',') {
            tokens.push_back({json_token::JSON_COMMA, ","});
            i++;
        }
        else if (c == '"') {
            i++;
            size_t start = i;
            bool escaped = false;

            while (i < n) {
                if (escaped) {
                    escaped = false;
                    i++;
                } else if (str[i] == '"') {
                    break;
                } else if (str[i] == '\\') {
                    escaped = true;
                    i++;
                } else {
                    i++;
                }
            }

            if (i >= n) {
                L_ERR("Unterminated string in JSON");
                throw std::runtime_error("Unterminated string");
            }

            std::string value = str.substr(start, i - start);
            tokens.push_back({json_token::JSON_STRING, value});
            i++;
        }
        else if (c == '\'') {
            i++;
            size_t start = i;
            bool escaped = false;

            while (i < n) {
                if (escaped) {
                    escaped = false;
                    i++;
                } else if (str[i] == '\'') {
                    break;
                } else if (str[i] == '\\') {
                    escaped = true;
                    i++;
                } else {
                    i++;
                }
            }

            if (i >= n) {
                L_ERR("Unterminated string in JSON");
                throw std::runtime_error("Unterminated string");
            }

            std::string value = str.substr(start, i - start);
            tokens.push_back({json_token::JSON_STRING, value});
            i++;
        }
        else if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') {
            size_t start = i;
            bool has_dot = false;

            if (c == '-') {
                i++;
                if (i >= n || !std::isdigit(static_cast<unsigned char>(str[i]))) {
                    L_ERR("Invalid number format: '-' not followed by digit");
                    throw std::runtime_error("Invalid number format");
                }
            }

            while (i < n) {
                if (std::isdigit(static_cast<unsigned char>(str[i]))) {
                    i++;
                } else if (str[i] == '.') {
                    if (has_dot) {
                        L_ERR("Invalid number format: multiple dots");
                        throw std::runtime_error("Invalid number format");
                    }
                    has_dot = true;
                    i++;
                    if (i >= n || !std::isdigit(static_cast<unsigned char>(str[i]))) {
                        L_ERR("Invalid number format: dot not followed by digit");
                        throw std::runtime_error("Invalid number format");
                    }
                } else {
                    break;
                }
            }

            std::string num_str = str.substr(start, i - start);
            if (has_dot) {
                tokens.push_back({json_token::JSON_FLOAT, num_str});
            } else {
                tokens.push_back({json_token::JSON_INTEGER, num_str});
            }
        }
        else if (std::isalpha(static_cast<unsigned char>(c))) {
            size_t start = i;
            while (i < n && std::isalpha(static_cast<unsigned char>(str[i]))) {
                i++;
            }

            std::string keyword = str.substr(start, i - start);
            if (keyword == "true") {
                tokens.push_back({json_token::JSON_TRUE, "true"});
            } else if (keyword == "false") {
                tokens.push_back({json_token::JSON_FALSE, "false"});
            } else if (keyword == "null") {
                tokens.push_back({json_token::JSON_NULL, "null"});
            } else {
                L_ERR(std::string("Unexpected keyword: ") + keyword);
                throw std::runtime_error("Unexpected keyword: " + keyword);
            }
        }
        else {
            L_ERR(std::string("Unexpected character: ") + c);
            throw std::runtime_error("Unexpected character in JSON");
        }
    }

    return tokens;
}

json_value parse_object(const std::vector<state>& tokens, size_t& index) {
    json_value obj(json_value::type::OBJECT);
    index++;  // 跳过 {

    while (index < tokens.size() && tokens[index].token != json_token::JSON_OBJECT_END) {
        if (tokens[index].token != json_token::JSON_STRING) {
            L_ERR("Expected string key in object");
            throw std::runtime_error("Expected string key in object");
        }

        std::string key = tokens[index].value;
        index++;

        if (index >= tokens.size() || tokens[index].token != json_token::JSON_COLON) {
            L_ERR("Expected colon after key");
            throw std::runtime_error("Expected colon after key");
        }
        index++;  // 跳过 :

        json_value value = parse_json_data(tokens, index);
        // 修复：通过make_unique构造智能指针，并用move转移value所有权
        obj.object_val[key] = std::make_unique<json_value>(std::move(value));

        if (index < tokens.size() && tokens[index].token == json_token::JSON_COMMA) {
            index++;  // 跳过 ,
        } else if (tokens[index].token != json_token::JSON_OBJECT_END) {
            L_ERR("Expected comma or closing brace in object");
            throw std::runtime_error("Expected comma or closing brace in object");
        }
    }

    if (index < tokens.size() && tokens[index].token == json_token::JSON_OBJECT_END) {
        index++;  // 跳过 }
    } else {
        L_ERR("Unclosed JSON object");
        throw std::runtime_error("Unclosed JSON object");
    }
    return obj;
}

json_value parse_array(const std::vector<state>& tokens, size_t& index) {
    json_value arr(json_value::type::ARRAY);
    index++;  // 跳过 [

    while (index < tokens.size() && tokens[index].token != json_token::JSON_ARRAY_END) {
        json_value element = parse_json_data(tokens, index);
        // 修复：使用move避免拷贝（因element是局部变量，可转移所有权）
        arr.array_val.push_back(std::move(element));

        if (index < tokens.size() && tokens[index].token == json_token::JSON_COMMA) {
            index++;  // 跳过 ,
        } else if (tokens[index].token != json_token::JSON_ARRAY_END) {
            L_ERR("Expected comma or closing bracket in array");
            throw std::runtime_error("Expected comma or closing bracket in array");
        }
    }

    if (index < tokens.size() && tokens[index].token == json_token::JSON_ARRAY_END) {
        index++;  // 跳过 ]
    } else {
        L_ERR("Unclosed JSON array");
        throw std::runtime_error("Unclosed JSON array");
    }
    return arr;
}

json_value parse_json_data(const std::vector<state>& tokens, size_t& index) {
    if (index >= tokens.size()) {
        L_ERR("Unexpected end of JSON input");
        throw std::runtime_error("Unexpected end of JSON input");
    }

    switch (tokens[index].token) {
        case json_token::JSON_OBJECT_BEGIN:
            return parse_object(tokens, index);

        case json_token::JSON_ARRAY_BEGIN:
            return parse_array(tokens, index);

        case json_token::JSON_STRING: {
            json_value val(json_value::type::STRING);
            val.str_val = tokens[index].value;
            index++;
            return val;
        }

        case json_token::JSON_INTEGER: {
            json_value val(json_value::type::INTEGER);
            try {
                val.int_val = std::stoi(tokens[index].value);
            } catch (const std::exception& e) {
                L_ERR(std::string("Invalid integer value: ") + tokens[index].value + " error: " + e.what());
                throw;
            }
            index++;
            return val;
        }

        case json_token::JSON_FLOAT: {
            json_value val(json_value::type::FLOAT);
            try {
                val.float_val = std::stod(tokens[index].value);
            } catch (const std::exception& e) {
                L_ERR(std::string("Invalid float value: ") + tokens[index].value + " error: " + e.what());
                throw;
            }
            index++;
            return val;
        }

        case json_token::JSON_TRUE: {
            json_value val(json_value::type::BOOLEAN);
            val.bool_val = true;
            index++;
            return val;
        }

        case json_token::JSON_FALSE: {
            json_value val(json_value::type::BOOLEAN);
            val.bool_val = false;
            index++;
            return val;
        }

        case json_token::JSON_NULL: {
            return json_value(json_value::type::NULL_VALUE);
        }

        default:
            L_ERR("Unexpected JSON token: " + std::to_string(static_cast<int>(tokens[index].token)));
            throw std::runtime_error("Unexpected JSON token");
    }
}

Value json_value_to_value(const json_value& jv);

Value parse_json(const std::string &json_str) {
    try {
        std::vector<state> tokens = tokenize(json_str);
        size_t index = 0;

        if (tokens.empty()) {
            throw std::runtime_error("Empty JSON input");
        }

        if (tokens[index].token != json_token::JSON_OBJECT_BEGIN &&
            tokens[index].token != json_token::JSON_ARRAY_BEGIN) {
            throw std::runtime_error("JSON must start with object or array");
            }

        json_value root = parse_json_data(tokens, index);

        if (index != tokens.size()) {
            throw std::runtime_error("Unexpected tokens at end of JSON");
        }

        // 转换json_value为Lamina Value类型
        return json_value_to_value(root);

    } catch (const std::exception& e) {
        L_ERR(std::string("JSON parsing error: ") + e.what());
        throw;
    }
}

Value json_value_to_value(const json_value& jv) {
    switch (jv.t) {
        case json_value::type::ARRAY: {
            std::vector<Value> arr;
            for (const auto& elem : jv.array_val) {
                arr.push_back(json_value_to_value(elem));
            }
            return Value(arr);
        }
        case json_value::type::STRING:
            return Value(jv.str_val);
        case json_value::type::INTEGER:
            return Value(jv.int_val);
        case json_value::type::FLOAT:
            return Value(jv.float_val);
        case json_value::type::BOOLEAN:
            return Value(jv.bool_val);
        case json_value::type::NULL_VALUE:
            return Value(nullptr);
        case json_value::type::OBJECT: {
                std::vector<Value> flat_arr;

                for (const auto& [key, value_ptr] : jv.object_val) {
                    flat_arr.push_back(Value(key));
                    flat_arr.push_back(json_value_to_value(*value_ptr));
                }

                return LAMINA_ARR(flat_arr);
            }
        default:
            throw std::runtime_error("Unsupported JSON value type");
    }
}
