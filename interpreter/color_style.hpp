#pragma once
#include <string>
#include <mutex>
#include "interpreter.hpp"

// CC => Console Color
class CC {
public:
    // 禁止拷贝构造和赋值
    CC(const CC&) = delete;
    CC& operator=(const CC&) = delete;

    static CC& get_instance() {
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance_.init(); // 初始化颜色常量
        });
        return instance_;
    }

    std::string RESET;
    std::string BLACK;
    std::string RED;
    std::string GREEN;
    std::string YELLOW;
    std::string BLUE;
    std::string MAGENTA;
    std::string CYAN;
    std::string WHITE;
    std::string LIGHT_BLACK;
    std::string LIGHT_RED;
    std::string LIGHT_GREEN;
    std::string LIGHT_YELLOW;
    std::string LIGHT_BLUE;
    std::string LIGHT_MAGENTA;
    std::string LIGHT_CYAN;
    std::string LIGHT_WHITE;

private:
    CC() = default;
    // 单例实例
    static CC instance_;

    // 初始化函数：根据颜色支持状态设置常量值
    void init() {
        const bool has_color = Interpreter::supports_colors();

        RESET = has_color ? "\033[0m" : "";
        BLACK = has_color ? "\033[30m" : "";
        RED = has_color ? "\033[31m" : "";
        GREEN = has_color ? "\033[32m" : "";
        YELLOW = has_color ? "\033[33m" : "";
        BLUE = has_color ? "\033[34m" : "";
        MAGENTA = has_color ? "\033[35m" : "";
        CYAN = has_color ? "\033[36m" : "";
        WHITE = has_color ? "\033[37m" : "";
        LIGHT_BLACK = has_color ? "\033[90m" : "";
        LIGHT_RED = has_color ? "\033[91m" : "";
        LIGHT_GREEN = has_color ? "\033[92m" : "";
        LIGHT_YELLOW = has_color ? "\033[93m" : "";
        LIGHT_BLUE = has_color ? "\033[94m" : "";
        LIGHT_MAGENTA = has_color ? "\033[95m" : "";
        LIGHT_CYAN = has_color ? "\033[96m" : "";
        LIGHT_WHITE = has_color ? "\033[97m" : "";
    }
};

CC CC::instance_;

// 通过如Fore::get().RED使用
namespace Fore {
    inline CC& get() {
        return CC::get_instance();
    }
}

