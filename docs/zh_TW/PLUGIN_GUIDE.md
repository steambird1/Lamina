# Lamina 模組開發指南

<div align="right">
  <strong>繁體中文</strong> | <a href="../zh_CN/PLUGIN_GUIDE.md">简体中文</a> | <a href="../en_US/PLUGIN_GUIDE.md">English</a>
</div>

## 概述

Lamina 支援以動態庫（DLL/SO）模組擴充功能。模組系統採用現代化設計，提供型別安全的跨平台 API，支援命名空間管理與函式註冊機制。

## 專案結構

```
Lamina/
├── interpreter/          # 核心解譯器
│   ├── module_api.hpp    # 模組 API 定義
│   ├── module_loader.hpp # 模組載入器
│   └── ...               # 其他核心檔案
├── extensions/           # 擴充模組原始碼
│   ├── minimal/          # 範例模組
│   │   ├── ultra_minimal.cpp  # 模組實作
│   │   └── CMakeLists.txt     # 建構配置
│   └── standard/         # 標準庫模組
│       ├── array.cpp     # 陣列操作
│       ├── math.cpp      # 數學函式
│       ├── stdio.cpp     # 輸入輸出
│       └── ...           # 其他標準庫
├── modules/              # 編譯後模組 (.dll/.so)
└── test/                 # 測試檔案
```

## 模組 API 概述

### 核心特色

- **命名空間支援**：每個模組擁有獨立命名空間，避免函式名稱衝突
- **型別安全**：統一的 `LaminaValue` 型別系統，支援基本資料型態
- **跨平台相容**：使用 `LAMINA_EXPORT` 及 `LAMINA_CALL` 巨集確保平台相容性
- **動態載入**：執行時載入模組，無需重新編譯解譯器

### 資料型態系統

```cpp
// 支援的值型態
typedef enum {
    LAMINA_TYPE_NULL = 0,
    LAMINA_TYPE_BOOL,
    LAMINA_TYPE_INT,      // 整數型態
    LAMINA_TYPE_NUMBER,   // 浮點型態
    LAMINA_TYPE_STRING    // 字串型態
} LaminaValueType;

// 統一的值結構
typedef struct {
    LaminaValueType type;
    union {
        bool bool_val;
        int int_val;
        double number_val;
        const char* string_val;
    } data;
} LaminaValue;
```

### 模組介面結構

```cpp
// 模組函式簽名
typedef LaminaValue (LAMINA_CALL *LaminaFunction)(const LaminaValue* args, int argc);

// 函式註冊條目
typedef struct {
    const char* name;        // 函式名稱
    LaminaFunction func;     // 函式指標
    const char* description; // 函式描述
} LaminaFunctionEntry;

// 模組資訊
typedef struct {
    const char* namespace_name; // 命名空間名稱
    const char* version;        // 版本號
    const char* description;    // 模組描述
} LaminaModuleInfo;

// 模組匯出結構
typedef struct {
    LaminaModuleInfo info;           // 模組基本資訊
    LaminaFunctionEntry* functions;  // 匯出函式陣列
    int function_count;              // 函式數量
} LaminaModuleExports;
```

## 建立第一個模組

### 1. 最簡單的模組範例

以下為一個完整最小模組實作（參考 `extensions/minimal/ultra_minimal.cpp`）：

```cpp
#include "../../interpreter/module_api.hpp"
#include <cstdio>

// 簡單的測試函式
LAMINA_EXPORT LaminaValue LAMINA_CALL test_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};  // 零初始化
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = 42;
    
    printf("test_function called successfully!\n");
    return result;
}

// 更複雜的函式範例
LAMINA_EXPORT LaminaValue LAMINA_CALL hello_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = 100;
    
    printf("Hello from minimal module function!\n");
    return result;
}

// 匯出函式表
static LaminaFunctionEntry functions[] = {
    {"test", test_function, "Simple test function returning 42"},
    {"hello", hello_function, "Hello function returning 100"}
};

// 模組匯出結構
static LaminaModuleExports exports = {
    {
        "minimal",                    // 命名空間名稱
        "1.0.0",                     // 版本號
        "Ultra minimal test module"   // 描述
    },
    functions,                       // 函式陣列
    2                               // 函式數量
};

// 必要的匯出函式
LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    return &exports;
}
```

### 2. CMakeLists.txt 設定

```cmake
# 最小模組建構配置
cmake_minimum_required(VERSION 3.10)

# 設定模組名稱
set(MODULE_NAME minimal)

# 建立共享庫
add_library(${MODULE_NAME} SHARED
    ultra_minimal.cpp
)

# 設定輸出目錄
set_target_properties(${MODULE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
)

# 跨平台編譯設定
if(WIN32)
    # Windows 特定設定
    set_target_properties(${MODULE_NAME} PROPERTIES
        SUFFIX ".dll"
        PREFIX ""
    )
else()
    # Linux/macOS 設定
    set_target_properties(${MODULE_NAME} PROPERTIES
        SUFFIX ".so"
        PREFIX "lib"
    )
endif()

# 編譯標準
set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
```

