#include "repl_input.hpp"

#include "color_style.hpp"

#include <algorithm>
#include <iostream>
#include <set>
#include <signal.h>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#ifdef _WIN32
void CtrlCExit() {
    std::cout << "^C" << std::endl;
    throw CtrlCException();
}
#else
struct termios oldt;
volatile sig_atomic_t g_running = 1;    // 按下Ctrl+C后变为0
// void CtrlCExit() {
//     tcsetattr(STDIN_FILENO, TCSANOW, &oldt);// 恢复终端设置
//     std::cout << "^C" << std::endl;
//     throw CtrlCException();
// }
bool input_available() {    // 防止getchar阻塞
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval timeout = {0, 0};
    return (select(1, &fds, nullptr, nullptr, &timeout) > 0);
}
#endif

int console_getchar() {
#ifdef _WIN32
    // Windows 平台：直接调用 _getch（避免与标准库 getch 冲突）
    return _getch();
#else
    // Linux/macOS 平台：通过 termios 临时关闭终端回显和缓冲
    struct termios old_attr, new_attr;
    int ch;

    // 获取当前终端属性并保存（用于后续恢复）
    if (tcgetattr(STDIN_FILENO, &old_attr) != 0) {
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }
    new_attr = old_attr;  // 复制属性作为修改基础

    // 修改属性：关闭回显（ICANON）和缓冲（ECHO）
    new_attr.c_lflag &= ~(ICANON | ECHO);
    new_attr.c_cc[VMIN] = 1;  // 至少读取 1 个字符才返回
    new_attr.c_cc[VTIME] = 0; // 不设置超时，立即返回

    // 应用新的终端属性
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_attr) != 0) {
        perror("tcsetattr failed");
        exit(EXIT_FAILURE);
    }

    // 读取字符（此时无回显、无缓冲）
    ch = getchar();

    // 恢复终端原始属性（关键：避免影响后续终端行为）
    tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);

    return ch;
#endif
}

// 关键字表（按需求定义）
const std::set<std::string> KEYWORDS = {
    "if", "var", "else", "struct", "func", "print", "define", "while"
};

// 分隔符
const std::string SEPARATORS = " \t\n\r()[]{};,.+-*/=<>!&|";

/**
 * @brief 判断字节是否为UTF-8多字节字符的首字节
 */
bool is_utf8_first_byte(unsigned char c) {
    return (c <= 0x7F) || (c >= 0xC0 && c <= 0xF7);
}

/**
 * @brief 获取UTF-8字符的字节长度（1-4字节）
 */
size_t utf8_char_len(unsigned char c) {
    if (c <= 0x7F) return 1;
    if (c <= 0xDF) return 2;
    if (c <= 0xEF) return 3;
    if (c <= 0xF7) return 4;
    return 1; // 非法字节按1字节处理
}

/**
 * @brief 从输入缓冲区和char_indices，获取光标位置对应的“字符索引”（非字节索引）
 */
size_t get_char_index(const std::vector<size_t>& char_indices, size_t cursor_pos) {
    const auto it = std::upper_bound(char_indices.begin(), char_indices.end(), cursor_pos);
    return std::distance(char_indices.begin(), it) - 1;
}

/**
 * @brief 提取光标前的“当前单词”（从光标向前找分隔符，确定单词范围）
 */
std::pair<size_t, std::string> get_current_word(const std::string& buf, size_t cursor_pos) {
    // 向前找单词起始位置
    size_t word_start = cursor_pos;
    while (word_start > 0 && SEPARATORS.find(buf[word_start - 1]) == std::string::npos) {
        word_start--;
    }
    // 提取单词内容
    std::string word = buf.substr(word_start, cursor_pos - word_start);
    return {word_start, word};
}

/**
 * @brief 搜索关键字表，找到前缀匹配的关键字（仅返回唯一匹配，便于补全）
 */
std::string find_completion(const std::string& prefix) {
    if (prefix.size() < 3) return ""; // 输入不足3字符不补全
    std::vector<std::string> matches;
    for (const auto& kw : KEYWORDS) {
        if (kw.substr(0, prefix.size()) == prefix) {
            matches.push_back(kw);
        }
    }
    return (matches.size() == 1) ? matches[0] : ""; // 仅唯一匹配时返回
}

/**
 * @brief 渲染输入行（含prompt、高亮、补全候选），并移动光标到正确位置
 */
