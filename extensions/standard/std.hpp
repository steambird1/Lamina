#pragma once
#include "lamina_api/lamina.hpp"
#include "lamina_api/value.hpp"

/**
 * 拼接多个字符串，并返回一个新字符串
 */
Value concat(const std::vector<Value>& args);

/**
 * 获取字符串指定位置的字符，以Int类型返回
 */
Value char_at(const std::vector<Value>& args);

/**
 * 获取字符串长度
 */
Value length(const std::vector<Value>& args);

/**
 * 从指定位置开始查找子字符串，
 * 若查找成功，返回第一个符合结果的子字符串索引，
 * 否则返回-1。
 * 参数1：str（字符串）
 * 参数2：start_index（开始位置索引）
 * 参数3：sub_str（子字符串）
 */
Value find(const std::vector<Value>& args);

/**
 * 截取子字符串
 * 参数1：str（字符串）
 * 参数2：start_index（子字符串起始位置）
 * 参数3：len（子字符串长度）
 */
Value sub_string(const std::vector<Value>& args);

/**
 * 从指定位置开始替换原字符串，并返回一个新字符串
 * 参数1：str（原字符串）
 * 参数2：start_index（开始位置）
 * 参数3：sub_str（子字符串）
 */
Value replace_by_index(const std::vector<Value>& args);

inline std::unordered_map<std::string, Value> register_builtins() {
    return {
        LAMINA_BUILTINS_MODULE("string", "1.0.0", {
            {"concat", Value(std::make_shared<LmCppFunction>(std::move(concat)))}
        })
    };
}

// LAMINA_BUILTINS_FUNC("char_at", char_at),
// LAMINA_BUILTINS_FUNC("length", length),
// LAMINA_BUILTINS_FUNC("find", find),
// LAMINA_BUILTINS_FUNC("sub_string", sub_string),
// LAMINA_BUILTINS_FUNC("replace_by_index", replace_by_index),
