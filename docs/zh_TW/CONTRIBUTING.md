## 程式碼提交規範

<div align="right">
  <strong>繁體中文</strong> | <a href="../zh_CN/CONTRIBUTING.md">简体中文</a> | <a href="../en_US/CONTRIBUTING.md">English</a>
</div>
<br>

- 請先閱讀 [README](README.md) 以瞭解專案的基本情況，再繼續進行開發！
- 本專案採用 GPL-3.0 協議，請務必遵守相關規定！

## 專案結構：
```angular2html
Lamina
├── assets
│   ├── logo-icon.svg
│   └── logo.svg
├── extensions
│   └── standard
│       ├── math.cpp
│       ├── random.cpp
│       ├── random.hpp
│       ├── sockets.cpp
│       ├── sockets.hpp
│       ├── stdio.cpp
│       ├── times.cpp
│       └── times.hpp
├── interpreter
│   ├── ast.hpp
│   ├── bigint.hpp
│   ├── examples
│   │   ├── calculator.lm
│   │   ├── defin.lm
│   │   ├── hello.lm
│   │   ├── onestop.lm
│   │   └── quadratic.lm
│   ├── interpreter.cpp
│   ├── interpreter.hpp
│   ├── interpreter.md
│   ├── irrational.hpp
│   ├── lamina.hpp
│   ├── lexer.cpp
│   ├── lexer.hpp
│   ├── main.cpp
│   ├── module.cpp
│   ├── module.hpp
│   ├── parser.cpp
│   ├── parser.hpp
│   ├── rational.hpp
│   ├── trackback.hpp
│   └── value.hpp
├── LICENSE
└── README.md
├── CMakeLists.txt
├── compile-cn.md
├── compile-en.md
├── CONTRIUTING-CN.md
```
總計： 5 資料夾, 38 檔案。

## 創建 Pull Request:

```angular2html
1. 將程式碼 Fork 到你的倉庫。
2. 將程式碼 Clone 到本地。
3. 將修改後的程式碼推送到你的倉庫。
4. 創建 Pull Request。
```

創建 PR 時，請務必確保你的 PR 內容符合以下要求：

- 目的明確
- 語言流暢
- 程式碼規範

在編寫 PR 標題時，必須包含如下標識，主要分為如下類型：
```angular2html
1. [Feature]
2. [Bug Fix]
```
如果是針對某個模組，請在 PR 標題中同時標註類型及模組名稱，例如：
```angular2html
[Feature][stdio] 新增 print 函式對文件流操作的支援
```

## 關於庫/標準庫:

標準庫程式碼存放於 `extensions/standard` 目錄下，每個檔案對應一個模組，模組名稱即檔案名稱，且每個模組都需有對應的標頭檔，在標頭檔內註冊 Lamina 函式。

在擴充層註冊的 Lamina 變數皆為全域變數。

### Lamina 函式註冊方式：

呼叫 LAMINA_FUNC 巨集，例如：
```c++
namespace lamina{
     LAMINA_FUNC("lamina_func_name", cpp_func_name, arg_counts);
}
```
事實上，也可以不將函式註冊於 lamina 命名空間下，直接註冊，例如：
```c++
LAMINA_FUNC("lamina_func_name", cpp_func_name, arg_counts);
```

不過這是一種更規範的做法，推薦採用！

若需宣告 Lamina 函式，C++ 層的回傳型態必須為 `Value`，並攜帶 `const std::vector<Value> &args` 參數。我們建議使用 lamina.hpp 中對資料型態操作的巨集，如 `LAMINA_BOOL`，這對 Lamina 庫專案開發更直觀！

但因部分歷史遺留問題，部分 Lamina 標準庫內容尚未採用這些巨集作為回傳值。

編寫標準庫程式碼時，請遵循以下規範：

- 程式碼必須確保安全性。
- 程式碼必須符合 Lamina 擴充風格。

## !! 為 Lamina 編寫其他庫時亦應遵循上述規範。

## 模組解析：

Lamina 主要核心模組有

- bigint.hpp 大整數模組
- interpreter.cpp 解譯器模組
- irrational.hpp 無理數模組
- lamina.hpp 存取 Lamina 部分核心資源模組
- module.cpp 載入 Lamina 動態庫模組
- rational.hpp 有理數模組
- value.hpp 數值模組
- parser.cpp 解析器模組
- lexer.cpp 詞法分析器模組

讓我們從零開始，逐步講解這些模組內的函式，幫助你進入 Lamina 庫開發！

