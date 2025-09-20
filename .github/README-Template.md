# GitHub Templates 使用指南

本目录包含了Lamina项目的GitHub模板文件，旨在标准化issue提交和Pull Request流程。

## 📂 文件结构

```
.github/
├── ISSUE_TEMPLATE/          # Issue模板目录
│   ├── bug_report.yml       # Bug报告模板
│   ├── feature_request.yml  # 功能请求模板
│   ├── documentation.yml    # 文档改进模板
│   ├── help_wanted.yml      # 问题求助模板
│   └── config.yml          # Issue配置文件
├── workflows/              # GitHub Actions工作流
│   ├── label.yml           # 自动标签工作流
│   ├── pr_validation.yml   # PR验证工作流
│   └── issue_management.yml # Issue管理工作流
├── labeler.yml             # 自动标签配置
├── pull_request_template.md # PR模板
└── README.md              # 本文件
```

## 🎯 模板功能

### Issue 模板

1. **🐛 Bug报告** (`bug_report.yml`)
   - 标准化bug报告流程
   - 收集环境信息、重现步骤等关键信息
   - 自动添加 `bug` 标签

2. **✨ 功能请求** (`feature_request.yml`)
   - 收集新功能需求
   - 评估功能的优先级和影响
   - 自动添加 `enhancement` 标签

3. **📚 文档改进** (`documentation.yml`)
   - 收集文档改进建议
   - 针对不同类型的文档
   - 自动添加 `documentation` 标签

4. **❓ 问题求助** (`help_wanted.yml`)
   - 为用户提供帮助渠道
   - 收集使用问题和疑问
   - 自动添加 `question` 标签

### Pull Request 模板

- **全面的检查清单**：确保代码质量和完整性
- **变更类型分类**：便于维护者快速了解PR性质
- **测试和文档要求**：保证项目质量
- **兼容性检查**：避免破坏性变更

## 🤖 自动化功能

### 自动标签系统

- 根据文件变更路径自动添加相关标签
- 根据issue/PR内容智能添加标签
- 标记破坏性变更

### PR验证

- 检查PR描述完整性
- 验证必要信息是否填写
- 自动欢迎首次贡献者

### Issue管理

- 自动欢迎新issue
- 为求助类issue提供资源链接
- 管理长期无活动的issue

## 🚀 使用方法

### 对于贡献者

1. **提交Issue**
   - 访问 [Issues页面](https://github.com/lamina-dev/Lamina/issues/new/choose)
   - 选择合适的模板
   - 按照模板填写信息

2. **提交Pull Request**
   - 创建PR时会自动加载模板
   - 填写所有必要的信息
   - 完成检查清单中的项目

### 对于维护者

1. **Review Process**
   - 检查PR是否填写完整
   - 验证是否通过自动检查
   - 根据标签快速分类处理

2. **Issue Management**
   - 利用自动标签快速分类
   - 使用模板信息快速了解问题
   - 跟踪issue的处理状态

## 📋 标签说明

### 自动标签

- `bug` - Bug报告
- `enhancement` - 功能请求
- `documentation` - 文档相关
- `question` - 问题求助
- `core` - 核心解释器变更
- `extensions` - 扩展/插件相关
- `build` - 构建系统相关
- `breaking-change` - 破坏性变更

### 优先级标签

- `priority:high` - 高优先级
- `priority:medium` - 中等优先级
- `priority:low` - 低优先级

### 状态标签

- `good first issue` - 适合新手
- `stale` - 长期无活动
- `pinned` - 重要issue（不会被自动关闭）

## 🔧 自定义配置

如需修改模板或自动化规则，请编辑相应的配置文件：

- **修改标签规则**：编辑 `labeler.yml`
- **调整模板字段**：编辑对应的 `.yml` 模板文件
- **修改自动化行为**：编辑 `workflows/` 目录下的工作流文件

## 📞 获取帮助

如果您在使用模板时遇到问题：

1. 📖 查看 [贡献指南](../documents/CONTRIBUTING-CN.md)
2. 💬 加入 [QQ交流群](https://qm.qq.com/q/QwPXCgsJea)
3. 📝 提交 [问题求助Issue](https://github.com/lamina-dev/Lamina/issues/new?template=help_wanted.yml)

---

感谢您使用我们的模板系统！🎉
