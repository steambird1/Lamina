#include "standard.hpp"

std::unordered_map<std::string, Value> register_builtins() {
    return {
        // 字符串操作模块：封装字符串拼接、查找、截取等功能
        LAMINA_MODULE("string", "1.0.0", {
            LAMINA_FUNC("cat", cat),
            LAMINA_FUNC("at", char_at),
            LAMINA_FUNC("find", str_find),
            LAMINA_FUNC("sub", sub_string),
        }),

        // 数学计算模块：一级函数
        LAMINA_FUNC("sqrt", sqrt_),
        LAMINA_FUNC("pi", pi),
        LAMINA_FUNC("e", e),
        LAMINA_FUNC("abs", abs_),
        LAMINA_FUNC("sin", sin_),
        LAMINA_FUNC("cos", cos_),
        LAMINA_FUNC("tan", tan_),
        LAMINA_FUNC("log", log_),
        LAMINA_FUNC("round", round_),
        LAMINA_FUNC("floor", floor_),
        LAMINA_FUNC("ceil", ceil_),
        LAMINA_FUNC("dot", dot),
        LAMINA_FUNC("cross", cross),
        LAMINA_FUNC("norm", norm),
        LAMINA_FUNC("normalize", normalize),
        LAMINA_FUNC("det", det),
        LAMINA_FUNC("size", size),
        LAMINA_FUNC("idiv", idiv),
        LAMINA_FUNC("fraction", fraction),
        LAMINA_FUNC("decimal", decimal),
        LAMINA_FUNC("pow", pow_),
        LAMINA_FUNC("gcd", gcd),
        LAMINA_FUNC("lcm", lcm),
        LAMINA_FUNC("range", range),

        // 随机模块：封装随机浮点数、整数、字符串生成功能
        LAMINA_MODULE("random", "1.0.0", {
            LAMINA_FUNC("random", random_),
            LAMINA_FUNC("randint", randint),
            LAMINA_FUNC("randstr", randstr)
        }),

        // 时间模块：封装当前时间、日期、格式化日期获取功能
        LAMINA_MODULE("time", "1.0.0", {
            LAMINA_FUNC("get_time", get_time),
            LAMINA_FUNC("get_date", get_date),
            LAMINA_FUNC("get_format_date", get_format_date)
        }),

        // 数组处理模块：封装数组元素访问、修改、查找功能
        LAMINA_MODULE("array", "1.0.0", {
            LAMINA_FUNC("at", arr_at),
            LAMINA_FUNC("set", arr_set),
            LAMINA_FUNC("index_of", arr_index_of)
        }),

        // IO与系统模块：一级函数
        LAMINA_FUNC("input", input),
        LAMINA_FUNC("print", print),
        LAMINA_FUNC("system", system_),
        LAMINA_FUNC("assert", assert),

        // 操作模块：一级函数
        LAMINA_FUNC("typeof", typeof_),
        LAMINA_FUNC("getattr", getattr),
        LAMINA_FUNC("setattr", setattr),
        LAMINA_FUNC("update", update),
        LAMINA_FUNC("copy_struct", copy_struct),

        // 容器遍历模块：一级函数
        LAMINA_FUNC("foreach", foreach),
        LAMINA_FUNC("find", find),
        LAMINA_FUNC("map", map),
        LAMINA_FUNC("replace", replace),

        // CAS数学模块：封装符号计算相关的解析、化简、求导等功能
        LAMINA_MODULE("cas", "1.0.0", {
            LAMINA_FUNC("cas_parse", cas_parse),
            LAMINA_FUNC("cas_simplify", cas_simplify),
            LAMINA_FUNC("cas_differentiate", cas_differentiate),
            LAMINA_FUNC("cas_evaluate", cas_evaluate),
            LAMINA_FUNC("cas_store", cas_store),
            LAMINA_FUNC("cas_load", cas_load),
            LAMINA_FUNC("cas_evaluate_at", cas_evaluate_at),
            LAMINA_FUNC("cas_solve_linear", cas_solve_linear),
            LAMINA_FUNC("cas_numerical_derivative", cas_numerical_derivative)
        })
    };
}