編寫 Lamina 庫時，最重要的是 `lamina.hpp` 模組，此模組提供 Lamina 庫開發的基本巨集。
```c++
// 原始碼:
#pragma once
/*
    操作 LAMINA 核心資源的標頭檔
 */

template<class> constexpr bool always_false = false;


#define LAMINA_BOOL(value) Value((bool) value)
#define LAMINA_INT(value) Value((int) value)
#define LAMINA_DOUBLE(value) Value((double) value)
#define LAMINA_STRING(value) Value((const char*) value)
#define LAMINA_BIGINT(value) Value((const ::BigInt&)value)
#define LAMINA_RATIONAL(value) Value((const ::Rational&)value)
#define LAMINA_IRRATIONAL(value) Value((const ::Irrational&)value)
#define LAMINA_ARR(value) Value(value)
#define LAMINA_MATRIX(value) Value(value)
#define LAMINA_NULL Value()

#define LAMINA_FUNC_WIT_ANY_ARGS(func_name, func) \
void func##_any_args_entry(Interpreter& interpreter); \
namespace { \
struct func##_any_args_registrar { \
    func##_any_args_registrar() { \
        Interpreter::register_entry(&func##_any_args_entry); \
    } \
} func##_any_args_instance; \
} \
void func##_any_args_entry(Interpreter& interpreter) { \
    interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value { \
        return func(args); \
    }; \
}

#define LAMINA_FUNC(func_name, func, arg_count) \
void func##_entry(Interpreter& interpreter) LAMINA_EXPORT; \
namespace { \
struct func##_registrar { \
    func##_registrar() { \
        Interpreter::register_entry(&func##_entry); \
    } \
} func##_instance; \
} \
void func##_entry(Interpreter& interpreter) { \
    interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value { \
        if (args.size() != arg_count) { \
            std::cerr << "Error: " << func_name << "() requires " << arg_count <<" arguments\n"; \
            return Value(); \
        } \
        return func(args); \
    }; \
}

#define LAMINA_FUNC_MULTI_ARGS(func_name, func, arg_count) \
void func##_entry(Interpreter& interpreter); \
namespace { \
struct func##_registrar { \
    func##_registrar() { \
        Interpreter::register_entry(&func##_entry); \
    } \
} func##_instance; \
} \
void func##_entry(Interpreter& interpreter) { \
    interpreter.builtin_functions[func_name] = [](const std::vector<Value>& args) -> Value { \
        if (args.size() > arg_count) { \
            std::cerr << "Error: " << func_name << "() takes 0 to " << arg_count << " arguments\n"; \
            return Value(); \
        } \
        return func(args); \
    }; \
}

#define LAMINA_GET_VAR(interpreter, var) \
   interpreter.get_variable(#var)

#define L_ERR(msg)\
    error_and_exit(msg); \

#define LAMINA_GLOBAL_VAR(name, value) \
void global_var_##name##_entry(Interpreter& interpreter) { \
    interpreter.set_global_variable(#name, Value(value)); \
} \
namespace { \
struct global_var_##name##_registrar { \
    global_var_##name##_registrar() { \
        Interpreter::register_entry(&global_var_##name##_entry); \
    } \
} global_var_##name##_instance; \
}

```

- `LAMINA_FUNC_WIT_ANY_ARGS` 巨集用於註冊可接受任意參數數量的 Lamina 函式。
- `LAMINA_FUNC` 巨集用於註冊可接受固定參數數量的 Lamina 函式。
- `LAMINA_FUNC_MULTI_ARGS` 巨集用於註冊可接受 0 到固定參數數量的 Lamina 函式。

這些巨集的內部實作大同小異，只是參數數量判斷略有不同。

編譯成動態庫後，符號表會類似如下：
```
0000000000020edc T _Z10test_entryR11Interpreter
```

原始函式帶有 `_entry` 後綴，後續註冊到 `builtin_functions` 此 vector 容器。

`LAMINA_BOOL` 巨集用於操作 Lamina 布林型態資料
`LAMINA_INT` 巨集用於操作 Lamina 整數型態資料
`LAMINA_STRING` 巨集用於操作 Lamina 字串型態資料
`LAMINA_BIGINT` 巨集用於操作 Lamina 大整數型態資料
`LAMINA_RATIONAL` 巨集用於操作 Lamina 有理數型態資料
`LAMINA_IRRATIONAL` 巨集用於操作 Lamina 無理數型態資料
`LAMINA_ARR` 巨集用於操作 Lamina 陣列型態資料
`LAMINA_MATRIX` 巨集用於操作 Lamina 矩陣型態資料
`LAMINA_NULL` 巨集用於操作 Lamina 空值型態資料

