# 横切关注点（Cross-Cutting Concerns）

这里存放**跨多个模块**的不变量和规则。模块 Spec 必须在 `depends` 字段中引用相关的横切 Spec。

## Spec 清单

| 编号 | 标题 | 状态 | 影响模块 |
|------|------|------|---------|
| 10 | 线程模型 | draft | 全部 |
| 20 | 坐标系统 | draft | editor, preview |
| 30 | 主题系统 | draft | core, editor, preview, app |
| 40 | 高 DPI 适配 | draft | editor, preview |
| 50 | 字符编码与 IO | draft | core, app |
| 60 | 国际化 | draft | app |
| 70 | 性能预算 | draft | 全部 |
| 80 | 字体系统 | stable | editor, preview, app |

## 为什么需要横切 Spec

有些约束不属于任何单一模块，而是**跨模块的共同契约**。例如：

- **高 DPI 适配**：`editor` 和 `preview` 都有自己的布局和绘制，两者必须遵守同一套度量规则
- **坐标系统**：父块到子块的绝对坐标传递规则，在列表、引用、表格等多种块结构中必须一致
- **主题系统**：所有 UI 模块共享同一套 `Theme` 数据，深浅切换时必须同步响应

如果把这些约束写进单个模块 Spec，会出现：
1. 重复（多个 Spec 说同一件事）
2. 失同步（一处改了其他忘改）
3. 看不出全局（不知道哪些地方需要遵守）

**抽成横切 Spec 后**：
- 单个模块只需在 `depends` 中引用，并在 INV 里说「遵守 specs/横切关注点/40-高DPI适配.md 的所有 INV」
- 违反横切约束 = 违反多个模块 Spec，PR review 时会被放大

## 编号策略

使用「十位」编号（10/20/30…），方便将来插入新的横切概念而不用重编号。
