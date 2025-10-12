# <img src="./assets/logo-icon.svg" width="10%"> This is Lamina version 1.2.0 (Big Refactor) （debug) 

<img src="./assets/logo.svg" width="100%">

<div align="right">
    <strong> 简体中文</strong> | <a href="/docs/en_US/README.md"> English</a>
</div>
<br>

[![GitHub issues](https://img.shields.io/github/issues/lamina-dev/Lamina)](https://github.com/Lamina-dev/Lamina/issues)
[![GitHub stars](https://img.shields.io/github/stars/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/forks) 
[![GitHub contributors](https://img.shields.io/github/contributors/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/graphs/contributors)
![GitHub last commit](https://img.shields.io/github/last-commit/lamina-dev/Lamina?style=flat)
[![License](https://img.shields.io/badge/license-LGPLv2.1-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![Math](https://img.shields.io/badge/math-precise-green.svg)](#精确数学特性)
[![QQ](https://img.shields.io/badge/QQ-%E4%BA%A4%E6%B5%81%E7%BE%A4-red?logo=qq&logoColor=white)](https://qm.qq.com/q/QwPXCgsJea)


## 一种专注于精确数学计算的以面向过程为主体的编程语言

[语法指南](docs/zh_CN/wiki.md) • [示例代码](/examples) • [编译指南](/docs/zh_CN/Compile.md) • [贡献指南](/docs/zh_CN/CONTRIBUTING.md) • [Wiki](https://wiki.lm-lang.org) • [动态库插件开发](/docs/zh_CN/PLUGIN_GUIDE.md) • [ToDo list](TODO.md)

## 目录
- [语法指南](docs/zh_CN/wiki.md)
- [数学特性](#精确数学特性)
- [Lamina v1.2.0新特性](#Lamina-v1.2.0新特性) 
- [示例代码](/examples)

## 精确数学特性
1. **精确数学计算**：从底层解决浮点数精度丢失问题，支持有理数（分数）和无理数（√、π、e）的符号化存储与运算，多次循环运算仍保持精确。
2. **语法简洁直观**：支持自动补充分号、省略if/while语句的圆括号、无参函数简写等，降低代码冗余，符合数学表达习惯。
3. **原生数学友好**：无需第三方库，直接支持向量、矩阵运算、大整数阶乘等数学操作，满足复杂数学问题需求。
4. **友好开发体验**：交互式REPL支持关键字高亮、自动补齐，提供完整错误栈追踪，便于调试；智能终端自动适配色彩，避免乱码。
5. **模块化设计**：通过`include`语句引入外部模块，支持`::`命名空间访问符，实现代码复用与隔离。
6. **灵活数据类型**：涵盖精确数值类型（rational/irrational）、复合类型（数组/矩阵/结构体）及匿名函数，适配多样开发场景。

<br>

## Lamina-v1.2.0新特性
**Lamina v1.2.0 重磅更新：架构重构焕新，语法体验再升级**
经过深度重构的 Lamina v1.2.0 正式发布！本次更新不仅优化了底层架构，更带来了更灵活的语法、更强大的功能，以及更友好的开发体验，让代码编写效率大幅提升。

### 一、语法更灵活，编写更自由
- [x] **语法糖加持**：if/while 语句表达式可省略圆括号，无参函数定义无需写空括号，代码更简洁。
- [x] **自动补全细节**：支持自动添加分号，减少手动输入失误；通过 `\` 实现续行功能，长代码排版更清晰。
- [x] **匿名类型支持**：新增匿名结构体与匿名函数声明，灵活应对临时数据与逻辑场景。

### 二、功能更强大，覆盖更多场景
- [x] **模块与引用升级**：更新 include 语句并引入模块系统，搭配 `::` 命名空间访问运算符，项目管理更规范。
- [x] **数组能力增强**：支持 `arr[subscript]` 运算符(结构体也支持该运算符)，访问子项更方便。
- [x] **结构体能力增强**：支持 `.` 运算符访问成员，同时新增结构体深拷贝功能，数据处理更安全。
- [x] **内置能力扩充**：builtins 系统更新，新增 `typeof`/`find`/`replace`/`foreach` 等实用函数，还可通过 `locals`/`globals` 查看变量表，减少重复编码。

### 三、开发体验优化，调试更高效
- [x] **REPL 全面升级**：支持直接打印表达式结果、无颜色输出模式，同时新增关键字高亮与自动补齐功能，交互式开发更流畅。
- [x] **底层架构重构**：重构 Parser 与 AST 模块，将部分内置函数迁移至内置库，提升代码可维护性与运行效率。
- [x] **C++ 生态联动**：完全重构 C++ 模块加载器与扩展格式，让 Lamina 与 C++ 生态的协作更顺畅。

---
    
感谢所有参与开发的工作者和用户

<br>

如果您想为lamina贡献代码

您可以看看 [ToDo list](TODO.md)