可利用上述巨集直觀地操作 Lamina 函式回傳值，如隨機庫中：
```c++
Value randstr(const std::vector<Value> &args) {
     if (args.size() != 1 || !args[0].is_numeric()) {
          L_ERR("randstr() requires exactly one numeric argument");
          return LAMINA_NULL;
     }

     int length = std::stoi(args[0].to_string());
     if (length < 0) {
          L_ERR("randstr() length argument must be non-negative");
          return LAMINA_NULL;
     }

     static const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
     static std::random_device rd;
     static std::mt19937 gen(rd());
     std::uniform_int_distribution<> dis(0, charset.size() - 1);

     std::string result;
     result.reserve(length);
     for (int i = 0; i < length; ++i) {
          result += charset[dis(gen)];
     }

     return LAMINA_STRING(result.c_str());
}

```

此 Lamina 庫程式碼根據使用者輸入生成隨機字串，並利用 `LAMINA_STRING` 巨集回傳，失敗時則用 `LAMINA_NULL` 巨集回傳空值。

- `LAMINA_GET_VAR` 巨集用於執行時於已註冊函式中取得 Lamina 變數值。
- `LAMINA_GLOBAL_VAR` 巨集用於定義 Lamina 全域變數。

目前標準庫尚未使用這兩者，但仍提供範例以供開發參考：
```c++
#include "lamina.hpp"

LAMINA_GLOBAL_VAR(a, 1);
Interpreter interpreter;

Value func_a(const std::vector<Value> &args){
     Value a = LAMINA_GET_VAR(interpreter, a);
}

namespace lamina{
    LAMINA_FUNC(func_a, func_a, 0);
}
```
此範例展示了 Lamina 全域變數的註冊與使用，在取得變數時需傳入解譯器實例及變數名稱，註冊時則需指定名稱與值。

`L_ERR` 巨集用於在 Lamina 執行過程中丟出錯誤，例：
```c++
#include "lamina.hpp"

Value a(const std::vector<Value> &args){
     L_ERR("a is not defined");
     return LAMINA_NULL;
}
```

關於庫部分已說明完畢，接下來介紹 Lamina 更底層的解譯器模組！

Lamina 解譯器主要由以下模組構成，共同支援 Lamina 在數學運算上的優異表現：

- 大整數模組
- 無理數模組
- 有理數模組

此外，還有底層的解析器及詞法分析器模組，下文統稱語法處理模組。

先介紹 `interpreter.cpp` 原始檔案分析。

因篇幅關係僅展示函式原型與標頭檔內容。
```c++
#pragma once
#include "lamina.hpp"


#include "ast.hpp"
#include "value.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <set>
#include <functional>
#include <stack>

// Forward declaration for error handling
void error_and_exit(const std::string& msg);

// Stack frame for function call tracking
struct StackFrame {
    std::string function_name;
    std::string file_name;
    int line_number;
    
    StackFrame(const std::string& func, const std::string& file, int line)
        : function_name(func), file_name(file), line_number(line) {}
};

// Enhanced runtime error class with stack trace support
class RuntimeError : public std::exception {
public:
    std::string message;
    std::vector<StackFrame> stack_trace;
    
    RuntimeError(const std::string& msg) : message(msg) {}
    RuntimeError(const std::string& msg, const std::vector<StackFrame>& trace) 
        : message(msg), stack_trace(trace) {}
    
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Exception for return statements
class ReturnException : public std::exception {
public:
    Value value;
    explicit ReturnException(const Value& v) : value(v) {}
};


// Exception for break statements
class BreakException : public std::exception {
public:
    BreakException() = default;
};


// Exception for continue statements
class ContinueException : public std::exception {
public:
    ContinueException() = default;
};

class LAMINA_EXPORT Interpreter {
    // 禁止拷貝，允許移動
    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
    Interpreter(Interpreter&&) = default;
    Interpreter& operator=(Interpreter&&) = default;
public:
    Interpreter() {
        register_builtin_functions();
    }
    void execute(const std::unique_ptr<Statement>& node);
    Value eval(const ASTNode* node);
    // Print all variables in current scope
    void printVariables() const;
    void add_function(const std::string& name, FuncDefStmt* func);
    // Stack trace management
    void push_frame(const std::string& function_name, const std::string& file_name = "<script>", int line_number = 0);
    void pop_frame();
    std::vector<StackFrame> get_stack_trace() const;
    void print_stack_trace(const RuntimeError& error, bool use_colors = true) const;
    
    // Utility functions for error display
    static bool supports_colors();
    static void print_error(const std::string& message, bool use_colors = true);
    static void print_warning(const std::string& message, bool use_colors = true);
    // Builtin function type
    using BuiltinFunction = std::function<Value(const std::vector<Value>&)>;
    // Store builtin functions
    std::unordered_map<std::string, BuiltinFunction> builtin_functions;
    using EntryFunction = void(*)(Interpreter&);
    static void register_entry(EntryFunction func);
    // Variable assignment
    void set_variable(const std::string& name, const Value& val);
    // built global variable in interpreter
    void set_global_variable(const std::string& name, const Value& val);
    // Variable lookup
    Value get_variable(const std::string& name) const;
    // Variable scope stack, top is the current scope
    std::vector<std::unordered_map<std::string, Value>> variable_stack{ { } };
private:
    // Store function definitions
    std::unordered_map<std::string, FuncDefStmt*> functions;
    // List of loaded modules to prevent circular imports
    std::set<std::string> loaded_modules;
    // Store loaded module ASTs to keep function pointers valid
    std::vector<std::unique_ptr<ASTNode>> loaded_module_asts;

    // Stack trace for function calls
    std::vector<StackFrame> call_stack;
    // Recursion depth tracking
    int recursion_depth = 0;
    int max_recursion_depth = 100;  // 可變的遞迴深度限制
    // Enter/exit scope
    void push_scope();
    void pop_scope();
    // Load and execute module
    bool load_module(const std::string& module_name);
    // Register builtin functions
    void register_builtin_functions();
    static std::vector<EntryFunction> entry_functions;

};
```

