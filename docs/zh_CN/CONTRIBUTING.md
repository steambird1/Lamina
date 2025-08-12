## 代码提交规范

<div align="right">
  <a href="../zh_TW/CONTRIBUTING.md">🇹🇼 繁體中文</a> | <strong>🇨🇳 简体中文</strong> | <a href="../en_US/CONTRIBUTING.md">🇺🇸 English</a>
</div>
<br>

- 请先阅读 [README](/README.md) 了解项目的基本情况，在继续进行开发！
- 本项目使用GPL-3.0协议，请严格遵守本协议！

## 项目结构：
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
总计： 5 文件夹, 38 文件。

## 创建拉取请求:

```angular2html
1. Fork代码到你的仓库。
2. 克隆代码到本地。
3. 提交代码到你的仓库。
4. 创建拉取请求。
```

创建拉取请求时，你必须要确保你的PR内容符合以下要求：

- 目的明确
- 语言流畅
- 代码规范

在编写拉取请求标题时，必须携带如以下标识， 主要分为这几种：
```angular2html
1. [Feature]
2. [Bug Fix]
```
如果是针对某一个模块，请在创建拉取请求时，在标题中携带类型以及你的模块名称，例如：
```angular2html
[Feature][stdio] 新增print函数对于文件流操作的支持
```

## 有关库/标准库:

标准库的代码存放在`extensions/standard`目录下，每个文件对应一个模块，模块的名称就是文件的名称，且每个模块都要有对应的头文件，头文件内注册Lamina函数。

在拓展层注册的Lamina变量均为全局变量。

### 注册Lamina函数的方式： 

调用LAMINA_FUNC宏，例如：
```c++
namespace lamina{
     LAMINA_FUNC("lamina_func_name", cpp_func_name, arg_counts);
}
```
其实，您也可以不用将函数注册到lamina命名空间下，而是直接注册，例如：
```c++
LAMINA_FUNC("lamina_func_name", cpp_func_name, arg_counts);
```

但是，这是一种更为规范的方式，我们更推荐这样做！

若你需要声明Lamina函数，它在C++层的返回值必须声明为```Value```，并携带```const std::vector<Value> &args```参数，并且我们更推荐使用lamina.hpp中对于数据类型操作的宏，如```LAMINA_BOOL```，这对于你的Lamina库项目会更直观！

但是，由于部分历史遗留问题，一些Lamina标准库的内容没有使用这些宏作为返回值。

在编写标准库的代码时，必须要遵循以下规范：

- 代码必须要确保一定的安全性。
- 代码必须要符合Lamina拓展的风格。

## !! 当你为Lamina编写其他库的时候，也亦是如此。

## 模块解析：

Lamina主要的核心模块有

- bigint.hpp 大整数模块
- interpreter.cpp 解释器模块
- irrational.hpp 无理数模块
- lamina.hpp 访问Lamina部分核心资源的模块
- module.cpp 加载Lamina动态库的模块
- rational.hpp 有理数模块
- value.hpp 数值模块
- parser.cpp 解析器模块
- lexer.cpp 词法分析器模块

让我们从0开始，讲解这些模块内的函数，以便于让你进入Lamina的库开发！

在编写Lamina库中，最重要的便是```lamina.hpp```模块，此模块提供了Lamina库开发的一些基本的宏。
```c++
// Source Code:
#pragma once
/*
    对LAMINA核心资源操作的头文件
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

- ``` LAMINA_FUNC_WIT_ANY_ARGS ```宏用于注册一个可以接受任意参数数量的Lamina函数。
- ``` LAMINA_FUNC ```宏用于注册一个可以接受固定参数数量的Lamina函数。
- ``` LAMINA_FUNC_MULTI_ARGS ```宏用于注册一个可以接受0到固定参数数量的Lamina函数。

他们的宏的内部实现其实都大同小异，只不过对于参数数量的判断略有不同

最终编译成动态库之后，他们的符号表像是这样：
```
0000000000020edc T _Z10test_entryR11Interpreter
```

原函数将会带有```_entry```后缀，后续将注册到```builtin_functions```这个vector容器中。

```LAMINA_BOOL```宏用于操作Lamina中的布尔数据类型
```LAMINA_INT```宏用于操作Lamina中的整数数据类型
```LAMINA_STRING```宏用于操作Lamina中的字符串数据类型
```LAMINA_BIGINT```宏用于操作Lamina中的大整数数据类型
```LAMINA_RATIONAL```宏用于操作Lamina中的有理数数据类型
```LAMINA_IRRATIONAL```宏用于操作Lamina中的无理数数据类型
```LAMINA_ARR```宏用于操作Lamina中的数组数据类型
```LAMINA_MATRIX```宏用于操作Lamina中的矩阵数据类型
```LAMINA_NULL```宏用于操作Lamina中的空值数据类型

可以使用```LAMINA_BOOL```等宏来直观操作Lamina函数的返回值，例如随机库当中的：
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

这段Lamina库代码展示了根据用户输入的字符来随机生成一串字符串，并通过```LAMINA_STRING```宏来返回值，若失败，则使用```LAMINA_NULL```宏来返回空值。

- ```LAMINA_GET_VAR```宏用于在解释器运行时，在已经注册的函数内部，获取Lamina中的变量值。
- ```LAMINA_GLOBAL_VAR```宏用于定义Lamina中的全局变量。

这两个目前在标准库中还未使用，但是我们仍旧给出一个例子，便于开发：
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
该例子主要展示了Lamina全局变量的注册方法和使用方法，在获取变量时需要传入一个解释器实例，和一个变量名，在注册全局变量的时候，需要传入一个变量名参数和值。

```L_ERR```宏用于在Lamina内部执行过程中抛出一个错误，此函数不做过多讲解，仅给使用示例：
```c++
#include "lamina.hpp"

