---
title: TOC 面板 item 出现矩形边框
status: draft
created: 2026-04-20
related_specs: []
---

## Bug

TOC 面板中特定场景下某个 item 会出现矩形边框（类似焦点框/选中框），不应存在。
需要检查 QPushButton 的 focus 样式和 outline，确保去掉所有框选效果。