`error_and_exit` 函式會打印錯誤資訊並結束程式，實作如下：
```c++
void error_and_exit(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
    exit(1);
}
```
呼叫時需傳入錯誤訊息字串，該字串會被打印並終止程式。

`StackFrame` 結構儲存函式呼叫追蹤資訊，有以下成員：
- `function_name`：函式名稱
- `file_name`：檔案名稱
- `line_number`：行號

這些資訊在程式運行錯誤時會打印，協助開發定位問題。

Lamina 有多種例外類別，例如：
- `RuntimeError` 繼承自 `std::exception`，表示執行時錯誤。
- `ReturnException` 表示 return 語句例外。
- `BreakException` 表示 break 例外。
- `ContinueException` 表示 continue 例外。

Interpreter 類定義多種方法，處理運算、控制台內容、擴充函式與變數。

公開方法與成員說明如下：

`Interpreter` 建構子會呼叫 `register_builtin_functions()` 註冊內建/擴充函式。

`execute` 用於執行 AST 樹，參數為 `Statement` 指標，無回傳值。

`eval` 用於計算運算式值，參數為 `ASTNode` 指標，回傳 `Value`。

`printVariables` 打印目前作用域所有變數。

`add_function` 新增函式，參數為名稱與 `FuncDefStmt` 指標。

`push_frame` 新增函式呼叫資訊，參數為函式名、檔案名、行號。

`pop_frame` 移除呼叫資訊。

`register_entry` 註冊入口函式，參數為 `EntryFunction` 指標。

`set_variable` 設定變數，參數為名稱與值。

`get_variable` 取得變數，參數為名稱。

`set_global_variable` 設定全域變數。

控制台操作：

`get_stack_trace` 取得堆疊資訊，回傳 `std::vector<StackFrame>`。

`print_stack_trace` 打印堆疊資訊，參數為 `RuntimeError` 與 `bool`。

`supports_colors` 判斷終端機是否支援色彩。

`print_error` 打印錯誤，參數為字串與 `bool`。

`print_warning` 打印警告，參數為字串與 `bool`。

成員變數：

- `builtin_functions` 儲存所有內建/擴充函式，鍵為名稱，值為函式指標。
- `functions` 儲存函式定義。
- `variable_stack` 儲存所有變數與作用域。
- `loaded_module_asts` 儲存所有已載入模組的 AST。

接下來介紹 Lamina 數字處理部分。

