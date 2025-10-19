# Lamina ToDo List

## 符号含义
⚠️(在第二个RC发布前一定要完成)

🟡建议早日完成(以完备基本的语法特性)

🟢实现时间较不重要

🟧 已经实现但功能不完善


<b> 注意 ToDo list 的每一项需要标注状态 (已完成) (未完成) (正在做)
</b>

## 库方面
- [ ] IO库（正在做）<br>
     备注：`已在extensions/standard/io.cpp`有预留

- [ ] bit库（正在做）<br>     备注：

- [ ] 🟡 数学图像库（未完成）<br>     备注：

- [ ] ⚠️ 集合库（正在做）<br>     备注：

- [ ] 线程库（正在做）<br>     备注：

- [ ] console库（未完成）<br>
     备注：`已在extensions/standard/io.cpp`有预留

## 语法方面
- [ ] 🟡 结构体属性赋值语句 x.y = v（未完成）<br>
     备注：

- [ ] 🟡 列表项赋值语句 a[i] = v（未完成）<br>
     备注：

- [ ] 🟡 三元表达式（未完成）<br>
     备注：
    方案一: `if cond : expr else expr`

    方案二: `cond if expr else expr`

    方案三:  `cond ? expr ! expr`

- [ ] 🟡 类型注释及类型系统（未完成）<br>
     备注：

- [ ] 🟡 when语句（未完成）<br>
     备注：

- [ ] 🟡 is not or in not int 运算符（未完成）<br>
     备注：

- [ ] 🟡 管道运算符（未完成）<br>
     备注：

## 内置函数方面
- [x] exit(n)（已完成）<br>
     备注：

- [ ] eval(code)（未完成）<br>
     备注：

- [x] xpcall(fn)（已完成）<br>
     备注：

- [x] locals()（已完成）<br>
     备注：

- [x] globals()（已完成）<br>
     备注：

- [x] vars()（已完成）<br>
     备注：

- [ ] 🟡 help(key)（未完成）<br>
     备注：

- [ ] 🟡 open(name, pattern, encoding)（未完成）<br>
     备注：

- [ ] 🟧 find(arr, lambda, times)（未完成）<br>
    备注：

- [ ] 🟧 replace(arr, lambda)（未完成）<br>
     备注：

- [x] same(a, b)（已完成）<br>
     备注：

## 类型方面
- [ ] ⚠️ LmInt（未完成）<br>
     备注：

     优化现在Bigint效率，

     然后把Bigint改名为LmInt, 

     成为lamina唯一整数类型

- [ ] ⚠️ LmDec（未完成）<br>
     备注：

     就是无限精度的安全小数， 

     但如果用户要从有理数转到安全小数，

     需要设置小数位数(以防止无限循环小数和不循环小数）


- [ ] ⚠️ LmComplex（未完成）<br>
     备注：复数类型

- [ ] ⚠️ LmList（正在做）<br>
     备注：链表类型

     已完成基本实线，正在debug及完善

     无具体引用，欢迎后人补充，文件位置

     底层模板类：`interpreter/base_list.hpp`

     继承套用：`interpreter/list.hpp`

## 其他
- [ ] ⚠️ 测试新的c++ module loader（未完成）<br>
     备注：

- [ ] ⚠️ 修复repl input 的 bug（未完成）<br>
     备注：

- [ ] ⚠️ 报错系统使用 err_reporter 和 src_manger（未完成）<br>
     备注：
