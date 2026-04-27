---
title: 默认启动不创建未命名文件 + 空白引导面板
status: completed
created: 2026-04-24
completed: 2026-04-27
related_specs:
  - specs/模块-app/22-空白引导页.md
  - specs/模块-app/20-左侧面板.md
---

# 默认启动不创建未命名文件 + 空白引导面板（已完成）

## 需求
1. 全新启动（无会话恢复）时，不自动创建"未命名"空白文件标签，而是显示空白引导面板
2. 只有用户点击 Tab 栏右侧的 "+" 按钮时，才创建新的未命名文件
3. 空白引导面板可显示提示信息或使用说明（如快捷键、打开文件/文件夹入口等）

## 落地（2026-04-27）
- 新增 `specs/模块-app/22-空白引导页.md` 定义 INV-EMPTY-NO-AUTO-CREATE / WELCOME-MUTUAL / CLOSE-LAST / PLUS-VISIBLE / OPEN-EXITS / I18N / THEME / NO-CURRENT-TAB-DEREF
- 新增 `src/app/WelcomePanel.h/.cpp`：标题 / 副标题 / 三按钮（打开文件 / 打开文件夹 / 新建文件）/ 快捷键提示，主题响应
- `MainWindow::restoreSession` 末尾兜底从 `newTab()` 改为 `updateEmptyState()`
- `MainWindow::onCloseTab` 中关闭最后一个 Tab 后兜底同上
- `MainWindow::newTab` 和 `openFile` 末尾调 `updateEmptyState()`
- `applyTheme` 同步 WelcomePanel 主题
- i18n：zh_CN / en_US 各加 5 条 WelcomePanel context 翻译

## 验收（已通过）
- [T-1] 清空配置首次启动，无标签页，显示引导面板 ✓ 用户 UI 验证通过
- [T-2] 点击 "+" 按钮后创建未命名文件标签 ✓
- [T-3] 会话恢复有文件时正常恢复，不显示引导面板 ✓
