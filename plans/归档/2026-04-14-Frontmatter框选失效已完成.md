---
date: 2026-04-14
status: draft
related_specs: [specs/模块-preview/10-Frontmatter渲染.md]
---

# Frontmatter 渲染内容不支持框选

## 背景

用户反馈：预览区里 Markdown 正文可以用鼠标拖拽框选并复制，但文档头部由 frontmatter 渲染出来的 key/value 块（例如 `title: Frontmatter 渲染测试`）**无法框选、无法复制**，鼠标拖过去没任何选区反馈。

这与 `specs/模块-preview/10-Frontmatter渲染.md` 的 **INV-13（选区与复制）**直接冲突——Spec 明确要求 frontmatter block 整体可选，且复制文本为原始 YAML（`---\nkey: value\n...\n---`）。当前代码实现了"存在"与"渲染"，但没实现"可选"。

## 根因分析（已读代码验证）

frontmatter 的绘制路径与正文完全不同，**绕过**了驱动选区的数据结构：

1. **`PreviewPainter::paintFrontmatter`**（`src/preview/PreviewPainter.cpp:642-719`）直接用 `p->drawText(QPointF(...), key/value)` 把文字画到 QPainter 上，**从未调用** `pushSegment(...)` / 追加 `TextSegment`。对比正文路径：`paintInlineRuns` 会 `m_textSegments.append({rect, charStart, charLen, text, font, linkUrl})`（`src/preview/PreviewPainter.cpp:51`），这些 segment 就是选区 hit-test 的唯一数据源。
2. **`PreviewWidget::textIndexAtPoint`**（`src/preview/PreviewWidget.cpp:582-624`）遍历 `m_painter->textSegments()`，对落在某 segment 矩形内的鼠标点才能返回有效字符索引；frontmatter 区域**没有任何 segment 覆盖**，拖拽时 `m_selStart = m_selEnd` 永远落到"最近的非-frontmatter segment"上，视觉上就是选区"穿越"了 frontmatter 块直接跳到下面第一段。
3. **`PreviewWidget::extractBlockText`**（`src/preview/PreviewWidget.cpp:539-563`）只处理 `block.inlineRuns` 和 `block.codeText`，**没有** `block.type == Frontmatter` / `frontmatterRawText` 分支。即便选区强行跨过 frontmatter，复制也会把这段内容跳过去。
4. **字符索引空间已预留但没人消费**：`paintBlock`/`countBlockChars`（`src/preview/PreviewPainter.cpp:468-476`、`721-727`）在绘制时 `m_charCounter += block.frontmatterRawText.length() + 1`，说明全局选区 index 空间给 frontmatter 留了一段连续编号，但既没有 segment 把几何矩形映射到这个区间，也没有 `extractBlockText` 把这个区间映射回原文 —— 三方（segment / char-counter / extractText）**不闭环**。
5. **选区高亮绘制**（`src/preview/PreviewPainter.cpp:186、520`）只在 segment 上画 overlay；frontmatter 自己的 `paintFrontmatter` 里完全没有读 `m_selStart/m_selEnd`，也就不会显示任何高亮反馈。

**结论**：frontmatter 不是渲染 bug，是"未接入选区状态机"。修复方向是把 frontmatter block 整体（或每行 key/value）注册为 `TextSegment`，并在 `extractBlockText` 里补 frontmatter 分支输出 `frontmatterRawText`。

## 动作

- [ ] `src/preview/PreviewPainter.cpp::paintFrontmatter`：绘制 key / value 时按 INV-13 注册 segment。两种粒度选一：
  - **粗粒度（推荐，最小改动、对齐 INV-13"整体可选"语义）**：为整个 frontmatter block 注册一个 TextSegment——`rect = QRectF(absX, absY, w, h)`，`charStart = m_charCounter`，`charLen = frontmatterRawText.length()`，`text = frontmatterRawText`，font 用 fmFont。点击块内任何位置 → 命中该 segment → 拖拽即选中整块。
  - **细粒度**：每个 value 换行物理行单独 push 一个 segment（一行一个 rect）。视觉更精准但 hit-test 边界需精算，先用粗粒度。