Value a(const std::vector<Value> &args){
     L_ERR("a is not defined");
     return LAMINA_NULL;
}
```

关于库的部分，我们已经讲解完毕，跳出Lamina扩展层，我们来到更为底层解释器模块！

Lamina的解释器主要有这几个模块构成，他们共同支撑了Lamina在数学计算方面的优秀。

- 大整数模块
- 无理数模块
- 有理数模块

其次，还有更为底层的解析器模块和词法分析器模块，下文统称为语法处理模块。

让我们先从```interpreter.cpp```的源文件进行分析。

用于源码过长，这里只展示函数原型和头文件内容。
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
    // 禁止拷贝，允许移动
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
    int max_recursion_depth = 100;  // 可变的递归深度限制
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

```error_and_exit```此函数会打印错误信息并退出程序，他的具体实现如下：
```c++
void error_and_exit(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
    exit(1);
}
```
该函数使用时需要传入一个错误信息字符串，该字符串会被打印出来，并且程序会退出。

```StackFrame```结构体存储了关于函数调用的追踪信息，他有这么几个成员变量：
- ```function_name```：函数名
- ```file_name```：文件名
- ```line_number```：行号

这些信息会在程序运行调用函数错误时被打印出来，用于帮助开发者定位错误的位置。

在Lamina中，还有一些异常类，例如
```RuntimeError```类继承自```std::exception```，用于表示运行时错误。
```ReturnException```类继承自```std::exception```，用于表示返回语句错误。
```BreakException```类继承自```std::exception```，用于表示跳出循环语句错误。
```ContinueException```类继承自```std::exception```，用于表示继续循环语句错误。

解释器类定义了一些方法，用于计算表达式值和处理控制台内容以及扩展函数和变量。

下面关于类内方法的讲解顺序，均为公开方法到私有方法，公有成员变量到私有成员变量。

```Interpreter```此构造函数用于调用```register_builtin_functions()```方法，用于注册内置/扩展函数。

```execute```此方法用于执行AST树，他的参数是一个```Statement```指针，返回值为空。

```eval```此方法用于计算表达式值，他的参数是一个```ASTNode```指针，返回值是一个```Value```对象。

```printVariables```此方法用于打印栈内的所有变量，它的参数为空，返回值为空。

```add_function```此方法用于添加函数，他的参数是一个函数名和一个```FuncDefStmt```指针，返回值为空。

```push_frame```此方法用于添加函数调用信息，他的参数是一个函数名、文件名和行号，返回值为空。

```pop_frame```此方法用于弹出函数调用信息，他的参数为空，返回值为空。

```register_entry```此方法用于注册入口函数，他的参数是一个```EntryFunction```指针，返回值为空。

```set_variable```此方法用于设置变量，他的参数是一个变量名和一个```Value```对象，返回值为空。

```get_variable```此方法用于获取变量，他的参数是一个变量名，返回值是一个```Value```对象。

```set_global_variable```此方法用于设置全局变量，他的参数是一个变量名和一个```Value```对象，返回值为空。

此为对于控制台的操作：

```get_stack_trace```此方法用于获取栈调用信息，他的参数为空，返回值是一个```std::vector<StackFrame>```对象。

```print_stack_trace```此方法用于打印栈调用信息，他的参数是一个```RuntimeError```对象和一个```bool```值，返回值为空。

```supports_colors```此方法用于判断当前终端是否支持颜色输出，他的参数为空，返回值是一个```bool```值。

```print_error```此方法用于打印错误信息，他的参数是一个```std::string```对象和一个```bool```值，返回值为空。

```print_warning```此方法用于打印警告信息，他的参数是一个```std::string```对象和一个```bool```值，返回值为空。

成员变量：

```builtin_functions```此方法存储了所有的内置/扩展函数，他的键是函数名，值是函数指针。

```functions```此方法存储函数定义。

```variable_stack```此方法存储了所有的变量，以及作用域，他的键是变量名，值是```Value```对象。

```loaded_module_asts```此方法存储了所有的已加载模块的抽象语法树，他的键是模块名，值是抽象语法树指针。

接下来，我们从解释器继续讲解，引入到Lamina对于数字处理的部分。

首先，我们讲解大整数模块:
```c++
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <climits>

