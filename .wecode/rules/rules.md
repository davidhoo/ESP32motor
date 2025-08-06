# 开发规则

## 开发流程

本项目采用 **GitHub Flow** 作为开发流程标准。

### GitHub Flow 规范

1. **主分支保护**
   - `main` 分支为生产就绪代码
   - 禁止直接向 `main` 分支推送代码
   - 所有更改必须通过 Pull Request 合并

2. **功能开发流程**
   - 从 `main` 分支创建功能分支：`feature/功能描述`
   - 功能分支命名规范：
     - `feature/新增功能` - 新功能开发
     - `fix/问题修复` - Bug修复
     - `docs/文档更新` - 文档更新
     - `refactor/代码重构` - 代码重构

3. **Pull Request 要求**
   - PR标题清晰描述更改内容
   - PR描述包含：
     - 更改原因
     - 测试说明
     - 相关Issue链接（如有）
   - 至少一个代码审查者批准
   - 所有CI检查通过

4. **代码审查标准**
   - 代码风格符合项目规范
   - 功能测试通过
   - 文档已更新（如需要）
   - 无安全漏洞

5. **发布流程**
   - 通过PR合并到`main`分支即触发发布
   - 使用GitHub Releases创建版本标签
   - 遵循语义化版本号规范（SemVer）

6. **紧急修复**
   - 从`main`分支创建`hotfix/修复描述`分支
   - 快速审查后立即合并
   - 确保修复同时合并回开发分支

7. **GitHub CLI 使用规范**
   - 使用 `gh` 命令行工具创建和管理 PR
   - 创建 PR 命令模板（注意特殊字符转义）：
     ```bash
     # 基础命令（适用于简单标题和描述）
     gh pr create --title "feat: 添加新功能" --body "本次更改包含..."
     
     # 复杂内容处理（使用文件避免转义问题）
     echo "PR标题" > pr_title.txt
     echo "PR描述内容" > pr_body.md
     gh pr create --title "$(cat pr_title.txt)" --body-file pr_body.md
     
     # 或使用 heredoc 避免转义问题
     gh pr create --title "feat: 新功能" --body "$(cat << 'EOF'
     ## 更改说明
     - 修复了XXX问题
     - 优化了YYY性能
     EOF
     )"
     ```
   - 自动合并和清理：
     ```bash
     # 启用自动合并（当满足合并条件时）
     gh pr merge --auto --squash
     
     # 合并后立即删除分支
     gh pr merge --squash --delete-branch
     
     # 完整流程示例
     gh pr create --title "feat: 新功能" --body "功能描述" && \
     gh pr merge --squash --delete-branch
     ```
   - 特殊字符处理：
     - 避免在命令行直接使用引号、反斜杠等特殊字符
     - 使用单引号包裹复杂字符串，或使用 heredoc 语法