void render_line(const std::string& prompt, const std::string& buf, size_t cursor_pos, const std::string& completion) {
    // 清除当前行并移动到行首（ANSI控制码：\033[2K清除行，\r回车到行首）
    std::cout << "\033[2K\r" << prompt;

    // 解析缓冲区，按“普通文本/字符串/关键字”应用高亮
    bool in_string = false; // 是否处于双引号内（字符串高亮）
    size_t buf_len = buf.size();
    size_t current_pos = 0;

    while (current_pos < buf_len) {
        // 处理字符串（双引号包裹）
        if (buf[current_pos] == '"') {
            std::cout << ConClr::GREEN << buf[current_pos] << ConClr::RESET;
            in_string = !in_string;
            current_pos++;
            continue;
        }

        if (in_string) {
            // 字符串内部绿色高亮
            std::cout << ConClr::GREEN << buf[current_pos] << ConClr::RESET;
            current_pos++;
            continue;
        }

        // 先提取当前单词
        if (SEPARATORS.find(buf[current_pos]) == std::string::npos) {
            size_t word_end = current_pos;
            while (word_end < buf_len && SEPARATORS.find(buf[word_end]) == std::string::npos) {
                word_end++;
            }
            std::string word = buf.substr(current_pos, word_end - current_pos);
            // 判断是否为关键字：是则紫色，否则默认色
            if (KEYWORDS.contains(word)) {
                std::cout << ConClr::MAGENTA << word << ConClr::RESET;
            } else {
                std::cout << ConClr::RESET << word << ConClr::RESET;
            }
            current_pos = word_end;
            continue;
        }

        // 普通分隔符：默认色
        std::cout << ConClr::RESET << buf[current_pos] << ConClr::RESET;
        current_pos++;
    }

    // 显示补全候选
    if (!completion.empty()) {
        std::cout << ConClr::LIGHT_BLACK << completion << ConClr::RESET;
    }

    // 移动光标
    size_t cursor_offset = prompt.size() + cursor_pos;
    if (!completion.empty()) {
        cursor_offset -= completion.size(); // 补全部分不占光标位置
    }
    std::cout << "\033[" << cursor_offset << "G"; // ANSI：移动光标到第N列

    std::cout.flush(); // 强制刷新输出
}

