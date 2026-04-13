# 模块：editor（自绘编辑器）

## 职责

基于 `QAbstractScrollArea` 的完全自绘 Markdown 编辑器，提供文本编辑、语法高亮、搜索替换、IME、滚动等能力。追求 < 3ms 的打字延迟。

## 对应源码

`src/editor/`

## Spec 清单

| 编号 | 标题 | 状态 | 对应源文件 |
|------|------|------|-----------|
| 01 | 编辑器主控件 | draft | `EditorWidget.h/cpp` |
| 02 | 行布局引擎 | draft | `EditorLayout.h/cpp` |
| 03 | 绘制管线 | draft | `EditorPainter.h/cpp` |
| 04 | 输入与 IME | draft | `EditorInput.h/cpp` |
| 05 | 语法高亮 | draft | `SyntaxHighlighter.h/cpp` |
| 06 | 查找替换 | draft | `SearchBar.h/cpp`, `SearchWorker.h/cpp` |
| 07 | 行号绘制 | draft | `GutterRenderer.h/cpp` |
| 08 | 带缓冲绘制 | draft | `BandBuffer.h/cpp` |

## 依赖关系

```
EditorWidget
  ├─ EditorLayout       ← 行布局与坐标转换
  ├─ EditorPainter      ← 绘制逻辑
  ├─ EditorInput        ← 键盘/鼠标/IME
  ├─ SyntaxHighlighter  ← 正则高亮
  ├─ SearchBar          ← UI 浮层
  │    └─ SearchWorker  ← 后台搜索
  ├─ GutterRenderer     ← 行号列
  └─ BandBuffer         ← 可见区域位图缓存
     └─ core::Document  ← 文本源
```

## 性能预算

| 操作 | 目标 |
|------|------|
| 打字响应 | < 3 ms |
| 滚动一屏 | < 10 ms |
| 切换 10k 行文件 | < 50 ms |
| 全文档语法高亮 | < 200 ms |

## 全局约束

- 所有绘制必须在主线程
- 不得在 `paintEvent` 中触发布局重算，布局必须预先完成
- 搜索必须在 worker 线程，UI 不阻塞