首先介紹大整數模組:
```c++
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <climits>

class BigInt {
private:
    std::vector<int> digits;  // 數字，低位在前
    bool negative;

public:
    // 建構子
    BigInt() : negative(false) { digits.push_back(0); }
    
    BigInt(int n) : negative(n < 0) {
        if (n == 0) {
            digits.push_back(0);
            return;
        }
        
        // 處理 INT_MIN
        if (n == INT_MIN) {
            long long ln = static_cast<long long>(n);
            ln = -ln;
            while (ln > 0) {
                digits.push_back(ln % 10);
                ln /= 10;
            }
        } else {
            n = std::abs(n);
            while (n > 0) {
                digits.push_back(n % 10);
                n /= 10;
            }
        }
    }
    
    BigInt(const std::string& str) : negative(false) {
        if (str.empty() || str == "0") {
            digits.push_back(0);
            return;
        }
        
        int start = 0;
        if (str[0] == '-') {
            negative = true;
            start = 1;
        } else if (str[0] == '+') {
            start = 1;
        }
        
        for (int i = str.length() - 1; i >= start; i--) {
            if (str[i] >= '0' && str[i] <= '9') {
                digits.push_back(str[i] - '0');
            }
        }
        
        if (digits.empty()) {
            digits.push_back(0);
        }
        
        remove_leading_zeros();
    }
    
    // 移除前導零
    void remove_leading_zeros() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.size() == 1 && digits[0] == 0) {
            negative = false;
        }
    }
    
    // 轉字串
    std::string to_string() const {
        if (digits.size() == 1 && digits[0] == 0) {
            return "0";
        }
        
        std::string result;
        if (negative) result += "-";
        
        for (int i = digits.size() - 1; i >= 0; i--) {
            result += char('0' + digits[i]);
        }
        
        return result;
    }
    
    // 乘法
    BigInt operator*(const BigInt& other) const {
        BigInt result;
        result.digits.assign(digits.size() + other.digits.size(), 0);
        result.negative = (negative != other.negative);
        
        for (size_t i = 0; i < digits.size(); i++) {
            for (size_t j = 0; j < other.digits.size(); j++) {
                result.digits[i + j] += digits[i] * other.digits[j];
                if (result.digits[i + j] >= 10) {
                    result.digits[i + j + 1] += result.digits[i + j] / 10;
                    result.digits[i + j] %= 10;
                }
            }
        }
        
        result.remove_leading_zeros();
        return result;
    }
    
    // 轉 int（如可行）
    int to_int() const {
        if (digits.size() > 10) {
            return negative ? INT_MIN : INT_MAX;
        }
        
        long long result = 0;
        long long multiplier = 1;
        
        for (int digit : digits) {
            result += digit * multiplier;
            multiplier *= 10;
            if (result > INT_MAX) {
                return negative ? INT_MIN : INT_MAX;
            }
        }
        
        return negative ? -static_cast<int>(result) : static_cast<int>(result);
    }
    
    // 是否為零
    bool is_zero() const {
        return digits.size() == 1 && digits[0] == 0;
    }
    
    // 比較運算子
    bool operator==(const BigInt& other) const {
        return negative == other.negative && digits == other.digits;
    }
    
    bool operator!=(const BigInt& other) const {
        return !(*this == other);
    }
};
```

`BigInt` 提供更安全的整數處理方式，避免溢位。

內部定義兩個成員：
- `std::vector<int> digits` 存儲大整數每一位（低位在前）。
- `bool negative` 表示正負，`true` 為負。

建構子支援多種型態，包含 `int`、`long long`、`std::string` 等。

`BigInt` 提供加、減、乘、除與比較等運算子重載。

接下來介紹無理數/有理數模組

因篇幅暫不展示。

無理數模組支援精確表示 π、e 等常見無理數。

定義如下 enum 類型：
```c++
   enum class Type {
        SQRT,      // √n 形式
        PI,        // π 的倍數
        E,         // e 的倍數  
        LOG,       // log(n) 形式
        COMPLEX    // 複合形式 (a*√b + c*π + d*e + ...)
    };
```
私有成員如下，用於 √n 形式：
```c++
    // 對 √n：coefficient * √radicand
    double coefficient;  // 系數
    long long radicand;  // 根號內數值
```

複合形式則採系數映射方式：
```c++
    std::map<std::string, double> coefficients;
    double constant_term;  // 常數項
```

有理數模組支持 a/b 形式，可化簡分數。

在化簡過程中 rational 模組已檢查分母非零，無需額外考量安全性。

接下來介紹語法分析模組。

解析器模組定義多種運算式型態，AST 模組定義運算式節點。

詞法分析模組將輸入字串轉為語法單元（token）序列。

`Token` 結構包含型態、值與位置信息。

詞法分析功能為將字串轉為 token 序列，並處理註解、空白等。

採用簡易狀態機完成詞法分析。

最後介紹庫載入模組。

目前庫載入僅支援 Linux 平台，包含：

1. 動態連結庫載入
2. 符號查找與綁定
3. 函式呼叫

庫載入模組使用 `dlopen` 載入動態庫，`dlsym` 查找符號，`dlclose` 關閉動態庫。

`findSymbol` 函式接受符號名稱並回傳位址。
