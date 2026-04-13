# 模块：parser（Markdown 解析）

## 职责

封装 cmark-gfm 提供 Markdown → AST 的解析能力，包含防抖调度，确保解析不阻塞主线程。

## 对应源码

`src/parser/`

## Spec 清单

| 编号 | 标题 | 状态 | 对应源文件 |
|------|------|------|-----------|
| 01 | Markdown 解析器 | draft | `MarkdownParser.h/cpp` |
| 02 | AST 节点封装 | draft | `MarkdownAst.h/cpp` |
| 03 | 解析调度器 | draft | `ParseScheduler.h/cpp` |

## 依赖关系

```
ParseScheduler（主线程 QObject）
  ├─ 30ms 防抖定时器
  ├─ 后台线程池
  └─ 调用 MarkdownParser 解析
       └─ 产出 MarkdownAst，emit astReady(ast)
```

## 性能预算

| 操作 | 目标 |
|------|------|
| 1k 行 Markdown 解析 | < 10 ms |
| 10k 行 Markdown 解析 | < 50 ms |
| 防抖延迟 | 30 ms |

## 全局约束

- 解析必须在后台线程
- AST 必须是不可变快照（immutable snapshot），主线程消费时不加锁
- 用 `std::shared_ptr<const MarkdownAst>` 传递所有权
- 解析失败返回空 AST，不抛异常
