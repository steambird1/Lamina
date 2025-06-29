# Lamina Interpreter - 项目状态

这个页面###  Release (自动发布)
- **触发条件**###  Release (自动发布)
- **触发条件**: Git 标签推送 (v*.*.*)，手动触发
- **运行平台**: Ubuntu, Windows, macOS
- **主要任务**:
  - 构建优化版本
  - 创建发布包 (tar.gz, zip)  
  - **自动创建 GitHub Release**
  - 上传预编译二进制文件
  - 生成发布说明和下载链接

** Release 功能特点**:
-  自动检测版本标签
-  多平台二进制文件打包
-  自动生成 Release Notes
-  包含使用说明和特性介绍
-  支持手动触发发布

** 如何发布新版本**:
```bash
# 1. 确保代码已提交并推送
git add .
git commit -m "feat: prepare for v1.0.0 release"
git push

# 2. 创建并推送版本标签
git tag v1.0.0
git push origin v1.0.0

# 3. GitHub Actions 会自动：
#    - 在 3 个平台上构建
#    - 创建 GitHub Release
#    - 上传二进制文件
#    - 生成下载链接
```

###  Release 包内容

每个 Release 包都包含：
-  编译好的 Lamina 解释器
-  示例程序文件 (`examples/`)
-  README.md 文档
-  LICENSE 许可证文件
-  VERSION.txt 版本信息

| 平台 | 文件格式 | 文件名示例 |
|------|----------|------------|
| **Linux** | tar.gz | `lamina-linux-x64.tar.gz` |
| **Windows** | zip | `lamina-windows-x64.zip` |
| **macOS** | tar.gz | `lamina-macos-x64.tar.gz` |推送 (v*.*.*)，手动触发
- **运行平台**: Ubuntu, Windows, macOS
- **主要任务**:
  - 构建优化版本
  - 创建发布包 (tar.gz, zip)  
  - **自动创建 GitHub Release**
  - 上传预编译二进制文件
  - 生成发布说明和下载链接

** Release 功能特点**:
-  自动检测版本标签
-  多平台二进制文件打包
-  自动生成 Release Notes
-  包含使用说明和特性介绍
-  支持手动触发发布

** 如何发布新版本**:
```bash
# 1. 确保代码已提交并推送
git add .
git commit -m "feat: prepare for v1.0.0 release"
git push

# 2. 创建并推送版本标签
git tag v1.0.0
git push origin v1.0.0

# 3. GitHub Actions 会自动：
#    - 在 3 个平台上构建
#    - 创建 GitHub Release
#    - 上传二进制文件
#    - 生成下载链接
```a 解释器项目的自动化构建和测试状态。

##  自动化工作流状态