## 編譯與使用模組

### 編譯模組

```bash
# 配置建構
cmake -B build -DCMAKE_BUILD_TYPE=Debug .

# 編譯專案（包含所有模組）
cmake --build build --config Debug --parallel

# 模組輸出至：build/Debug/minimal.dll (Windows) 或 build/Debug/libminimal.so (Linux)
```

### 在 Lamina 腳本中使用模組

```lamina
// 載入模組
include "minimal.dll";

// 呼叫模組函式
let result = minimal.test();
print("Test result: " + result);  // 輸出: Test result: 42

let hello_result = minimal.hello();
print("Hello result: " + hello_result);  // 輸出: Hello result: 100
```

### 執行範例

```bash
# 確保模組檔案可存取
cd build/Debug

# 執行 Lamina 解譯器
./Lamina.exe ../../test/test_minimal.lamina
```

## 進階模組開發

### 參數處理與型別檢查

```cpp
LAMINA_EXPORT LaminaValue LAMINA_CALL advanced_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};
    
    // 檢查參數數量
    if (argc < 2) {
        result.type = LAMINA_TYPE_STRING;
        result.data.string_val = "Error: Need at least 2 arguments";
        return result;
    }
    
    // 型別檢查
    if (args[0].type != LAMINA_TYPE_INT || args[1].type != LAMINA_TYPE_INT) {
        result.type = LAMINA_TYPE_STRING;
        result.data.string_val = "Error: Arguments must be integers";
        return result;
    }
    
    // 執行計算
    int a = args[0].data.int_val;
    int b = args[1].data.int_val;
    
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = a * b;
    return result;
}
```

### 字串處理

```cpp
LAMINA_EXPORT LaminaValue LAMINA_CALL string_processor(const LaminaValue* args, int argc) {
    static char buffer[512];  // 靜態緩衝區確保生命週期
    LaminaValue result = {0};
    
    if (argc < 1 || args[0].type != LAMINA_TYPE_STRING) {
        result.type = LAMINA_TYPE_STRING;
        result.data.string_val = "Error: Expected string argument";
        return result;
    }
    
    // 安全字串處理
    snprintf(buffer, sizeof(buffer), "Processed: %s", args[0].data.string_val);
    
    result.type = LAMINA_TYPE_STRING;
    result.data.string_val = buffer;
    return result;
}
```

### 錯誤處理模式

```cpp
// 統一的錯誤回傳模式
static LaminaValue make_error(const char* message) {
    LaminaValue error = {0};
    error.type = LAMINA_TYPE_STRING;
    error.data.string_val = message;
    return error;
}

// 統一的成功回傳模式
static LaminaValue make_success_int(int value) {
    LaminaValue result = {0};
    result.type = LAMINA_TYPE_INT;
    result.data.int_val = value;
    return result;
}
```

## 最佳實踐

### 1. 模組設計原則

- **單一職責**：每個模組專注於特定功能領域
- **命名空間明確**：使用簡潔且具描述性的命名空間名稱
- **函式命名**：以動詞描述函式功能，保持一致性
- **版本管理**：採用語意化版本號（major.minor.patch）

### 2. 程式碼規範

```cpp
// 良好函式命名範例
{"calculate", math_calculate, "Calculate mathematical expression"},
{"format", string_format, "Format string with parameters"},
{"validate", data_validate, "Validate input data"}

// 避免命名
{"func1", some_function, "Does something"},
{"process", unclear_function, "Process data"}
```

### 3. 錯誤處理策略

- 始終驗證參數數量及型別
- 回傳具意義的錯誤訊息
- 採用一致錯誤格式
- 避免程式崩潰，優雅處理異常情況

### 4. 記憶體安全

- 使用靜態緩衝區或全域變數儲存回傳字串
- 避免回傳區域變數指標
- 使用安全字串函式（`snprintf`、`strncpy`）
- 確保緩衝區大小足夠且不溢位

### 5. 跨平台相容性

- 使用提供的 `LAMINA_EXPORT` 與 `LAMINA_CALL` 巨集
- 避免平台特定型態或函式
- 測試多平台及編譯器

## 故障排除

### 常見問題與解決方法

#### 1. 模組載入失敗

**問題**：`ERROR: Module not loaded`

```cpp
// 檢查：
// - DLL 檔案是否在正確位置
// - 檔案名稱是否與 include 語句一致
// - 是否有必要的依賴庫
```

**解決方法**：

- 確保模組檔案在 `build/Debug/` 目錄
- 檢查檔案副檔名（Windows: `.dll`, Linux: `.so`）
- 使用依賴工具檢查缺失的動態庫

