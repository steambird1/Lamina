#pragma once
#include <algorithm>
#include <cctype>// 鐢ㄤ簬 isspace
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

// 解析 Properties 文件
inline std::unordered_map<std::string, std::string> parse_properties(const std::string& file_path) {
    std::unordered_map<std::string, std::string> prop_map;
    std::ifstream file(file_path); // 打开文件

    // 检查文件是否成功打开
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open properties file: " + file_path);
    }

    std::string line;
    // 逐行读取文件
    while (std::getline(file, line)) {
        // 修剪行首尾的空白字符（空格、制表符等）
        auto trim = [](std::string& s) {
            // 去掉开头空白
            s.erase(s.begin(), std::ranges::find_if(s, [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            // 去掉结尾空白
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        };
        trim(line);

        // 跳过空行和注释行（# 或 ! 开头为注释）
        if (line.empty() || line[0] == '#' || line[0] == '!') {
            continue;
        }

        // 拆分 key 和 value（找第一个 = 或 : 作为分隔符）
        size_t sep_pos = line.find_first_of("=:");
        if (sep_pos == std::string::npos) {
            // 无分隔符，视为无效行（或按需求处理，如 key=空value）
            continue;
        }

        // 提取 key 并修剪空白
        std::string key = line.substr(0, sep_pos);
        trim(key);

        // 提取 value 并修剪空白（跳过分隔符后的空白）
        std::string value = line.substr(sep_pos + 1);
        trim(value);

        // 处理 value 中的转义字符（可选，如 \n 转成换行符，\= 转成 =）
        auto unescape = [](const std::string& s) {
            std::string res;
            for (size_t i = 0; i < s.size(); ++i) {
                if (s[i] == '\\' && i + 1 < s.size()) {
                    switch (s[i + 1]) {
                        case 'n': res += '\n'; break;
                        case 'r': res += '\r'; break;
                        case 't': res += '\t'; break;
                        case '=': res += '='; break;
                        case ':': res += ':'; break;
                        case '\\': res += '\\'; break;
                        default: res += s[i]; res += s[i + 1]; // 未知转义，保留原字符
                    }
                    ++i; // 跳过转义符后的字符
                } else {
                    res += s[i];
                }
            }
            return res;
        };
        value = unescape(value);

        // 存入 unordered_map
        prop_map.emplace(std::move(key), std::move(value));
    }

    file.close();
    return prop_map;
}
