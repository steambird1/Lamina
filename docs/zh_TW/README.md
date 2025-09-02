# <img src="/assets/logo-icon.svg" width="10%"> Lamina

<img src="/assets/logo.svg" width="100%">

<div align="right">
  <strong>繁體中文</strong> | <a href="/README.md">简体中文</a> | <a href="../en_US/README.md">English</a>
</div>
<br>

[![GitHub issues](https://img.shields.io/github/issues/lamina-dev/Lamina)](https://github.com/Lamina-dev/Lamina/issues)
[![GitHub stars](https://img.shields.io/github/stars/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/forks)
[![GitHub contributors](https://img.shields.io/github/contributors/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/graphs/contributors)
![GitHub last commit](https://img.shields.io/github/last-commit/lamina-dev/Lamina?style=flat)
[![License](https://img.shields.io/badge/license-GNUv3-blue.svg)](/LICENSE)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![Math](https://img.shields.io/badge/math-precise-green.svg)](#精確數學特性)
[![QQ](https://img.shields.io/badge/QQ-%E4%BA%A4%E6%B5%81%E7%BE%A3-red?logo=qq&logoColor=white)](https://qm.qq.com/q/QwPXCgsJea)

## 一種專注於精確數學計算的程序式程式語言

[快速開始](#快速開始) • [語法指南](#基礎語法) • [數學特性](#精確數學特性) • [範例程式碼](#範例程式碼) • [編譯指南](Compile.md) • [貢獻指南](CONTRIBUTING.md) • [Wiki](https://github.com/lamina-dev/Lamina/wiki) • [動態庫插件開發](PLUGIN_GUIDE.md)

---

## 目錄

- [概述](#概述)
- [精確數學特性](#精確數學特性)
- [快速開始](#快速開始)
- [基礎語法](#基礎語法)
- [資料型態](#資料型態)
- [運算子](#運算子)
- [內建函式庫](#內建函式庫)
- [範例程式碼](#範例程式碼)
- [設計原則](#設計原則)

---

## 概述

**Lamina** 是一種面向程序的數學計算語言，專注於高效解決數學問題，並支援圖靈完備性。其設計目標是提供簡潔、直覺的語法，同時支援複雜數學運算。

### 主要特色

- **精確數學**：支援有理數（分數）及無理數（√、π、e）的精確表示與運算
- **精度保護**：避免浮點數精度遺失，多次循環運算保持精確
- **數學友好**：原生支援向量、矩陣運算及數學函式
- **遞迴支援**：可設定遞迴深度限制
- **大整數**：支援任意精度的大整數運算
- **互動式REPL**：支援互動式程式設計與腳本執行
- **智能終端**：自動偵測終端色彩支援，避免亂碼
- **堆疊追蹤**：完整的錯誤堆疊追蹤，易於除錯

---

## 精確數學特性

Lamina 的核心優勢在於其精確數學計算能力，解決了傳統程式語言中浮點數精度遺失的問題。

### 有理數（Rational）

- **自動分數表示**：除法運算自動產生精確分數
- **多次運算保持精確**：避免累積誤差
- **自動化簡**：分數自動約簡至最簡形式

```lamina
var fraction = 16 / 9;           // 結果為 16/9，不是 1.777...
var result = fraction;

// 多次循環運算保持精確
var i = 1;
while (i <= 10) {
    result = result * 9 / 9;     // 始終保持為 16/9
    i = i + 1;
}
```

### 無理數（Irrational）

- **符號化表示**：√2、π、e 等保持符號形式
- **自動化簡**：√8 → 2√2，√(a²) → a
- **精確運算**：符號運算避免近似誤差

```lamina
var root2 = sqrt(2);             // 精確的 √2
var root8 = sqrt(8);             // 自動化簡為 2√2
var pi_val = pi();               // 精確的 π
var result = root2 * root2;      // 精確結果：2
```

### 精度對比範例

```lamina
// 傳統浮點數（其他語言）
// 0.1 + 0.2 = 0.30000000000000004

// Lamina 精確計算
var result = 1/10 + 2/10;        // 結果：3/10（完全精確）
```

---

## 快速開始

### 編譯與執行

```bash
# 初始化子模組（首次建構或更新後需要）
git submodule update --init --recursive

# 編譯
# 請參考編譯指南

# 互動模式
./lamina

# 執行腳本
./lamina script.lm
```

**注意**：Lamina 使用 libuv 函式庫以支援網路功能，該函式庫以 Git 子模組形式納入專案中。首次建構或更新程式碼前，請務必執行上述子模組初始化命令。

### 簡單範例

```lamina
// 精確數學計算
print("Hello, Lamina!");
var fraction = 16 / 9;          // 精確分數，不是浮點數
print("16/9 =", fraction);      // 輸出: 16/9

var root = sqrt(2);             // 精確無理數
print("√2 =", root);            // 輸出: √2
print("√2 × √2 =", root * root);// 輸出: 2（精確結果）

// 向量運算
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];
print("v1 + v2 =", v1 + v2);
print("v1 · v2 =", dot(v1, v2));

// 精確常數
var pi_val = pi();
print("π =", pi_val);           // 輸出: π（符號形式）
```

---

## 基礎語法

### 陳述句終止符

**分號必不可少**：Lamina 要求所有陳述句必須以分號 `;` 結束，這提升了解析效率及程式可讀性。

```lamina
var x = 10;           //  正確
print(x);             //  正確
bigint big = 100!;    //  正確

var y = 20            //  錯誤：缺少分號
print(y)              //  錯誤：缺少分號
```

### 模組系統

Lamina 支援模組化程式設計，可使用 `include` 陳述句引入其他檔案。

#### include 陳述句

**強制使用引號**：include 陳述句必須使用引號包裹檔案名稱，確保語法一致性與清晰。

```lamina
include "math_utils";     //  正確：使用引號
include "lib/vectors";    //  正確：支援相對路徑

include math_utils;       //  錯誤：必須使用引號
```

#### 模組搜尋路徑

Lamina 會依序搜尋下列路徑：

1. 當前目錄
2. `./` 目錄
3. `./include/` 目錄

如檔案名稱未包含 `.lm` 副檔名，會自動加入。

#### 內建模組

Lamina 提供特殊內建模組：

- `include "splash";` - 顯示 Lamina 啟動畫面
- `include "them";` - 顯示開發者資訊與致謝

### 保留字

以下為保留字，不可用作變數名或函式名：

```plaintext
var func if else while for return break continue
print true false null include define bigint input
```

### 註解

```lamina
// 單行註解
/* 區塊註解 */
```

### 變數宣告

```lamina
var x = 10;
var name = "Alice";
var arr = [1, 2, 3];
bigint large_num = 999999999999;  // 大整數須明確宣告
bigint factorial_result = 25!;    // 可直接由運算結果賦值
```

---

## 資料型態

### 基本型態

| 型態 | 說明 | 範例 |
|------|------|------|
| `int` | 整數 | `42`, `-10` |
| `float` | 浮點數 | `3.14`, `-0.5` |
| `rational` | 有理數（精確分數） | `16/9`, `1/3` |
| `irrational` | 無理數（精確符號） | `√2`, `π`, `e` |
| `bool` | 布林值 | `true`, `false` |
| `string` | 字串 | `"Hello"`, `"world"` |
| `null` | 空值 | `null` |
| `bigint` | 大整數（須明確宣告） | `bigint x = 999999;`, `bigint y = 30!;` |

### 複合型態

```lamina
// 陣列
var arr = [1, 2, 3];
var matrix = [[1,2], [3,4]];     // 2x2矩陣

// 函式
func add(a, b) { 
    return a + b; 
}
```

---

## 運算子

### 算術運算子

| 運算子 | 功能 | 範例 |
|--------|------|------|
| `+` | 加法 | `2 + 3 → 5` |
| `-` | 減法 | `5 - 2 → 3` |
| `*` | 乘法 | `4 * 3 → 12` |
| `/` | 除法（精確分數） | `5 / 2 → 5/2` |
| `%` | 取餘 | `7 % 3 → 1` |
| `^` | 次方運算 | `2^3 → 8` |
| `!` | 階乘 | `5! → 120` |

### 向量/矩陣運算

```lamina
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];

print(v1 + v2);                 // 向量加法：[5, 7, 9]
print(dot(v1, v2));             // 點積：32
print(2 * v1);                  // 標量乘法：[2, 4, 6]
```

### 比較運算子

`==` `!=` `>` `<` `>=` `<=`

---

## 內建函式庫

### 數學函式

| 函式 | 說明 | 範例 |
|------|------|------|
| `sqrt(x)` | 平方根（精確無理數） | `sqrt(2) → √2` |
| `pi()` | 圓周率常數 | `pi() → π` |
| `e()` | 自然常數 | `e() → e` |
| `abs(x)` | 絕對值 | `abs(-5) → 5` |
| `sin(x)` | 正弦函式 | `sin(0) → 0` |
| `cos(x)` | 餘弦函式 | `cos(0) → 1` |
| `log(x)` | 自然對數 | `log(e()) → 1` |

### 向量/矩陣函式

| 函式 | 說明 | 範例 |
|------|------|------|
| `dot(v1, v2)` | 向量點積 | `dot([1,2], [3,4]) → 11` |
| `cross(v1, v2)` | 三維向量叉積 | `cross([1,0,0], [0,1,0]) → [0,0,1]` |
| `norm(v)` | 向量模長 | `norm([3,4]) → 5` |
| `det(m)` | 矩陣行列式 | `det([[1,2],[3,4]]) → -2` |

### 工具函式

| 函式 | 說明 | 範例 |
|------|------|------|
| `print(...)` | 輸出 | `print("x =", 42)` |
| `input(prompt)` | 取得輸入 | `input("Name: ")` |
| `size(x)` | 取得大小 | `size([1,2,3]) → 3` |
| `fraction(x)` | 小數轉分數 | `fraction(0.75) → 3/4` |
| `decimal(x)` | 分數轉小數 | `decimal(3/4) → 0.75` |
| `visit()` | 訪問 | `visit(a, 3, 0)`|
|`visit_by_str()`| 訪問 | `visit_by_str(b, "name")` |

### 字串處理函式

| 函式 | 說明 | 範例 |
|------|------|------|
| `string_concat(...)` | 串接多個字串 | `string_concat("abc", "abc", "abc") → "abcabcabc"` |
| `string_char_at(str, index)` | 取得字串指定位置的字元（回傳 Int） | `string_char_at("abc", 1) → 98` |
| `string_length(str)` | 取得字串長度 | `string_length("abc") → 3` |
| `string_find(str, start_index, sub_str)` | 從指定位置開始查詢子字串 | `string_find("abcAAA123", 0, "AAA") → 3` |
| `string_sub_string(str, start_index, len)` | 取得指定長度的子字串 | `string_sub_string("abcAAA123", 3, 3) → "AAA"` |
| `string_replace_by_index(str, start_index, sub_str)` | 從指定位置開始替換原字串 | `string_replace_by_index("abcAAA123", 3, "CCC") → "abcCCC123"` |

---

## 範例程式碼

### 精確數學運算

```lamina
// 精確分數運算
print("=== 精確分數運算 ===");
var fraction = 16 / 9;           // 精確分數 16/9
print("16/9 =", fraction);

// 多次循環保持精確性
var result = fraction;
var i = 1;
while (i <= 5) {
    result = result * 9 / 9;     // 應保持為 16/9
    print("第", i, "次循環:", result);
    i = i + 1;
}

// 精確無理數運算
print("\n=== 精確無理數運算 ===");
var root2 = sqrt(2);             // 精確的 √2
var root3 = sqrt(3);             // 精確的 √3
print("√2 =", root2);
print("√3 =", root3);
print("√2 + √3 =", root2 + root3);
print("√2 × √3 =", root2 * root3);
print("√2 × √2 =", root2 * root2);

// 精確常數
var pi_val = pi();
var e_val = e();
print("π =", pi_val);
print("e =", e_val);
print("π + e =", pi_val + e_val);
```

### 分數與小數轉換

```lamina
// 小數轉分數
print("=== 小數轉分數 ===");
print("0.5 =", fraction(0.5));      // 1/2
print("0.25 =", fraction(0.25));    // 1/4
print("0.75 =", fraction(0.75));    // 3/4
print("0.125 =", fraction(0.125));  // 1/8
print("0.625 =", fraction(0.625));  // 5/8

// 分數轉小數
print("\n=== 分數轉小數 ===");
print("1/2 =", decimal(1/2));       // 0.5
print("3/4 =", decimal(3/4));       // 0.75
print("1/3 =", decimal(1/3));       // 0.333333
print("2/3 =", decimal(2/3));       // 0.666667

// 精確計算的優勢
print("\n=== 精確計算 vs 浮點計算 ===");
var float_result = 0.1 + 0.2;
var exact_result = fraction(0.1) + fraction(0.2);
print("浮點計算: 0.1 + 0.2 =", float_result);
print("精確計算: 1/10 + 1/5 =", exact_result);
print("轉為小數:", decimal(exact_result));
```

### 解二次方程式

```lamina
func quadratic(a, b, c) {
    var discriminant = b^2 - 4*a*c;
    if (discriminant < 0) {
        print("複數根尚未支援");
        return null;
    } else {
        var root1 = (-b + sqrt(discriminant))/(2*a);
        var root2 = (-b - sqrt(discriminant))/(2*a);
        return [root1, root2];
    }
}

var roots = quadratic(1, -3, 2);
print("根:", roots);  // 輸出 [2, 1]
```

### 大整數階乘

```lamina
// 直接計算
bigint result = 25!;
print("25! =", result);

// 遞迴版本
func factorial_big(n) {
    if (n <= 1) return 1;
    return n * factorial_big(n - 1);
}

bigint fact10 = factorial_big(10);
print("10! =", fact10);
```

### 向量與矩陣運算

```lamina
// 向量運算
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];
var v3 = [1, 0, 0];
var v4 = [0, 1, 0];

print("向量加法:", v1 + v2);
print("點積:", dot(v1, v2));
print("叉積:", cross(v3, v4));
print("模長:", norm(v1));

// 矩陣運算
var matrix = [[1, 2], [3, 4]];
print("行列式:", det(matrix));
```

### 模組化程式設計

```lamina
// 引入數學工具模組
include "math_utils";

// 引入向量運算模組  
include "vectors";

// 使用模組中定義的函式
var result = advanced_calculation(10, 20);
var vec = create_vector(1, 2, 3);

// 顯示啟動畫面
include "splash";
```

---

## 設計原則

1. **簡潔性**：語法簡潔，符合數學表達習慣
2. **數學友好**：直接支援向量、矩陣及常用數學運算
3. **精確計算**：支援有理數及無理數的精確表示，避免精度遺失
4. **程序導向**：以函式組織程式碼，避免複雜物件模型
5. **圖靈完備**：支援無限循環、遞迴及任意計算
6. **使用者可設定**：遞迴深度限制、大整數型態等須使用者明確設定

---

## 未來擴充

- **複數支援**：實作複數型態及相關運算
- **符號微積分**：支援符號化微分與積分
- **語法糖**：支援分數字面量（如 `16/9`）及無理數字面量（如 `√5`）
- **並行運算**：支援多執行緒加速複雜運算
- **進階數學庫**：統計函式、數值分析等

---

## 錯誤處理與偵錯

### 堆疊追蹤支援

Lamina 提供完整的錯誤堆疊追蹤：

```text
Traceback (most recent call last):
  File "script.lm", line 12, in level1
  File "script.lm", line 8, in level2
  File "script.lm", line 4, in level3
RuntimeError: Undefined variable 'unknown_variable'
```

### 智能終端支援

- 自動色彩偵測：依終端能力自動啟用/停用色彩
- NO_COLOR 支援：遵循 NO_COLOR 環境變數標準
- 重新導向友好：輸出重新導向時自動停用色彩避免亂碼

---

## 運算子優先順序

`! > ^ > * / % > + - > == != > && ||`