// 核心函数
std::string repl_readline(const std::string& prompt, const std::string& placeholder) {
    std::cout << prompt;
    std::string buf = placeholder;       // 输入缓冲区
    size_t cursor_pos;                      // 光标初始位置
    std::vector<size_t> char_indices;       // 记录每个UTF-8字符的起始字节位置
    std::string current_completion;         // 当前补全候选（如输入whi时，completion是"le"）

    if (!buf.empty()) {
        cursor_pos = buf.size();
    }
    cursor_pos = 0;

    size_t byte_pos = 0;
    char_indices.clear();
    byte_pos = 0;
    while (byte_pos < buf.size()) {
        char_indices.push_back(byte_pos);
        const auto c = static_cast<unsigned char>(buf[byte_pos]);
        byte_pos += utf8_char_len(c);
    }
    // 确保至少有一个元素（处理空缓冲区）
    if (char_indices.empty()) {
        char_indices.push_back(0);
    }
    char_indices.push_back(byte_pos);

    while (true) {
        const int ch = console_getchar(); // 获取不回显字符

        // 控制键处理
        // 回车键
        if (ch == '\r' || ch == '\n' || ch == 0x0D) {
            std::cout << ConClr::RESET << "\r\n" << ConClr::RESET;
            std::cout.flush();
            return buf;
        }

        // 退格键
        if (ch == 8 || ch == 127 || ch == 0x7F) { // 8=Backspace，127=Del（兼容不同终端）
            if (cursor_pos == 0) continue; // 光标在开头，无法删除
            // 找到光标前字符的起始字节（按UTF-8字符删除）
            const size_t char_idx = get_char_index(char_indices, cursor_pos);
            const size_t char_start = char_indices[char_idx];
            // 删除字符（从char_start到cursor_pos）
            buf.erase(char_start, cursor_pos - char_start);
            cursor_pos = char_start;
            // 更新char_indices（重新计算所有字符的起始位置）
            char_indices.clear();
            byte_pos = 0;
            while (byte_pos < buf.size()) {
                char_indices.push_back(byte_pos);
                const auto c = static_cast<unsigned char>(buf[byte_pos]);
                byte_pos += utf8_char_len(c);
            }
            char_indices.push_back(byte_pos);
            // 重新计算补全
            auto [word_start, word] = get_current_word(buf, cursor_pos);
            current_completion = find_completion(word);
            if (!current_completion.empty()) {
                current_completion = current_completion.substr(word.size()); // 补全部分（如whi→while，补全"le"）
            }
            render_line(prompt, buf, cursor_pos, current_completion);
            continue;
        }

        // 方向键（左：0xE0 0x4B，右：0xE0 0x4D，上：0xE0 0x48，下：0xE0 0x50）—— 兼容大部分终端的方向键编码
        if (ch == 0xE0 || ch == 0x1B) { // 方向键前缀（增加0x1B作为前缀选项）
            int arrow_ch = console_getchar();
            // 处理ESC序列的情况（如某些终端方向键是\033[A格式）
            if (arrow_ch == '[') {
                arrow_ch = console_getchar();
            }

            if (arrow_ch == 0x4B || arrow_ch == 'D') { // 左方向键
                if (cursor_pos > 0) {
                    size_t char_idx = get_char_index(char_indices, cursor_pos);
                    cursor_pos = char_indices[char_idx - 1]; // 移动到前一个字符的起始字节
                }
            } else if (arrow_ch == 0x4D || arrow_ch == 'C') { // 右方向键
                if (cursor_pos < buf.size()) {
                    size_t char_idx = get_char_index(char_indices, cursor_pos);
                    cursor_pos = char_indices[char_idx + 1]; // 移动到后一个字符的起始字节
                }
            }
            // 重新计算补全（光标移动可能改变当前单词）
            auto [word_start, word] = get_current_word(buf, cursor_pos);
            current_completion = find_completion(word);
            if (!current_completion.empty()) {
                current_completion = current_completion.substr(word.size());
            }
            render_line(prompt, buf, cursor_pos, current_completion);
            continue;
        }

        // Tab键（触发补全）
        if (ch == '\t') {
            if (!current_completion.empty()) {
                // 补全：将current_completion追加到缓冲区
                buf.insert(cursor_pos, current_completion);
                cursor_pos += current_completion.size();
                // 更新char_indices
                char_indices.clear();
                byte_pos = 0;
                while (byte_pos < buf.size()) {
                    char_indices.push_back(byte_pos);
                    const auto c = static_cast<unsigned char>(buf[byte_pos]);
                    byte_pos += utf8_char_len(c);
                }
                char_indices.push_back(byte_pos);
                current_completion.clear(); // 补全后清空候选
            }
            render_line(prompt, buf, cursor_pos, current_completion);
            continue;
        }

        // 可打印字符处理
        // 过滤不可见控制符
        if (ch < 32 && ch != '\t' && ch != '\r' && ch != '\n') continue;

        // 读取UTF-8多字节字符
        std::string utf8_char;
        utf8_char += static_cast<char>(ch);
        const auto first_byte = static_cast<unsigned char>(ch);
        const size_t char_byte_len = utf8_char_len(first_byte);
        // 读取剩余字节
        for (size_t i = 1; i < char_byte_len; i++) {
            const int next_ch = console_getchar();
            if (next_ch == EOF || !is_utf8_first_byte(static_cast<unsigned char>(next_ch))) {
                // 放弃该字符
                utf8_char.clear();
                break;
            }
            utf8_char += static_cast<char>(next_ch);
        }
        if (utf8_char.empty()) continue;

        // 将UTF-8字符插入缓冲区
        buf.insert(cursor_pos, utf8_char);
        cursor_pos += utf8_char.size();
        // 更新char_indices
        char_indices.clear();
        byte_pos = 0;
        while (byte_pos < buf.size()) {
            char_indices.push_back(byte_pos);
            const auto c = static_cast<unsigned char>(buf[byte_pos]);
            byte_pos += utf8_char_len(c);
        }
        char_indices.push_back(byte_pos);

        // 计算补全候选
        auto [word_start, word] = get_current_word(buf, cursor_pos);
        current_completion = find_completion(word);
        if (!current_completion.empty()) {
            current_completion = current_completion.substr(word.size());
        }

        // 重新渲染
        render_line(prompt, buf, cursor_pos, current_completion);
    }
}