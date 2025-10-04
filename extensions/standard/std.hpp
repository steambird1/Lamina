#pragma once
#include "lamina_api/lamina.hpp"
#include "lamina_api/value.hpp"

// 拼接多个字符串，并返回一个新字符串
Value concat(const std::vector<Value>& args);

//获取字符串指定位置的字符，以Int类型返回
Value char_at(const std::vector<Value>& args);

//获取字符串长度
Value length(const std::vector<Value>& args);

// 从指定位置开始查找子字符串，若查找成功，返回第一个符合结果的子字符串索引，
Value find(const std::vector<Value>& args);

// 截取子字符串
Value sub_string(const std::vector<Value>& args);

// 从指定位置开始替换原字符串，并返回一个新字符串
Value replace_by_index(const std::vector<Value>& args);

// 平方根：需1个数值参数（如数字、向量/矩阵的每个元素）
Value sqrt(const std::vector<Value>& args);

// 圆周率π：需0个参数，直接返回π的数值
Value pi(const std::vector<Value>& args);

// 自然常数e：需0个参数，直接返回e的数值
Value e(const std::vector<Value>& args);

// 绝对值：需1个数值参数（处理数字、向量/矩阵元素的正负）
Value abs(const std::vector<Value>& args);

// 正弦函数：需1个参数（弧度值，可处理单个数字或向量/矩阵元素）
Value sin(const std::vector<Value>& args);

// 余弦函数：需1个参数（弧度值，可处理单个数字或向量/矩阵元素）
Value cos(const std::vector<Value>& args);

// 正切函数：需1个参数（弧度值，可处理单个数字或向量/矩阵元素）
Value tan(const std::vector<Value>& args);

// 自然对数（ln）：需1个正数值参数（处理数字、向量/矩阵元素）
Value log(const std::vector<Value>& args);

// 四舍五入：需1个数值参数，返回最接近的整数
Value round(const std::vector<Value>& args);

// 向下取整（地板函数）：需1个数值参数，返回不大于该值的最大整数
Value floor(const std::vector<Value>& args);

// 向上取整（天花板函数）：需1个数值参数，返回不小于该值的最小整数
Value ceil(const std::vector<Value>& args);

// 点积：需2个同维度向量参数，返回标量结果
Value dot(const std::vector<Value>& args);

// 叉积：需2个3维向量参数，返回3维向量结果
Value cross(const std::vector<Value>& args);

// 范数（默认L2范数/欧几里得范数）：需1个向量/矩阵参数，返回标量
Value norm(const std::vector<Value>& args);

// 归一化：需1个向量参数，返回单位向量（各元素除以范数）
Value normalize(const std::vector<Value>& args);

// 行列式：需1个方阵参数（如2x2、3x3矩阵），返回标量结果
Value det(const std::vector<Value>& args);

// 大小/维度：需1个容器类参数（向量/矩阵），返回元素个数或维度信息（如{行,列}）
Value size(const std::vector<Value>& args);

// 整数除法：需2个整数参数（被除数、除数），返回商的整数部分（截断小数）
Value idiv(const std::vector<Value>& args);

// 分数部分：需1个数值参数，返回该数的小数部分（如5.7返回0.7，-3.2返回-0.2）
Value fraction(const std::vector<Value>& args);

// 小数转整数（或提取整数部分）：需1个数值参数，返回整数部分（如5.7返回5，-3.2返回-3）
Value decimal(const std::vector<Value>& args);

// 幂运算（base^exponent）：需2个参数（底数、指数），返回幂运算结果
Value pow(const std::vector<Value>& args);

// 最大公约数：需2个正整数参数，返回二者的最大公约数
Value gcd(const std::vector<Value>& args);

// 最小公倍数：需2个正整数参数，返回二者的最小公倍数
Value lcm(const std::vector<Value>& args);

inline std::unordered_map<std::string, Value> register_builtins() {
    return {
    };
}