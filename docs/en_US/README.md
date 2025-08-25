# <img src="/assets/logo-icon.svg" width="10%"> Lamina 
<img src="/assets/logo.svg" width="100%">

<div align="right">
  <a href="../zh_TW/README.md">繁體中文</a> | <a href="/README.md">简体中文</a> | <strong>English</strong>
</div>
<br>

[![GitHub issues](https://img.shields.io/github/issues/lamina-dev/Lamina)](https://github.com/Lamina-dev/Lamina/issues)
[![GitHub stars](https://img.shields.io/github/stars/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/forks)
[![GitHub contributors](https://img.shields.io/github/contributors/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/graphs/contributors)
![GitHub last commit](https://img.shields.io/github/last-commit/lamina-dev/Lamina?style=flat)
[![License](https://img.shields.io/badge/license-GNUv3-blue.svg)](/LICENSE)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![Math](https://img.shields.io/badge/math-precise-green.svg)](#precise-mathematics)
[![QQ](https://img.shields.io/badge/QQ-Group-red?logo=qq&logoColor=white)](https://qm.qq.com/q/QwPXCgsJea)

## A procedural programming language focusing on precise mathematical computation

[Quick Start](#quick-start) • [Syntax Guide](#basic-syntax) • [Mathematical Features](#precise-mathematics) • [Sample Code](#sample-code) • [Compilation Guide](Compile.md) • [Contributing Guide](CONTRIBUTING.md) • [Wiki](https://github.com/lamina-dev/Lamina/wiki) • [Dynamic Library Plugin Development](PLUGIN_GUIDE.md)

---
## Contents

- [Overview](#overview)
- [Precise Mathematics](#precise-mathematics)
- [Quick Start](#quick-start)
- [Basic Syntax](#basic-syntax)
- [Data Types](#data-types)
- [Operators](#operators)
- [Built-in Functions](#built-in-functions)
- [Sample Code](#sample-code)
- [Design Principles](#design-principles)

---

## Overview

**Lamina** is a procedural language for mathematical computation, focusing on efficient solutions for mathematical problems and Turing completeness. Its goal is to provide concise, intuitive syntax while supporting complex mathematical operations.

### Key Features

- **Precise Mathematics**: Supports rational (fractional) and irrational (√, π, e) numbers with exact representation and calculation.
- **Precision Protection**: Avoids floating-point precision loss; repeated loop calculations remain exact.
- **Math-Friendly**: Native support for vector, matrix operations, and mathematical functions.
- **Recursion Support**: Configurable recursion depth limits.
- **Big Integers**: Arbitrary-precision integer computation.
- **Interactive REPL**: Supports interactive programming and script execution.
- **Smart Terminal**: Automatically detects terminal color support to avoid garbled output.
- **Stack Trace**: Complete error stack trace for easy debugging.

---

## Precise Mathematics

Lamina’s core advantage is its ability for precise mathematics, solving the floating-point precision loss problem found in traditional languages.

### Rational Numbers

- **Automatic Fraction Representation**: Division produces exact fractions.
- **Accurate Multiple Calculations**: Avoids accumulated errors.
- **Automatic Simplification**: Fractions are automatically reduced to simplest form.

```lamina
var fraction = 16 / 9;           // Result is 16/9, not 1.777...
var result = fraction;

// Multiple loop calculations remain exact
var i = 1;
while (i <= 10) {
    result = result * 9 / 9;     // Always remains 16/9
    i = i + 1;
}
```

### Irrational Numbers

- **Symbolic Representation**: √2, π, e remain as symbols.
- **Automatic Simplification**: √8 → 2√2, √(a²) → a
- **Exact Calculation**: Symbolic calculation avoids approximation errors

```lamina
var root2 = sqrt(2);             // Exact √2
var root8 = sqrt(8);             // Automatically simplified to 2√2
var pi_val = pi();               // Exact π
var result = root2 * root2;      // Exact result: 2
```

### Precision Comparison Example

```lamina
// Traditional floating-point (other languages)
// 0.1 + 0.2 = 0.30000000000000004

// Lamina precise calculation
var result = 1/10 + 2/10;        // Result: 3/10 (fully exact)
```

---

## Quick Start

### Compile and Run

```bash
# Initialize submodules (required for first build or after update)
git submodule update --init --recursive

# Compile
# Please refer to the compilation guide

# Interactive mode
./lamina

# Run script
./lamina script.lm
```

**Note**: Lamina uses the libuv library for networking, included as a Git submodule. Before the first build or after code updates, always run the submodule initialization command above.

### Simple Example

```lamina
// Precise mathematical calculation
print("Hello, Lamina!");
var fraction = 16 / 9;          // Precise fraction, not floating-point
print("16/9 =", fraction);      // Output: 16/9

var root = sqrt(2);             // Precise irrational number
print("√2 =", root);            // Output: √2
print("√2 × √2 =", root * root);// Output: 2 (exact result)

// Vector operations
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];
print("v1 + v2 =", v1 + v2);
print("v1 · v2 =", dot(v1, v2));

// Precise constants
var pi_val = pi();
print("π =", pi_val);           // Output: π (symbolic)
```

---

## Basic Syntax

### Statement Terminator

**Semicolon required**: All statements in Lamina must end with a semicolon `;`, improving parsing efficiency and code readability.

```lamina
var x = 10;           //  Correct
print(x);             //  Correct
bigint big = 100!;    //  Correct

var y = 20            //  Error: missing semicolon
print(y)              //  Error: missing semicolon
```

### Module System

Lamina supports modular programming, using the `include` statement to import files.

#### Include Statement

**Quotation marks required**: The `include` statement must use quotation marks for filenames, ensuring consistent and clear syntax.

```lamina
include "math_utils";     //  Correct: uses quotes
include "lib/vectors";    //  Correct: supports relative paths

include math_utils;       //  Error: must use quotes
```

#### Module Search Paths

Lamina searches for module files in order:

1. Current directory
2. `./` directory
3. `./include/` directory

If the filename lacks the `.lm` extension, it will be added automatically.

#### Built-in Modules

Some special built-in modules are provided:

- `include "splash";` - Shows Lamina splash screen
- `include "them";` - Shows developer info and acknowledgments

### Keywords

The following are reserved words, not allowed as variable or function names:

```plaintext
var func if else while for return break continue
print true false null include define bigint input
```

### Comments

```lamina
// Single-line comment
/* Block comment */
```

### Variable Declaration

```lamina
var x = 10;
var name = "Alice";
var arr = [1, 2, 3];
bigint large_num = 999999999999;  // Big integer must be explicitly declared
bigint factorial_result = 25!;    // Can assign from calculation result
```

---

## Data Types

### Basic Types

| Type      | Description                      | Example                        |
|-----------|----------------------------------|--------------------------------|
| `int`     | Integer                          | `42`, `-10`                    |
| `float`   | Floating-point                   | `3.14`, `-0.5`                 |
| `rational`| Rational (exact fraction)        | `16/9`, `1/3`                  |
| `irrational`| Irrational (exact symbol)      | `√2`, `π`, `e`                 |
| `bool`    | Boolean                         | `true`, `false`                |
| `string`  | String                          | `"Hello"`, `"world"`           |
| `null`    | Null                            | `null`                         |
| `bigint`  | Big integer (explicit)          | `bigint x = 999999;`, `bigint y = 30!;` |

### Composite Types

```lamina
// Array
var arr = [1, 2, 3];
var matrix = [[1,2], [3,4]];     // 2x2 matrix

// Function
func add(a, b) { 
    return a + b; 
}
```

---

## Operators

### Arithmetic Operators

| Operator | Function         | Example                |
|----------|------------------|------------------------|
| `+`      | Addition         | `2 + 3 → 5`            |
| `-`      | Subtraction      | `5 - 2 → 3`            |
| `*`      | Multiplication   | `4 * 3 → 12`           |
| `/`      | Division (exact) | `5 / 2 → 5/2`          |
| `%`      | Modulo           | `7 % 3 → 1`            |
| `^`      | Power            | `2^3 → 8`              |
| `!`      | Factorial        | `5! → 120`             |

### Vector/Matrix Operations

```lamina
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];

print(v1 + v2);                 // Vector addition: [5, 7, 9]
print(dot(v1, v2));             // Dot product: 32
print(2 * v1);                  // Scalar multiplication: [2, 4, 6]
```

### Comparison Operators

`==` `!=` `>` `<` `>=` `<=`

---

## Built-in Functions

### Mathematical Functions

| Function    | Description                 | Example            |
|-------------|----------------------------|--------------------|
| `sqrt(x)`   | Square root (exact)        | `sqrt(2) → √2`     |
| `pi()`      | Pi constant                | `pi() → π`         |
| `e()`       | Euler constant             | `e() → e`          |
| `abs(x)`    | Absolute value             | `abs(-5) → 5`      |
| `sin(x)`    | Sine function              | `sin(0) → 0`       |
| `cos(x)`    | Cosine function            | `cos(0) → 1`       |
| `log(x)`    | Natural logarithm          | `log(e()) → 1`     |

### Vector/Matrix Functions

| Function         | Description           | Example                          |
|------------------|----------------------|----------------------------------|
| `dot(v1, v2)`    | Vector dot product   | `dot([1,2], [3,4]) → 11`         |
| `cross(v1, v2)`  | 3D vector cross      | `cross([1,0,0], [0,1,0]) → [0,0,1]` |
| `norm(v)`        | Vector magnitude     | `norm([3,4]) → 5`                |
| `det(m)`         | Matrix determinant   | `det([[1,2],[3,4]]) → -2`        |

### Utility Functions

| Function           | Description         | Example                        |
|--------------------|--------------------|--------------------------------|
| `print(...)`       | Print output       | `print("x =", 42)`             |
| `input(prompt)`    | Get input          | `input("Name: ")`              |
| `size(x)`          | Get size           | `size([1,2,3]) → 3`            |
| `fraction(x)`      | Decimal to fraction| `fraction(0.75) → 3/4`         |
| `decimal(x)`       | Fraction to decimal| `decimal(3/4) → 0.75`          |
| `visit()`          | Visit              | `visit(a, 3, 0)`               |
| `visit_by_str()`   | Visit by string    | `visit_by_str(b, "name")`      |

### String Processing Functions

| Function                    | Description                      | Example                                   |
|-----------------------------|----------------------------------|-------------------------------------------|
| `string_concat(...)`        | Concatenate strings              | `string_concat("abc", "abc", "abc") → "abcabcabc"` |
| `string_char_at(str, index)`| Get char code at index           | `string_char_at("abc", 1) → 98`           |
| `string_length(str)`        | String length                    | `string_length("abc") → 3`                |
| `string_find(str, start_index, sub_str)` | Find substring       | `string_find("abcAAA123", 0, "AAA") → 3`  |
| `string_sub_string(str, start_index, len)` | Substring         | `string_sub_string("abcAAA123", 3, 3) → "AAA"` |
| `string_replace_by_index(str, start_index, sub_str)` | Replace by index | `string_replace_by_index("abcAAA123", 3, "CCC") → "abcCCC123"` |

---

## Sample Code

### Precise Math Calculation

```lamina
// Precise fraction calculation
print("=== Precise Fraction Calculation ===");
var fraction = 16 / 9;           // Precise fraction 16/9
print("16/9 =", fraction);

// Multiple loops, remain exact
var result = fraction;
var i = 1;
while (i <= 5) {
    result = result * 9 / 9;     // Should remain 16/9
    print("Loop", i, ":", result);
    i = i + 1;
}

// Precise irrational calculation
print("\n=== Precise Irrational Calculation ===");
var root2 = sqrt(2);             // Exact √2
var root3 = sqrt(3);             // Exact √3
print("√2 =", root2);
print("√3 =", root3);
print("√2 + √3 =", root2 + root3);
print("√2 × √3 =", root2 * root3);
print("√2 × √2 =", root2 * root2);

// Precise constants
var pi_val = pi();
var e_val = e();
print("π =", pi_val);
print("e =", e_val);
print("π + e =", pi_val + e_val);
```

### Fraction and Decimal Conversion

```lamina
// Decimal to fraction
print("=== Decimal to Fraction ===");
print("0.5 =", fraction(0.5));      // 1/2
print("0.25 =", fraction(0.25));    // 1/4
print("0.75 =", fraction(0.75));    // 3/4
print("0.125 =", fraction(0.125));  // 1/8
print("0.625 =", fraction(0.625));  // 5/8

// Fraction to decimal
print("\n=== Fraction to Decimal ===");
print("1/2 =", decimal(1/2));       // 0.5
print("3/4 =", decimal(3/4));       // 0.75
print("1/3 =", decimal(1/3));       // 0.333333
print("2/3 =", decimal(2/3));       // 0.666667

// Advantage of precise calculation
print("\n=== Precise vs. Floating Point ===");
var float_result = 0.1 + 0.2;
var exact_result = fraction(0.1) + fraction(0.2);
print("Floating point: 0.1 + 0.2 =", float_result);
print("Precise: 1/10 + 1/5 =", exact_result);
print("As decimal:", decimal(exact_result));
```

### Solving Quadratic Equations

```lamina
func quadratic(a, b, c) {
    var discriminant = b^2 - 4*a*c;
    if (discriminant < 0) {
        print("Complex roots not supported");
        return null;
    } else {
        var root1 = (-b + sqrt(discriminant))/(2*a);
        var root2 = (-b - sqrt(discriminant))/(2*a);
        return [root1, root2];
    }
}

var roots = quadratic(1, -3, 2);
print("Roots:", roots);  // Output [2, 1]
```

### Big Integer Factorial

```lamina
// Direct calculation
bigint result = 25!;
print("25! =", result);

// Recursive version
func factorial_big(n) {
    if (n <= 1) return 1;
    return n * factorial_big(n - 1);
}

bigint fact10 = factorial_big(10);
print("10! =", fact10);
```

### Vector and Matrix Operations

```lamina
// Vector operations
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];
var v3 = [1, 0, 0];
var v4 = [0, 1, 0];

print("Vector addition:", v1 + v2);
print("Dot product:", dot(v1, v2));
print("Cross product:", cross(v3, v4));
print("Magnitude:", norm(v1));

// Matrix operations
var matrix = [[1, 2], [3, 4]];
print("Determinant:", det(matrix));
```

### Modular Programming

```lamina
// Include math utility module
include "math_utils";

// Include vector calculation module  
include "vectors";

// Use functions defined in module
var result = advanced_calculation(10, 20);
var vec = create_vector(1, 2, 3);

// Show splash screen
include "splash";
```

---

## Design Principles

1. **Simplicity**: Concise syntax, follows mathematical conventions
2. **Math-Friendly**: Direct support for vectors, matrices, and common math operations
3. **Precise Calculation**: Exact rational and irrational representation, avoid precision loss
4. **Procedural**: Organize code with functions, no complex object models
5. **Turing Complete**: Supports infinite loops, recursion, arbitrary computation
6. **User Configurable**: Recursion depth, big integer types must be explicitly set

---

## Future Extensions

- **Complex Number Support**: Implement complex types and operations
- **Symbolic Calculus**: Support symbolic differentiation and integration
- **Syntax Sugar**: Support fraction literals (e.g., `16/9`) and irrational literals (e.g., `√5`)
- **Parallel Computing**: Multi-threading for faster computation
- **Advanced Math Libraries**: Statistics, numerical analysis, etc.

---

## Error Handling & Debugging

### Stack Trace Support

Lamina provides full error stack traces:

```text
Traceback (most recent call last):
  File "script.lm", line 12, in level1
  File "script.lm", line 8, in level2
  File "script.lm", line 4, in level3
RuntimeError: Undefined variable 'unknown_variable'
```

### Smart Terminal Support

- Automatic color detection: Enables/disables color based on terminal capabilities
- NO_COLOR support: Follows the NO_COLOR environment variable standard
- Redirection friendly: Disables color when output is redirected to avoid garbled text

---

## Operator Precedence

`! > ^ > * / % > + - > == != > && ||`
