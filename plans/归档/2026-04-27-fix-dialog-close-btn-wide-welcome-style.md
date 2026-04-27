---
title: 快捷键 / 更新历史对话框关闭按钮改为欢迎页风格 + 横向铺满
status: in_progress
created: 2026-04-27
related_specs:
  - specs/模块-app/07-快捷键弹窗.md
  - specs/模块-app/22-空白引导页.md
---

# 改进：对话框关闭按钮风格统一到 WelcomePanel + 宽度铺满

## 现状
- 三对话框（快捷键 / 更新历史 / 关于）关闭按钮已通过 `DialogButtonStyle.h::styleDialogCloseButton` 风格统一（accent 色边框、淡背景、固定 min-width=70px）
- 关闭按钮在 ShortcutsDialog / ChangelogDialog 中放在底部右对齐的 HBox layout

## 用户期望
- 快捷键面板 / 更新历史面板的**关闭按钮**：
  - 视觉风格**与 WelcomePanel 主按钮一致**（accent 实心背景、白字、圆角 6px、padding 大、字号 11pt）
  - **横向铺满整个对话框宽度**（不是右对齐的小按钮，而是宽条按钮）

## 设计
- `DialogButtonStyle.h` 增加 `styleDialogPrimaryButtonWide(QPushButton*, const Theme&)`：
  - 背景：theme.accentColor（实心）
  - 前景：白色
  - 边框：无
  - 圆角：6px
  - padding：12px 上下，水平取决于布局
  - 字号 11pt，semi-bold
  - hover：accent.darker(110)
- `ShortcutsDialog::applyTheme` / `ChangelogDialog::applyTheme`（或 ctor）：
  - 关闭按钮的父 HBox 改为单 widget addWidget（不带 stretch 兄弟节点）
  - 按钮 sizePolicy = Expanding 让其横向铺满
  - 调 `styleDialogPrimaryButtonWide` 替代 `styleDialogCloseButton`
- About 对话框用 QMessageBox，按钮无法控制 layout 横向铺满（QMessageBox 内部固定按钮 row）→ **不改**，保留当前 `styleDialogCloseButton` 风格（确认范围）

## 验收
- T-1：快捷键面板底部关闭按钮：**横向铺满**对话框宽度，accent 实心背景，白字
- T-2：更新历史面板底部关闭按钮：同 T-1
- T-3：主题切换后两按钮颜色跟随
- T-4：与 WelcomePanel 主按钮（"打开文件..."）视觉风格一致
