#include "lamina_api/lamina.hpp"
#include "standard.hpp"

/**
 * 拼接多个字符串，并返回一个新字符串
 */
Value cat(const std::vector<Value>& args) {
    std::string str;
    for (const auto& arg : args) {
        if (!arg.is_string()) {
            L_ERR("Args Must Be String");
            return LAMINA_NULL;
        }
        const std::string arg_str = std::get<std::string>(arg.data);
        str += arg_str;
    }

    return Value(str);
}

/**
 * 获取字符串指定位置的字符，以Int类型返回
 */
Value char_at(const std::vector<Value>& args) {
    if (!args[0].is_string()) {
        L_ERR("First Arg Must Be A String");
        return LAMINA_NULL;
    }
    if (!args[1].is_int()) {
        L_ERR("Second Arg Must Be A Int");
        return LAMINA_NULL;
    }

    const std::string str = std::get<std::string>(args[0].data);
    const int index = std::get<int>(args[1].data);

    if (index < 0 || static_cast<size_t>(index) >= str.length()) {
        L_ERR("Char Index Out Of Range");
    }

    return Value(str.at(static_cast<size_t>(index)));
}

/**
 * 获取字符串长度
 */
Value length(const std::vector<Value>& args) {
    if (!args[0].is_string()) {
        L_ERR("First Arg Must Be A String");
        return LAMINA_NULL;
    }

    const std::string str = std::get<std::string>(args[0].data);

    return Value(static_cast<int>(str.length()));
}

/**
 * 从指定位置开始查找子字符串，
 * 若查找成功，返回第一个符合结果的子字符串索引，
 * 否则返回-1。
 * 参数1：str（字符串）
 * 参数2：start_index（开始位置索引）
 * 参数3：sub_str（子字符串）
 */
Value str_find(const std::vector<Value>& args) {
    if (!args[0].is_string()) {
        L_ERR("First Arg Must Be A String");
        return LAMINA_NULL;
    }
    if (!args[1].is_int()) {
        L_ERR("Second Arg Must Be A Int");
        return LAMINA_NULL;
    }
    if (!args[2].is_string()) {
        L_ERR("Third Arg Must Be A String");
        return LAMINA_NULL;
    }

    const std::string str = std::get<std::string>(args[0].data);
    const int start_index = std::get<int>(args[1].data);
    const std::string sub_str = std::get<std::string>(args[2].data);

    if (start_index < 0 || static_cast<size_t>(start_index) >= str.length()) {
        L_ERR("Start Index Out Of Range");
    }

    size_t index = str.find(sub_str, static_cast<size_t>(start_index));

    if (index != str.npos) {
        return Value(static_cast<int>(index));
    }
    return Value(-1);
}

/**
 * 截取子字符串
 * 参数1：str（字符串）
 * 参数2：start_index（子字符串起始位置）
 * 参数3：len（子字符串长度）
 */
Value sub_string(const std::vector<Value>& args) {
    if (!args[0].is_string()) {
        L_ERR("First Arg Must Be A String");
        return LAMINA_NULL;
    }
    if (!args[1].is_int()) {
        L_ERR("Second Arg Must Be A Int");
        return LAMINA_NULL;
    }
    if (!args[2].is_int()) {
        L_ERR("Third Arg Must Be A Int");
        return LAMINA_NULL;
    }

    const std::string str = std::get<std::string>(args[0].data);
    const int start_index = std::get<int>(args[1].data);
    const int len = std::get<int>(args[2].data);

    if (start_index < 0 || static_cast<size_t>(start_index) >= str.length()) {
        L_ERR("Start Index Out Of Range");
    }

    const std::string sub_str = str.substr(start_index, len);

    return Value(sub_str);
}

Value to_string(const std::vector<Value>& args) {
    check_cpp_function_argv(args,1);
    return Value(  args[0].to_string()  );
}