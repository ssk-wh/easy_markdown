---
date: 2026-04-15
status: completed
related_specs:
  - specs/模块-editor/README.md
---

# 编辑器语法染色补 strikethrough 与 bold-italic

## 背景

当前 `SyntaxHighlighter` 覆盖 heading / bold / italic / code / link / list / quote / fence，但
缺两种常见行内标记：

- **strikethrough（`~~text~~`）**：parser/preview 已支持（`AstNodeType::Strikethrough`，
  `PreviewPainter` 走 `isStrikethrough` 画删除线），`EditorInput` 甚至已绑定 `Ctrl+D` 将选区包裹为 `~~...~~`。
  **但编辑器染色视觉缺失**：用户输入 `~~foo~~` 后在编辑区看不到任何样式提示，
  只能切到预览看效果——交互断裂。
- **bold-italic（`***text***`）**：当前 bold 正则 `\*\*([^*]+)\*\*` 和 italic 正则
  `(?<!\*)\*([^*]+)\*(?!\*)` 均无法匹配 `***foo***`（内部都含 `*`），导致此种三星语法
  毫无染色。

本 plan 聚焦这两处小缺口的染色补齐，不涉及 h1-h6 级别分色、frontmatter 染色、表格边框等
「2026-04-14-编辑器语法染色增强」的大改动。

## 动作

- [x] `SyntaxHighlighter` 新增 strikethrough 格式，正则 `~~([^~]+)~~`，format 启用 `setFontStrikeOut(true)`
- [x] `SyntaxHighlighter` 新增 bold-italic 格式，正则 `\*\*\*([^*]+)\*\*\*`，format 粗体+斜体
- [x] bold-italic 必须在 bold / italic 之前匹配，避免子串重复着色
- [x] strikethrough 与 bold-italic 都尊重 inline code 范围（`~~inside`code`~~` 不着删除线穿越 code）
- [x] 新增测试 `tests/editor/SyntaxHighlighterTokensTest.cpp`
- [x] `specs/模块-editor/README.md` 补 INV-EDIT-STRIKE 与 INV-EDIT-BOLDITALIC 说明与 T 条目
- [x] `CHANGELOG.md` 追加条目

## 验收

- **T-STRIKE-1**：输入 `~~foo~~`，高亮 tokens 包含一个 format.fontStrikeOut() == true
- **T-BOLDITALIC-1**：输入 `***foo***`，高亮 tokens 包含一个既粗体又斜体的 format
- **T-PRIORITY-1**：`~~` / `***` 标记出现在 `` `inline code` `` 内部时不产生对应的删除线/粗斜体 token

## 风险

- regex `[^~]+` 对含 `~` 字符的名字会失败（如 `~~a~b~~` 里 `a~b` 含 `~`），这是刻意保留的
  非贪婪简化，避免跨多行误匹配；与现 bold/italic 使用的 `[^*]+` 策略对齐。
- bold-italic `[^*]+` 同理不处理 `***a*b***` 这种包含 `*` 的边缘情况，与现有 bold/italic 一致。
