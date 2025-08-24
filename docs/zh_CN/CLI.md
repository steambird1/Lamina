### Lamina支持执行这些指令
```bash
lamina <>          ||  start repl
lamina <path>      ||  run the file at path
lamina run <path>  ||  run the file at path
lamina version     ||  show the version of lamina
lamina repl        ||  start repl
lamina help        ||  show help
```

### 开发环境如何执行?
**第一步**
如果你是mingw gcc 套件
且编译时未使用静态链接，需将以下文件复制到 cmake-build-debug 同级目录：

```plaintext
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

这些文件位于 MinGW 的bin目录（如C:\mingw64\bin）。

**第二步** 切换到`cmake-build-debug`目录·
```bash
 cd .\cmake-build-debug
```

**第三步** 尝试
```bash
.\Lamina.exe version
```
如果有输出，就成功了