class BigInt {
private:
    std::vector<int> digits;  // 存储数字，低位在前
    bool negative;

public:
    // 构造函数
    BigInt() : negative(false) { digits.push_back(0); }
    
    BigInt(int n) : negative(n < 0) {
        if (n == 0) {
            digits.push_back(0);
            return;
        }
        
        // 处理 INT_MIN 的特殊情况
        if (n == INT_MIN) {
            // INT_MIN 的绝对值超出 int 范围，直接处理
            long long ln = static_cast<long long>(n);
            ln = -ln;  // 现在安全了
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
    
    // 移除前导零
    void remove_leading_zeros() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.size() == 1 && digits[0] == 0) {
            negative = false;
        }
    }
    
    // 转换为字符串
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
    
    // 转换为int（如果可能）
    int to_int() const {
        if (digits.size() > 10) {  // 太大了
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
    
    // 检查是否为零
    bool is_zero() const {
        return digits.size() == 1 && digits[0] == 0;
    }
    
    // 比较运算符
    bool operator==(const BigInt& other) const {
        return negative == other.negative && digits == other.digits;
    }
    
    bool operator!=(const BigInt& other) const {
        return !(*this == other);
    }
};
```


`BigInt`提供了对于整形处理的更加安全的形式，避免了整形溢出的问题。

在`BigInt`的内部，我们定义了两个成员变量：
- `std::vector<int> digits` 用于存储大整数的每一位数字，低位在前。
- `bool negative` 用于表示大整数的正负性，`true`表示负数，`false`表示正数。

`BigInt`的构造函数有多个重载，允许从不同的类型构造大整数，例如从`int`、`long long`、`std::string`等。

`BigInt`还提供了一些基本的运算符重载，如加法、减法、乘法、除法等，以及比较运算符。

接下来，我们继续讲解无理数/有理数模块

由于代码过长，这里暂时不做任何展示

无理数模块支持常见的无理数的精确表示，例如π、e等。

在无理数模块当中，我们定义了一个enum类，用于表示无理数的类型，这里一共有四个类型，我们这边直接展示enum类的代码。
```c++
   enum class Type {
        SQRT,      // √n 形式
        PI,        // π 的倍数
        E,         // e 的倍数  
        LOG,       // log(n) 形式
        COMPLEX    // 复合形式 (a*√b + c*π + d*e + ...)
    };
```
在私有成员变量里，我们定义了两个变量，用于表达根号下n的形式。
```c++
    // 对于 √n 形式：coefficient * √radicand
    double coefficient;  // 系数
    long long radicand;  // 根号内的数
```

对于不同形式，无理数模块也做了针对性处理，比如根号下n的形式，使用coefficient * √radicand，对于复合形式，我们采用了系数映射的方式：
```c++
    std::map<std::string, double> coefficients;
    double constant_term;  // 常数项
```

在有理数模块中，可以表达基本的a/b形式，以及不常见的分数，并且支持化简分数。

在化简分数的过程中，不用过于考虑分母值的安全性，因为在 rational 模块中，我们已经对分母进行了检查，确保分母不为零。

我们跳出有关于数字处理的部分，接下来讲解语法分析模块。

在语法分析模块中的解析器模块，我们在头文件中定义了一些函数原型。

解析器模块用于解析不同的表达式类型，在ast模块中定义了表达式的节点。

词法分析模块用于将输入的字符串转换为词法单元序列，每个词法单元对应一个标记（token）。

在词法分析模块中，我们定义了一个`Token`结构体，用于表示词法单元。

`Token`结构体包含了词法单元的类型、值以及位置信息。

词法分析模块的主要功能是将输入的字符串转换为词法单元序列，同时处理注释、空白字符等。

在词法分析模块中，我们使用了一个简单的状态机来实现词法分析。

接下来，我们可以进入库加载的模块。

目前库加载仅支持Linux平台下的加载，库加载主要有这几个部分构成:

1. 动态链接库的加载
2. 符号的查找和绑定
3. 函数的调用

在库加载模块中，我们使用了`dlopen`函数来加载动态链接库，使用`dlsym`函数来查找符号，使用`dlclose`函数来关闭动态链接库。

`findSymbol`函数接受一个符号名作为参数，返回该符号的地址。

