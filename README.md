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

[语法指南](docs/zh_CN/wiki.md) • [数学特性](#精确数学特性) • [示例代码](/examples) • [编译指南](/docs/zh_CN/Compile.md) • [贡献指南](/docs/zh_CN/CONTRIBUTING.md) • [Wiki](https://wiki.lm-lang.org) • [动态库插件开发](/docs/zh_CN/PLUGIN_GUIDE.md) • [ToDo list](TODO.md)

- [语法指南](docs/zh_CN/wiki.md)

## 精确数学特性
1. **精确数学计算**：从底层解决浮点数精度丢失问题，支持有理数（分数）和无理数（√、π、e）的符号化存储与运算，多次循环运算仍保持精确。
2. **语法简洁直观**：支持自动补充分号、省略if/while语句的圆括号、无参函数简写等，降低代码冗余，符合数学表达习惯。
3. **原生数学友好**：无需第三方库，直接支持向量、矩阵运算、大整数阶乘等数学操作，满足复杂数学问题需求。
4. **友好开发体验**：交互式REPL支持关键字高亮、自动补齐，提供完整错误栈追踪，便于调试；智能终端自动适配色彩，避免乱码。
5. **模块化设计**：通过`include`语句引入外部模块，支持`::`命名空间访问符，实现代码复用与隔离。
6. **灵活数据类型**：涵盖精确数值类型（rational/irrational）、复合类型（数组/矩阵/结构体）及匿名函数，适配多样开发场景。

<br>

<h2>🎉Lamina v1.2.0(Big Refactor) 有什么新功能 ?</h2>

<ul>
<li>自动添加分号</li>
<li>if, while语句表达式部分可以省略圆括号</li>
<li>定义没有参数的函数，可以省略圆括号</li>
<li>匿名结构体声明</li>
<li>匿名函数声明</li>
<li>include语句更新+模块系统</li>
<li>builtins系统更新</li>
<li>.运算符：结构体访问成员运算符</li>
<li>::运算符：命名空间访问成员运算符</li>
<li>\ 续行功能</li>
<li>结构体深拷贝</li>
<li>重构 Parser, ast</li>
<li>将部分内置函数移到内置库</li>
<li>c++ module loader更新</li>
<li>c++扩展格式更新</li>
<li>新增函数 typeof、find、replace、foreach、copy、map、exit、tostring</li>
<li>新增函数 locals、globals</li>
<li>loop语句</li>
<li>repl直接打印表达式</li>
<li>repl支持不带颜色的输出</li>
<li>repl关键字高亮及自动补齐</li>
</ul>

<b>延迟到下一个版本的特性</b>
<ul>
<li>数学区间相关扩展</li>
<li>set 集合类型</li>
<li>取消int类型, 取消bigint语句, Bigint更名为lmInt, 成为唯一整数类型并优化它的性能,然后在有理数/无理数/虚数/小数的数字部分都使用lmInt </li>
<li>Decimal类型</li>
<li>虚数功能</li>
<li>array[index] = item setitem语法</li>
<li>struct.item = value setmember语法</li>
<li>线程库(debug)</li>
<li>带上下文的报错系统</li>
<li>多语言支持</li>
</ul>
</details>

---

<br>
    
感谢所有参与开发的工作者和用户

<br>

如果您想为lamina贡献代码

您可以看看 [ToDo list](TODO.md)