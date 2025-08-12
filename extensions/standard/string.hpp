#ifndef STRING_HPP
#define STRING_HPP
#include "lamina.hpp"

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

namespace Lamina {
    LAMINA_FUNC_WIT_ANY_ARGS("string_concat", concat);
    LAMINA_FUNC_MULTI_ARGS("string_char_at", char_at, 2);
    LAMINA_FUNC_MULTI_ARGS("string_length", length, 1);
    LAMINA_FUNC_MULTI_ARGS("string_find", find, 3);
    LAMINA_FUNC_MULTI_ARGS("string_sub_string", sub_string, 3);
    LAMINA_FUNC_MULTI_ARGS("string_replace_by_index", replace_by_index, 3);
}

#endif