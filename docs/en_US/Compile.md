## Compile

<div align="right">
  <a href="../zh_TW/Compile.md">繁體中文</a> | <a href="../zh_CN/Compile.md">简体中文</a> | <strong>English</strong>
</div>
<br>

Linux:

```shell
mkdir build
cd build
cmake ..
make
```

Windows With MinGW:

```shell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

Windows With MSVC:

```shell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
msbuild ALL_BUILD.vcxproj
```

Use xmake

```shell
xmake
```

# WARNING: If you cannot compile it with cmake in MacOS, try to use Unix makefiles rather than Xcode
