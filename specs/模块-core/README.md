# 模块：core（核心数据模型）

## 职责

提供文本存储、撤销、选区、文件 I/O、主题、最近文件等**无 GUI 依赖**的基础设施。所有上层模块都可以依赖 core，core 不依赖任何其他模块。

## 对应源码

`src/core/`

## Spec 清单

| 编号 | 标题 | 状态 | 对应源文件 |
|------|------|------|-----------|
| 01 | 文本存储（PieceTable） | draft | `PieceTable.h/cpp` |
| 02 | 文档模型（Document） | draft | `Document.h/cpp` |
| 03 | 撤销栈（UndoStack） | draft | `UndoStack.h/cpp` |
| 04 | 选区模型（Selection） | draft | `Selection.h/cpp` |
| 05 | 行索引（LineIndex） | draft | `LineIndex.h/cpp` |
| 06 | 文件映射（MappedFile） | draft | `MappedFile.h/cpp` |
| 07 | 最近文件（RecentFiles） | draft | `RecentFiles.h/cpp` |
| 08 | 主题系统（Theme） | draft | `Theme.h/cpp` |

## 依赖关系

```
Document
  ├─ PieceTable（持有）
  ├─ UndoStack（持有）
  └─ Selection（持有）

LineIndex    ← 被 Editor 使用，从 PieceTable 增量计算
MappedFile   ← 被 PieceTable 用于大文件的只读映射
RecentFiles  ← 独立，QSettings 持久化
Theme        ← 被 Editor 和 Preview 读取
```

## 全局约束

- 所有类必须是**线程安全**或明确标注为主线程独占
- 禁止依赖 Qt::Widgets，只允许 Qt::Core
- 文本 API 以 UTF-16 码元为单位（与 QString 一致）
