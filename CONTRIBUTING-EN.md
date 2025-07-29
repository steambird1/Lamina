# Code Submission Guidelines -- English

*   Please read [https://bgithub.xyz/Ziyang-Bai/Lamina/blob/main/README.md](https://bgithub.xyz/Ziyang-Bai/Lamina/blob/main/README.md) first to understand the basic information of the project before proceeding with development!

*   This project uses the GPL-3.0 license; please strictly abide by this license!

## Project Structure:

```
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
├── CONTRIUTING-EN.md
```

Total: 5 folders, 39 files.

## Creating a Pull Request:

```
1. Fork the code to your repository.

2. Clone the code to your local machine.

3. Commit the code to your repository.

4. Create a pull request.
```

When creating a pull request, you must ensure that your PR content meets the following requirements:

*   Clear purpose

*   Fluent language

*   Code compliance with standards

When writing the pull request title, it must carry one of the ranks, mainly divided into these types:
```
1. [Feature]

2. [Bug Fix]
```

If it is for a specific module, please include the type and your module name in the title when creating the pull request. For example:

```
[Feature][stdio] Add support for file stream operations in the print function
```

## About Libraries/Standard Libraries:

The standard library code is stored in the `extensions/standard` directory. Each file corresponds to a module, where the module name is the same as the file name, and each module must have a corresponding header file that registers Lamina functions.

All Lamina variables registered in the extension layer are global variables.

### How to Register Lamina Functions:

Call the ```LAMINA_FUNC``` macro, for example:

```c++
namespace lamina{
    LAMINA_FUNC("lamina_func_name", cpp_func_name, arg_counts);
}
```

But, in fact, you can also register functions directly without placing them in the lamina namespace, for example:
```c++
LAMINA_FUNC("lamina_func_name", cpp_func_name, arg_counts);
```

However, the former is a more standardized approach, and we recommend it!

If you need to declare a Lamina function, its return value at the C++ layer must be declared as `Value`, with the parameter `const std::vector<Value> &args`. We also recommend using the macros for data type operations in lamina.hpp, such as `LAMINA_BOOL`, which will make your Lamina library project more intuitive!

However, due to some historical issues, some content in the Lamina standard library does not use these macros for return values.

When writing standard library code, you must follow these guidelines:

*   The code must ensure a certain level of security.

*   The code must conform to the style of Lamina extensions.

## !! The same applies when you write other libraries for Lamina.

## Module Analysis:

The main core modules of Lamina include:

*  bigint.hpp: Big integer module

*   interpreter.cpp: Interpreter module

*   irrational.hpp: Irrational number module

*   lamina.hpp: Module for accessing some core resources of Lamina

*   module.cpp: Module for loading Lamina dynamic libraries

*   rational.hpp: Rational number module

*   value.hpp: Value module

*   parser.cpp: Parser module

*   lexer.cpp: Lexer module

Let's start from scratch and explain the functions within these modules to help you get started with Lamina library development!

In Lamina library development, the most important module is `lamina.hpp`, which provides some basic macros for Lamina library development.

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

*   The `LAMINA_FUNC_WIT_ANY_ARGS` macro is used to register a Lamina function that can accept any number of arguments.

*   The `LAMINA_FUNC` macro is used to register a Lamina function that accepts a fixed number of arguments.

*   The `LAMINA_FUNC_MULTI_ARGS` macro is used to register a Lamina function that accepts 0 to a fixed number of arguments.

The internal implementation of these macros is quite similar, with only slight differences in the judgment of the number of arguments.

After being compiled into a dynamic library, their symbol tables look like this:



```
0000000000020edc T \_Z10test\_entryR11Interpreter
```

The original function will have an `_entry` suffix and will later be registered in the `builtin_functions` vector container.

The `LAMINA_BOOL` macro is used to manipulate boolean data types in Lamina.

The `LAMINA_INT` macro is used to manipulate integer data types in Lamina.

The `LAMINA_STRING` macro is used to manipulate string data types in Lamina.

The `LAMINA_BIGINT` macro is used to manipulate big integer data types in Lamina.

The `LAMINA_RATIONAL` macro is used to manipulate rational number data types in Lamina.

The `LAMINA_IRRATIONAL` macro is used to manipulate irrational number data types in Lamina.

The `LAMINA_ARR` macro is used to manipulate array data types in Lamina.

The `LAMINA_MATRIX` macro is used to manipulate matrix data types in Lamina.

The `LAMINA_NULL` macro is used to manipulate null values in Lamina.

You can use macros like `LAMINA_BOOL` to intuitively manipulate the return values of Lamina functions. For example, in the random library:


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

This Lamina library code demonstrates generating a random string based on the user-input length, returning the value via the `LAMINA_STRING` macro, or returning a null value via the `LAMINA_NULL` macro if it fails.

*   The `LAMINA_GET_VAR` macro is used to retrieve the value of a variable in Lamina within a registered function during interpreter runtime.

*   The `LAMINA_GLOBAL_VAR` macro is used to define global variables in Lamina.

These two are not yet used in the standard library, but we still provide an example for development:

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

This example demonstrates the registration and usage of Lamina global variables. To retrieve a variable, you need to pass an interpreter instance and a variable name. When registering a global variable, you need to pass a variable name parameter and a value.

The `L_ERR` macro is used to throw an error during Lamina's internal execution. Here's a usage example without further explanation:

```c++
#include "lamina.hpp"

Value a(const std::vector<Value> &args){
     L_ERR("a is not defined");
     return LAMINA_NULL;
}
```

We've covered the library section. Now, let's move beyond the Lamina extension layer to the more underlying interpreter module!

Lamina's interpreter mainly consists of these modules, which collectively support Lamina's excellence in mathematical calculations:


*   Big integer module

*   Irrational number module

*   Rational number module

Additionally, there are more underlying parser and lexer modules, collectively referred to as syntax processing modules below.

Let's start with the source file analysis of `interpreter.cpp`.

Due to the lengthy source code, only function prototypes and header file content are shown here.

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

The `error_and_exit` function prints an error message and exits the program. Its specific implementation is as follows:

```c++
void error_and_exit(const std::string& msg) {
    std::cerr << "Error: " << msg << std::endl;
    exit(1);
}
```

This function requires passing an error message string, which will be printed, and the program will exit.

The `StackFrame` struct stores tracking information about function calls, with these member variables:

*   `function_name`: Function name

*   `file_name`: File name

*   `line_number`: Line number

This information will be printed when an error occurs during function calls at runtime to help developers locate the error.

In Lamina, there are several exception classes, such as:

The `RuntimeError` class inherits from `std::exception` and is used to represent runtime errors.

The `ReturnException` class inherits from `std::exception` and is used to represent return statement errors.

The `BreakException` class inherits from `std::exception` and is used to represent loop break statement errors.

The `ContinueException` class inherits from `std::exception` and is used to represent loop continue statement errors.

The interpreter class defines methods for evaluating expression values, handling console content, and extending functions and variables.

The following explanation of class methods follows the order from public to private methods, and public member variables to private member variables.

The `Interpreter` constructor calls the `register_builtin_functions()` method to register built-in/extended functions.

The `execute` method is used to execute the AST tree, taking a `Statement` pointer as a parameter and returning nothing.

The `eval` method is used to evaluate expression values, taking an `ASTNode` pointer as a parameter and returning a `Value` object.

The `printVariables` method is used to print all variables in the stack, taking no parameters and returning nothing.

The `add_function` method is used to add functions, taking a function name and a `FuncDefStmt` pointer as parameters and returning nothing.

The `push_frame` method is used to add function call information, taking a function name, file name, and line number as parameters and returning nothing.

The `pop_frame` method is used to pop function call information, taking no parameters and returning nothing.

The `register_entry` method is used to register entry functions, taking an `EntryFunction` pointer as a parameter and returning nothing.

The `set_variable` method is used to set variables, taking a variable name and a `Value` object as parameters and returning nothing.

The `get_variable` method is used to retrieve variables, taking a variable name as a parameter and returning a `Value` object.

The `set_global_variable` method is used to set global variables, taking a variable name and a `Value` object as parameters and returning nothing.

Console operations:

The `get_stack_trace` method is used to retrieve stack call information, taking no parameters and returning a `std::vector<StackFrame>` object.

The `print_stack_trace` method is used to print stack call information, taking a `RuntimeError` object and a `bool` value as parameters and returning nothing.

The `supports_colors` method is used to determine whether the current terminal supports color output, taking no parameters and returning a `bool` value.

The `print_error` method is used to print error messages, taking a `std::string` object and a `bool` value as parameters and returning nothing.

The `print_warning` method is used to print warning messages, taking a `std::string` object and a `bool` value as parameters and returning nothing.

Member variables:

`builtin_functions` stores all built-in/extended functions, with keys as function names and values as function pointers.

`functions` stores function definitions.

`variable_stack` stores all variables and scopes, with keys as variable names and values as `Value` objects.

`loaded_module_asts` stores the abstract syntax trees of all loaded modules, with keys as module names and values as abstract syntax tree pointers.

Next, we continue from the interpreter to Lamina's number processing section.

First, let's explain the big integer module:


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

`BigInt` provides a safer form of integer processing, avoiding integer overflow issues.

Internally, `BigInt` defines two member variables:

*   `std::vector<int> digits`: Stores each digit of the big integer, with the least significant digit first.

*   `bool negative`: Indicates the sign of the big integer; `true` for negative, `false` for positive.

`BigInt` has multiple overloaded constructors, allowing big integers to be constructed from different types such as `int`, `long long`, `std::string`, etc.

`BigInt` also provides some basic operator overloads, such as addition, subtraction, multiplication, division, and comparison operators.

Next, we continue with the irrational/rational number modules.

Due to the length of the code, no examples are shown here.

The irrational number module supports precise representation of common irrational numbers such as π, e, etc.

In the irrational number module, we define an enum class to represent the type of irrational number. There are four types, and we directly show the enum class code here:


```c++
   enum class Type {
        SQRT,      // √n 形式
        PI,        // π 的倍数
        E,         // e 的倍数  
        LOG,       // log(n) 形式
        COMPLEX    // 复合形式 (a*√b + c*π + d*e + ...)
    };
```
In the private member variables, we define two variables to express the form of √n:


```c++
    // 对于 √n 形式：coefficient * √radicand
    double coefficient;  // 系数
    long long radicand;  // 根号内的数
```
The irrational number module handles different forms specifically. For example, the √n form uses coefficient \* √radicand, and for composite forms, we use a coefficient mapping approach:


```c++
    std::map<std::string, double> coefficients;
    double constant_term;  // 常数项
```

The rational number module can express basic a/b forms and uncommon fractions, and supports fraction simplification.

During fraction simplification, there's no need to overly consider the safety of the denominator value, because the rational module already checks the denominator to ensure it is not zero.

Moving beyond number processing, let's explain the syntax analysis module.

In the parser module of the syntax analysis module, we define some function prototypes in the header file.

The parser module is used to parse different expression types, and the ast module defines expression nodes.

The lexer module is used to convert input strings into a sequence of lexical units, where each lexical unit corresponds to a token.

In the lexer module, we define a `Token` struct to represent lexical units.

The `Token` struct contains the type, value, and position information of the lexical unit.

The main function of the lexer module is to convert input strings into a sequence of lexical units, while handling comments, whitespace characters, etc.

In the lexer module, we use a simple state machine to implement lexical analysis.

Next, we can move to the library loading module.

Currently, library loading only supports loading on Linux platforms. Library loading mainly consists of these parts:

1.  Loading of dynamic link libraries

2.  Symbol lookup and binding

3.  Function calling

In the library loading module, we use the `dlopen` function to load dynamic link libraries, `dlsym` to find symbols, and `dlclose` to close dynamic link libraries.

The `findSymbol` function accepts a symbol name as a parameter and returns the address of that symbol.