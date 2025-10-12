#include "src_manger.hpp"
#include "color_style.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace {
    std::mutex cache_mutex;
}

inline std::vector<std::string> split_lines(const std::string& content) {
        std::vector<std::string> lines;
        std::stringstream ss(content);
        std::string line;

        while (std::getline(ss, line, '\n')) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);
        }

        return lines;
    }

// 行号从1开始
inline std::string get_slice(const std::string& src_path, const int& src_line_start, const int& src_line_end) {
    // 检查行号有效性
    if (src_line_start < 1 || src_line_end < src_line_start) {
        return "";
    }

    try {
        // 标准化路径
        fs::path normalized_path = fs::canonical(src_path);
        std::string key = normalized_path.string();

        // 线程安全地获取文件内容
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = src_manager::opened_files.find(key);
        if (it == src_manager::opened_files.end()) {
            return "";  // 文件未在缓存中
        }
        const std::string& content = it->second;

        // 分割内容为行
        std::vector<std::string> lines = split_lines(content);

        // 检查行号是否在有效范围内
        size_t start_idx = static_cast<size_t>(src_line_start - 1);
        size_t end_idx = static_cast<size_t>(src_line_end - 1);

        if (start_idx >= lines.size()) {
            return "";  // 起始行超出文件范围
        }

        // 确保结束行不超出文件范围
        if (end_idx >= lines.size()) {
            end_idx = lines.size() - 1;
        }

        // 拼接指定范围的行
        std::stringstream result;
        for (size_t i = start_idx; i <= end_idx; ++i) {
            if (i != start_idx) {
                result << "\n";  // 行间用换行符分隔
            }
            result << lines[i];
        }

        return result.str();
    } catch (const std::exception& e) {
        // 处理可能的文件系统异常
        return "";
    }
}

std::string get_file_at_path(const std::string& path) {
    const fs::path normalized_path = fs::canonical(path);
    const std::string key = normalized_path.string();

    std::lock_guard<std::mutex> lock(cache_mutex);

    // 查找缓存
    const auto it = src_manager::opened_files.find(key);
    if (it == src_manager::opened_files.end()) {
        std::cout << ConClr::RED << "file not in opened_files map: " << key << ConClr::RESET << std::endl;
    }

    return it->second;
}

std::string open_lm_file(const std::string& path) {
    // 处理路径：补全.lm后缀并标准化
    fs::path file_path = path;
    if (file_path.extension() != ".lm") {
        file_path += ".lm";
    }
    const fs::path normalized_path = fs::canonical(file_path);
    const std::string key = normalized_path.string();

    // 先检查缓存，避免重复读取
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = src_manager::opened_files.find(key);
        if (it != src_manager::opened_files.end()) {
            return it->second;
        }
    }

    // 缓存未命中，读取文件
    std::ifstream file(normalized_path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cout << ConClr::RED << "Can't to read it: " + key << ConClr::RESET << std::endl;
    }

    const size_t file_size = fs::file_size(normalized_path);
    std::string content;
    content.resize(file_size);

    file.read(&content[0], file_size);
    if (!file) {
        std::cout << ConClr::RED << "Fail to read it: " << key << ConClr::RESET << std::endl;
    }

    // 写入缓存
    std::lock_guard<std::mutex> lock(cache_mutex);
    src_manager::opened_files.emplace(key, std::move(content));  // 使用移动语义减少拷贝

    return src_manager::opened_files[key];
}
