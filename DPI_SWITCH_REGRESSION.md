# DPI 切换回归问题及修复

## 问题描述

在 DPI 切换时出现块重合问题：
- **场景**：在 B 屏（1.5x DPI）打开应用 → 移动窗口到 A 屏（1x DPI）
- **现象**：代码块内容和其他标题内容重合到一起了
- **根本原因**：高度估计修复导致某些情况下高度计算不足

## 原始修复的问题

之前的高度估计修复为了保证 DPI 一致性，完全去掉了混合字体大小的考虑：

```cpp
// 原始修复（过于激进）
qreal lineHeight = m_lineHeight;
for (const auto& run : runs) {
    QFontMetricsF fm(run.font);
    totalWidth += fm.horizontalAdvance(run.text);
    // 不再调整 lineHeight
}
```

**问题**：
- 如果段落包含特别大的字体（如粗体文本），估计高度会不足
- DPI 切换时，这导致块重合

## 改进的修复方案

PreviewLayout.cpp 第 372-399 行的改进：

```cpp
qreal maxRunHeight = 0.0;

for (const auto& run : runs) {
    if (run.text == "\n") {
        newlineCount++;
        continue;
    }
    QFontMetricsF fm(run.font);  // 逻辑像素
    totalWidth += fm.horizontalAdvance(run.text);
    maxRunHeight = qMax(maxRunHeight, fm.height());
}

// 保持 DPI 一致性：两个值都是逻辑像素
if (maxRunHeight > m_lineHeight * 0.8) {
    lineHeight = maxRunHeight * 1.5;
}
```

**改进点**：
1. **保持 DPI 一致性** - 两个都使用逻辑像素（无 device 参数）
2. **考虑混合字体** - 如果有特别大的字体，增加行高
3. **DPI 比例保持** - 即使 DPI 改变，maxRunHeight 和 m_lineHeight 的比例关系不变

## DPI 切换流程验证

### 从 B 屏（1.5x）→ A 屏（1x）

1. **B 屏初始化**：
   - updateMetrics(B_device) → m_lineHeight = 36
   - buildFromAst() → 计算块高度，maxRunHeight * 1.5 = 45
   - 块高度 = 45 * lineCount

2. **移动到 A 屏，resizeEvent 触发**：
   - updateMetrics(A_device) → m_lineHeight = 24
   - rebuildLayout() → 重新计算块高度
   - maxRunHeight 相对于逻辑像素，不变（比例关系保持）
   - 新块高度 = 新lineHeight * lineCount（更小，但合理）

3. **paintEvent 中的 DPI 检测**：
   - 如果 DPI 变化被检测，再次调用 rebuildLayout()
   - 确保最终块高度正确

### 高度一致性保证

关键是**两个系统使用同一度量标准**：
- 布局阶段：estimateParagraphHeight 使用逻辑像素
- 绘制阶段：paintInlineRuns 也使用逻辑像素（p->device()）
- 都会在 updateMetrics 中同步调整 m_lineHeight

## 验证方式

### 关键测试场景

1. **正常场景**（块高度足够）：
   - [ ] A 屏：正常显示，块间距合理
   - [ ] B 屏：拖过去显示正确，块间距保持
   - [ ] 回到 A 屏：块不重合

2. **混合字体场景**（包含粗体、大字体）：
   - [ ] 包含加粗文本的段落高度足够
   - [ ] DPI 切换时不会导致重合

3. **边界情况**：
   - [ ] 非常小的字体
   - [ ] 非常大的字体
   - [ ] 快速切换多个屏幕

## 相关代码位置

| 文件 | 行号 | 改动 |
|------|------|------|
| PreviewLayout.cpp | 372-399 | estimateParagraphHeight 改进 |
| PreviewLayout.cpp | 34-62 | updateMetrics（保持不变） |
| PreviewPainter.cpp | 287-365 | paintInlineRuns（使用 p->device()） |
| PreviewWidget.cpp | 94-135 | DPI 检测和重建布局逻辑 |

## 后续观察

监控以下场景，确保没有其他回归：
- [ ] 代码块在不同 DPI 的高度
- [ ] 列表项的行间距
- [ ] 表格单元格的内容对齐
- [ ] 引用块中的嵌套内容