#### 2. 函式找不到

**問題**：`ERROR: Function 'funcname' not found in module`

```cpp
// 檢查：
// - 函式是否正確註冊於 functions 陣列
// - 函式名稱拼寫正確
// - 命名空間是否相符
```

**解決方法**：

- 驗證 `LaminaFunctionEntry` 陣列中的函式名稱
- 確認呼叫時使用正確命名空間前綴
- 檢查 `function_count` 是否正確

#### 3. 命名空間不一致

**問題**：`ERROR: Namespace mismatch`

```cpp
// 腳本：include "minimal.dll"; minimal.test();
// 模組：namespace_name = "different_name"
```

**解決方法**：

- 確保模組的 `namespace_name` 與呼叫時前綴一致
- 建議命名空間名稱與檔案名相同

#### 4. 編譯錯誤

**問題**：編譯時出現連結錯誤或符號未定義

```cpp
// 常見原因：
// - 缺少 LAMINA_EXPORT 巨集
// - 呼叫約定不符
// - 標頭檔路徑錯誤
```

**解決方法**：

- 確保所有匯出函式使用 `LAMINA_EXPORT` 與 `LAMINA_CALL`
- 檢查 `module_api.hpp` 包含路徑
- 驗證 CMake 設定正確

### 除錯技巧

#### 1. 加入除錯輸出

```cpp
LAMINA_EXPORT LaminaValue LAMINA_CALL debug_function(const LaminaValue* args, int argc) {
    #ifdef _DEBUG
    printf("DEBUG: Function called with %d arguments\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("DEBUG: Arg[%d] type=%d\n", i, args[i].type);
    }
    #endif
    
    // 實際函式邏輯...
}
```

#### 2. 模組載入驗證

```cpp
LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    printf("Module initializing: %s v%s\n", 
           exports.info.namespace_name, 
           exports.info.version);
    return &exports;
}
```

#### 3. 使用系統工具

- **Windows**: 使用 `dumpbin /exports module.dll` 檢查匯出符號
- **Linux**: 使用 `nm -D module.so` 或 `objdump -T module.so`
- **除錯器**: 使用 GDB (Linux) 或 Visual Studio 除錯器 (Windows)

## 範例與參考

### 完整範例模組

參見專案內示例模組獲取完整實作：

#### minimal 模組 (`extensions/minimal/`)

- **檔案**: `ultra_minimal.cpp`
- **功能**: 最基礎模組範例
- **函式**: `test()`, `hello()`
- **用途**: 學習模組開發基礎

#### standard 模組 (`extensions/standard/`)

- **array.cpp**: 陣列操作函式
- **math.cpp**: 數學運算函式
- **stdio.cpp**: 輸入輸出函式
- **times.cpp**: 時間處理函式
- **random.cpp**: 隨機數產生
- **sockets.cpp**: 網路通訊功能

### 快速開始模板

建立新模組完整流程：

1. **建立目錄結構**

```bash
mkdir extensions/mymodule
cd extensions/mymodule
```

2. **建立原始檔 (mymodule.cpp)**

```cpp
#include "../../interpreter/module_api.hpp"

LAMINA_EXPORT LaminaValue LAMINA_CALL my_function(const LaminaValue* args, int argc) {
    LaminaValue result = {0};
    result.type = LAMINA_TYPE_STRING;
    result.data.string_val = "Hello from my module!";
    return result;
}

static LaminaFunctionEntry functions[] = {
    {"greet", my_function, "Greet the user"}
};

static LaminaModuleExports exports = {
    {"mymodule", "1.0.0", "My custom module"},
    functions,
    1
};

LAMINA_EXPORT LaminaModuleExports* LAMINA_CALL lamina_module_init() {
    return &exports;
}
```

3. **建立建構檔 (CMakeLists.txt)**

```cmake
add_library(mymodule SHARED mymodule.cpp)
set_target_properties(mymodule PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    CXX_STANDARD 17
)
```

4. **加入主建構系統**
於主 `CMakeLists.txt` 中添加：

```cmake
add_subdirectory(extensions/mymodule)
```

5. **編譯與測試**

```bash
cmake --build build --config Debug
cd build/Debug
./Lamina.exe
```

### API 參考速查

#### 型態定義

```cpp
LAMINA_TYPE_NULL     // 空值
LAMINA_TYPE_BOOL     // 布林值
LAMINA_TYPE_INT      // 整數
LAMINA_TYPE_NUMBER   // 浮點數
LAMINA_TYPE_STRING   // 字串
```

#### 巨集定義

```cpp
LAMINA_EXPORT        // 匯出函式宣告
LAMINA_CALL          // 呼叫約定
```

#### 必要函式

```cpp
lamina_module_init() // 模組初始化入口
```

---