| 工作流 | 状态 | 描述 |
|--------|------|------|
| **CI** | [![CI](https://github.com/Ziyang-bai/Lamina/workflows/CI/badge.svg)](https://github.com/Ziyang-bai/Lamina/actions/workflows/ci.yml) | 持续集成 - 基本编译和测试 |
| **Build** | [![Build](https://github.com/Ziyang-bai/Lamina/workflows/Build%20Lamina%20Interpreter/badge.svg)](https://github.com/Ziyang-bai/Lamina/actions/workflows/build.yml) | 多平台构建测试 |
| **Release** | [![Release](https://github.com/Ziyang-bai/Lamina/workflows/Release/badge.svg)](https://github.com/Ziyang-bai/Lamina/actions/workflows/release.yml) | 自动发布构建 |
| **Code Quality** | [![Code Quality](https://github.com/Ziyang-bai/Lamina/workflows/Code%20Quality/badge.svg)](https://github.com/Ziyang-bai/Lamina/actions/workflows/quality.yml) | 代码质量检查 |

##  工作流详情

###  CI (持续集成)
- **触发条件**: 每次 push 和 pull request
- **运行平台**: Ubuntu Latest
- **主要任务**:
  - 编译 Lamina 解释器
  - 运行基本功能测试
  - 测试 include 语句语法强制检查
  - 运行示例程序
  - 内存泄漏检测 (Valgrind)

###  Build (多平台构建)
- **触发条件**: push 到 main/develop 分支，PR 到 main/develop
- **运行平台**: Ubuntu, Windows, macOS
- **编译器**: GCC, Clang++, MSVC
- **主要任务**:
  - 在多个平台和编译器上构建
  - 基本功能测试
  - 生成构建产物
  - 打包发布文件

###  Release (自动发布)
- **触发条件**: Git 标签推送 (v*.*.*)，手动触发
- **运行平台**: Ubuntu, Windows, macOS
- **主要任务**:
  - 构建优化版本
  - 创建发布包 (tar.gz, zip)
  - 自动创建 GitHub Release
  - 上传预编译二进制文件

###  Code Quality (代码质量)
- **触发条件**: push 到 main/develop 分支，PR 到 main/develop
- **运行平台**: Ubuntu Latest
- **主要任务**:
  - 代码格式检查 (clang-format)
  - 静态代码分析 (cppcheck)
  - 常见问题检查
  - 文档完整性检查
  - 基础安全扫描

##  自动化测试覆盖

###  功能测试
- [x] Hello World 输出
- [x] 基本算术运算
- [x] 精确分数计算
- [x] 函数定义和调用
- [x] Include 语句语法检查
- [x] 示例程序执行

###  质量保证
- [x] 多平台编译兼容性
- [x] 多编译器支持
- [x] 内存泄漏检测
- [x] 静态代码分析
- [x] 代码格式规范
- [x] 安全漏洞扫描

###  文档检查
- [x] README 完整性
- [x] 许可证文件存在
- [x] 示例文件可用性
- [x] 关键章节覆盖

##  工作流执行频率

| 工作流 | 触发频率 | 平均耗时 |
|--------|----------|----------|
| CI | 每次提交 | ~3-5 分钟 |
| Build | 重要分支提交 | ~10-15 分钟 |
| Release | 版本标签 | ~15-20 分钟 |
| Code Quality | 重要分支提交 | ~5-8 分钟 |

##  项目统计

- **支持平台**: Linux, Windows, macOS
- **编译器支持**: GCC, Clang++, MSVC
- **自动化测试**: 6+ 基本功能测试
- **代码质量工具**: clang-format, cppcheck, valgrind
- **发布自动化**: 完全自动化的版本发布流程

##  维护说明

### 更新工作流
所有工作流文件位于 `.github/workflows/` 目录：
- `ci.yml` - 持续集成
- `build.yml` - 多平台构建
- `release.yml` - 自动发布
- `quality.yml` - 代码质量检查
- `badges.yml` - README 徽章更新

### 新增测试
要添加新的测试用例，请在 `ci.yml` 的测试步骤中添加相应的测试脚本。

### 发布新版本
** 自动化 Release 流程**:

1. **准备发布**:
   ```bash
   # 确保所有更改已提交
   git add .
   git commit -m "feat: prepare for v1.0.0 release"
   git push origin main
   ```

2. **创建版本标签**:
   ```bash
   # 创建带注释的标签 (推荐)
   git tag -a v1.0.0 -m "Release version 1.0.0: 添加 include 引号强制检查"
   
   # 或创建轻量标签
   git tag v1.0.0
   
   # 推送标签触发 Release
   git push origin v1.0.0
   ```

3. **手动触发** (可选):
   - 访问 GitHub 仓库的 Actions 页面
   - 选择 "Release" 工作流
   - 点击 "Run workflow" 按钮

4. **Release 自动完成**:
   -  多平台编译 (Linux/Windows/macOS)
   -  创建压缩包 (tar.gz/zip)
   -  生成 GitHub Release 页面
   -  上传二进制文件和文档
   -  添加详细的发布说明

** Release Notes 包含内容**:
- 🎉 版本介绍和新特性
- 📦 各平台下载链接
-  使用方法说明
- ⚡ 主要功能特点
- 📚 文档和示例链接
- 🔄 更新和修复说明

**⚠️ 注意事项**:
- 版本标签必须以 `v` 开头 (如 `v1.0.0`, `v2.1.3`)
- Release 工作流只在推送标签时触发
- 确保所有 CI 测试通过后再发布
- 每个标签只会触发一次 Release 构建

---

 **最后更新**: 此状态页面会随着项目发展持续更新。
