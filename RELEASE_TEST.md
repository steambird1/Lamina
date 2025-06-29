#  GitHub Actions Release 测试指南

这个文档展示如何测试 Lamina 项目的自动化 Release 功能。

##  测试步骤

### 1. 准备测试环境

```bash
# 确保在主分支且代码最新
git checkout main
git pull origin main

# 检查当前状态
git status
```

### 2. 创建测试 Release

```bash
# 方案 A: 创建测试版本标签
git tag v0.1.0-test -m "Test release for GitHub Actions"
git push origin v0.1.0-test

# 方案 B: 创建正式版本标签  
git tag v1.0.0 -m "First stable release with include syntax enforcement"
git push origin v1.0.0
```

### 3. 监控 Release 过程

1. **查看 Actions 运行状态**:
   - 访问: `https://github.com/Ziyang-bai/Lamina/actions`
   - 查找 "Release" 工作流运行

2. **预期的构建流程**:
   ```
    build-and-release (ubuntu-latest)
    build-and-release (windows-latest)  
    build-and-release (macos-latest)
    create-release
   ```

3. **预期的构建时间**: 约 15-20 分钟

### 4. 验证 Release 结果

**检查 GitHub Release 页面**:
- 访问: `https://github.com/Ziyang-bai/Lamina/releases`
- 验证以下内容:

 **Release 标题**: "Lamina v1.0.0"
 **发布说明**: 包含中文介绍和使用方法
 **下载文件**:
   - `lamina-linux-x64.tar.gz`
   - `lamina-windows-x64.zip`
   - `lamina-macos-x64.tar.gz`

### 5. 测试下载的二进制文件

```bash
# Linux/macOS
wget https://github.com/Ziyang-bai/Lamina/releases/download/v1.0.0/lamina-linux-x64.tar.gz
tar -xzf lamina-linux-x64.tar.gz
cd lamina-linux-x64/
./lamina

# Windows (PowerShell)
Invoke-WebRequest -Uri "https://github.com/Ziyang-bai/Lamina/releases/download/v1.0.0/lamina-windows-x64.zip" -OutFile "lamina-windows-x64.zip"
Expand-Archive lamina-windows-x64.zip
cd lamina-windows-x64/
./lamina.exe
```

### 6. 功能验证测试

创建测试文件 `test.lm`:
```lamina
print("Hello from Lamina!");
var x = 16 / 9;
print("Precise fraction: 16/9 =", x);

// 测试 include 语法强制检查
include "nonexistent";  // 应该显示模块加载错误
```

运行测试:
```bash
./lamina test.lm
```

**期望输出**:
```
Hello from Lamina!
Precise fraction: 16/9 = 16/9
Error: Cannot load module 'nonexistent'
...
```

##  故障排除

### 常见问题

**Q: Release 工作流没有触发？**
A: 检查标签格式，必须以 `v` 开头 (如 `v1.0.0`)

**Q: 构建失败？**
A: 查看 Actions 日志，通常是编译错误或依赖问题

**Q: Release 页面没有文件？**
A: 检查 `create-release` 作业的日志，可能是文件路径问题

**Q: 下载的文件无法运行？**
A: 确认平台匹配，给文件添加执行权限: `chmod +x lamina`

### 调试命令

```bash
# 查看所有标签
git tag -l

# 查看标签详情
git show v1.0.0

# 删除错误的标签 (本地)
git tag -d v1.0.0

# 删除错误的标签 (远程)
git push origin --delete v1.0.0

# 重新创建标签
git tag v1.0.0 -m "Corrected release"
git push origin v1.0.0
```

##  成功指标

Release 测试成功的标志:
-  所有平台构建成功 (3/3)
-  GitHub Release 页面正常创建
-  所有下载文件可用且功能正常
-  版本信息正确显示
-  Release Notes 格式正确
-  包含所有必要文件 (可执行文件、文档、示例)

---

**注意**: 测试完成后记得删除测试标签以保持仓库整洁：
```bash
git tag -d v0.1.0-test
git push origin --delete v0.1.0-test
```