- [ ] `src/preview/PreviewPainter.cpp::paintFrontmatter`：在绘制背景后、绘制文字前，读取 `m_selStart/m_selEnd`，若与该 block 的 `[charStart, charStart + rawText.length()]` 有交集，则在整块范围上叠加半透明选区色（复用 `m_theme.editorSelection` 或现有 inline 选区色 `QColor(0,120,215,80)`）。至少提供"整块被选中"的视觉反馈。
- [ ] `src/preview/PreviewWidget.cpp::extractBlockText`：新增 `if (block.type == LayoutBlock::Frontmatter)` 分支 → `out += block.frontmatterRawText; out += '\n';`（末尾 `\n` 与 `countBlockChars` 里 `+1` 保持一致，确保 `m_plainText` 总长度 = `m_charCounter`）。同时确保 return 之后**不要**递归 children（frontmatter 无子块，但防御式短路）。
- [ ] 确认 `paintFrontmatter` 内 `pushSegment` 的 charStart 计算与 `paintBlock` 里在该 block 绘制**之前**的 `m_charCounter` 值一致（目前 PreviewPainter.cpp:473 是绘制完之后才 +=，要么改为在 paintFrontmatter **开头**先记下 `startCharIndex = m_charCounter`，要么把 `m_charCounter +=` 提前到 paintFrontmatter 调用前）。避免选区索引与文本索引错位 1 个 block。
- [ ] （可选，验收 T-14 需要）若 Spec 仍要求复制为"原始 YAML"而非 segment 显示字符，且 `frontmatterRawText` 里已含起止 `---`，确认 `extractBlockText` 输出完整；否则补齐 `---\n...\n---` 包裹。

## 验收

- [ ] **[T-F1]** 在 frontmatter 块内任意位置按下鼠标左键拖拽到块外，松开后：frontmatter 区域显示可见的选区高亮（整块或至少命中行）；Ctrl+C 复制到剪贴板的文本包含 `---\ntitle: xxx\n...\n---`
- [ ] **[T-F2]** 在 frontmatter 块内三击（或 Ctrl+A）后，frontmatter 文本参与 select-all；复制内容中包含原始 YAML，不含渲染后的表格空白
- [ ] **[T-F3]** 从正文中段向上拖拽穿越 frontmatter 到文档顶部，选区连续覆盖正文 + frontmatter + 文档头部空白，复制文本顺序 = 原文顺序（frontmatter 的 rawText 排在正文之前）
- [ ] **[T-F4]** 仅选中 frontmatter 下面一段正文时，frontmatter 本身**不应**有选区高亮（回归测试：新 segment 不会因 charStart 错位而被误选）

## 风险

- **索引错位风险**：`m_charCounter` 累加顺序是"先画该 block → 再 +=rawText.length()+1"。如果 segment 的 charStart 取当前值（block 绘制**后**的值），就会比 `extractBlockText` 里该 block 的起点大一个 block 的长度，选区与文本严重错位。必须把 `+=` 调整到 frontmatter paint **之前**（与其它 block 类型保持"先 push segment 再递增"的统一顺序，参考 paintInlineRuns 的现有实现）。
- **整块 segment 的 hit-test 副作用**：如果整个 frontmatter block 注册成一个大 segment，鼠标点击块内任意位置都会 hit 到"charStart"或"charStart+charLen"两个极端，中间位置 `hitTestSegment` 会按字符度量逐字计算 —— 但 frontmatter 的 rawText 和渲染后的两列表格字符排列**不是一一对应**（表格有换行、缩进、padding），拖拽中间会出现"视觉上选中了半行但复制出的是 rawText 的前半段"的不对称。MVP 可接受（Spec INV-13 本来就要求"整体可选"，中间拖拽不必做像素级精度）。
- **两列布局的 hit-test 不精确**：key 列和 value 列在视觉上是独立矩形，若改为细粒度 segment，key 列和 value 列之间的空白区域不在任何 segment 里，拖拽经过会触发"最近 segment"回退逻辑，可能导致选区跳变。先用粗粒度规避。
- **与 search/marking 的交互**：frontmatter 目前是否支持 Ctrl+F 搜索到其文本未验证；若 `m_plainText` 原先不含 frontmatter，本次改完后 `m_plainText` 会变长，search 索引可能需要重建。需要跑一遍搜索回归。
- **m_charCounter 与 extractBlockText 的统一性**：INV 约束是 `m_charCounter` == `m_plainText.length()`。改完后两者都要同步包含 frontmatter，否则选区 index → 文本 offset 的映射会在 frontmatter 之后的所有 block 上全局偏移。
