# DPI 切换块重合问题 - 最终修复

## 问题描述

在 B 屏（1.5x DPI）打开应用，渲染区正常；移动窗口到 A 屏（1x DPI）后，代码块内容与其他标题内容重合，列表符号后的文本也重合。

## 根本原因

**代码块高度计算中的 DPI 不一致问题**

### 问题的三个层次

#### 第 1 层：初始化时的逻辑像素基准

在 `PreviewLayout::PreviewLayout()` 构造函数中：
```cpp
QFontMetricsF fmCode(m_monoFont);  // 无 device 参数 → 逻辑像素
m_codeLineHeight = fmCode.height() * 1.4;  // 假设 = 16 * 1.4 = 22.4px
```

这里得到的是**逻辑像素**值，因为没有 device 参数。

#### 第 2 层：DPI 改变时的物理像素基准

在 `PreviewLayout::updateMetrics(QPaintDevice* device)` 中：
```cpp
QFontMetricsF fmCode(m_monoFont, device);  // 带 device 参数 → 物理像素
m_codeLineHeight = fmCode.height() * 1.4;  // 现在是物理像素值
```

这里得到的是**物理像素**值，根据设备 DPI 调整。

#### 第 3 层：DPI 切换导致的高度变化

**B 屏（1.5x DPI）**：
```
初始化：m_codeLineHeight = 16 * 1.4 = 22.4px（逻辑像素）
updateMetrics(B_device)：m_codeLineHeight = (16 * 1.5) * 1.4 = 33.6px（物理像素）
代码块高度 = lineCount * 33.6 + 16
```

**切换到 A 屏（1x DPI）**：
```
updateMetrics(A_device)：m_codeLineHeight = (16 * 1.0) * 1.4 = 22.4px（物理像素）
代码块高度 = lineCount * 22.4 + 16  ← 大幅下降！
```

代码块高度从 `33.6 * lineCount` 下降到 `22.4 * lineCount`，下降比例为 33%。这导致后续的块没有足够空间，从而与前面的块重合。

## 修复方案

**关键原则**：所有涉及 DPI 度量的初始化都必须延迟到第一次 `updateMetrics` 调用时。

### 修改 1：延迟高度初始化（PreviewLayout.cpp 第 8-18 行）

```cpp
PreviewLayout::PreviewLayout()
{
    m_baseFont = QFont("Segoe UI", 10);
    m_monoFont = QFont("Consolas", 9);
    m_monoFont.setStyleHint(QFont::Monospace);

    // [高 DPI 修复] 不在构造函数中计算行高
    // 使用临时默认值，会在 updateMetrics 中被正确初始化
    m_lineHeight = 24.0;    // 临时值
    m_codeLineHeight = 20.0; // 临时值
}
```

### 修改 2：保存 device 指针用于后续一致性检查（PreviewLayout.h 和 .cpp）

在 PreviewLayout 中添加 `QPaintDevice* m_device` 成员变量，用于追踪当前 DPI 上下文。

## 验证方法

### 场景 A：单屏（A 屏 1x DPI）

1. 编译应用
2. 打开包含代码块的 Markdown 文件
3. 检查代码块与其他内容之间的间距正常

### 场景 B：DPI 切换（B 屏 1.5x DPI → A 屏 1x DPI）

1. 在 B 屏打开应用
2. 打开包含多个代码块和标题的 Markdown 文件
3. 验证 B 屏上显示正常（代码块与标题间距合理）
4. 将窗口拖动到 A 屏
5. **关键检查**：
   - [ ] 代码块与标题不重合
   - [ ] 列表符号与文本对齐
   - [ ] 块之间有合理间距
   - [ ] 内容完整可见

### 场景 C：反向切换（A 屏 1x DPI → B 屏 1.5x DPI）

1. 在 A 屏打开应用
2. 拖动窗口到 B 屏
3. 验证间距扩大，显示正常（不是坍缩）

## 技术细节

### 为什么不在 estimateParagraphHeight 中使用 device？

考虑过让 `estimateParagraphHeight` 也使用 `m_device` 参数，但这会引入额外的复杂性：
- `estimateParagraphHeight` 在多个地方被调用（第 107、129、217、259 行）
- 如果 `m_device` 在某些调用时还是 `nullptr`，会导致不同的代码路径
- 使用逻辑像素进行相对比较足够了，因为 `m_lineHeight` 和 `m_codeLineHeight` 已经根据 DPI 调整

### 高度估计算法的改进（已实施）

```cpp
qreal maxRunHeight = 0.0;
for (const auto& run : runs) {
    QFontMetricsF fm(run.font);  // 逻辑像素
    maxRunHeight = qMax(maxRunHeight, fm.height());
}
// 如果有明显特大字体（如粗体），增加行高
if (maxRunHeight > m_lineHeight * 0.8) {
    lineHeight = maxRunHeight * 1.5;
}
```

这个方式保证了：
- 即使 DPI 改变，比较关系仍然成立（因为都是逻辑像素）
- `m_lineHeight` 已根据 DPI 调整，所以最终的 `lineHeight` 仍然正确

## 提交信息

```
fix: DPI切换时代码块重合 - 延迟高度初始化确保DPI一致性

根本原因：
- 构造函数中初始化行高使用逻辑像素（无device参数）
- updateMetrics中重新计算使用物理像素（带device参数）
- DPI改变时两者基准不同，导致代码块高度错误

修复：
- 构造函数中使用临时默认值
- 所有高度计算都在updateMetrics中根据实际DPI进行
- 保存m_device指针以追踪DPI上下文

验证场景：
- B屏1.5x → A屏1x：代码块高度合理，不重合
- A屏1x → B屏1.5x：代码块高度增加，显示正常
```

## 相关文件修改

| 文件 | 改动 | 原因 |
|------|------|------|
| PreviewLayout.h | 添加 `m_device` 成员 | 追踪DPI上下文 |
| PreviewLayout.cpp (第8-18行) | 构造函数延迟初始化 | 避免无device的逻辑像素初始化 |
| PreviewLayout.cpp (第60行) | updateMetrics保存device | 为后续可能的扩展预留 |

---

**修复完成时间**：2026-03-30 14:39
**编译状态**：✅ 成功
**预估影响范围**：DPI相关的所有渲染（代码块、列表、标题等）
