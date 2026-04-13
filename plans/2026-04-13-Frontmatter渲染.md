---
date: 2026-04-13
status: draft
related_specs: [specs/模块-preview/10-Frontmatter渲染.md]
---

# 预览区 Frontmatter 渲染

## 背景

很多 Markdown 文档（尤其是本项目的 Spec 文件、Jekyll/Hugo/Obsidian 内容、各种静态站源文）在开头会有一段被 `---` 包裹的 YAML frontmatter，用于存放 id、status、owners、tags 等元数据。SimpleMarkdown 目前把这段内容按"水平分割线 + 普通段落"的组合处理——视觉上就是开头一大坨纯文本紧跟一条横线，**非常丑**，而且破坏了文档正文的视觉节奏。

设计目标是识别首个 `---...---` 块，把它当作结构化的"信息表"渲染：

- 带背景的独立 block（与代码块类似）
- 背景是系统强调色 + 预览背景做 50% RGB 混合，淡化但仍有色彩暗示
- 内部是 n 行 2 列的无表头表格：左列是 key，右列是 value
- 第一列宽度按最长 key 自适应，但不超过可视区 50%
- key / value 都用等宽字体，与代码块保持一致的视觉族
- 缩放、主题切换、高 DPI 都要正确联动

## 目标

落地 `specs/模块-preview/10-Frontmatter渲染.md` 中 INV-1 到 INV-15 的全部行为契约，使 T-1 到 T-19 通过（INV-16 的链接点击交互标为 future，不在本 Plan 范围）。

具体可观测目标：

1. 打开 `specs/模块-preview/10-Frontmatter渲染.md`（本 Spec 自身），预览区显示带背景框的 frontmatter 表格
2. 打开一个首行不是 `---` 的文档，预览区无任何变化（回归安全）
3. Ctrl+滚轮缩放时 frontmatter 字号随之变化
4. 切换深/浅色主题时 frontmatter 背景色、边框色、文字色同步更新
5. 用鼠标选中整个 frontmatter 并 Ctrl+C，粘贴到编辑器得到原始 YAML 文本（包含起止 `---`）
6. 首行 `---` 但没有结束 `---` 的文档不会被误识别（INV-7 回退）

## 影响范围

### 新增文件

- `specs/模块-preview/10-Frontmatter渲染.md`（本次 PR 已创建）
- `plans/2026-04-13-Frontmatter渲染.md`（本文件）
- `tests/preview/FrontmatterRenderTest.cpp`（GoogleTest，覆盖 T-1 到 T-19）

### 修改文件

- `src/parser/MarkdownAst.h`
  - `AstNodeType` 新增 `Frontmatter`
  - `AstNode` 新增 `frontmatterEntries` / `frontmatterRawText` 字段
- `src/parser/MarkdownParser.h` / `.cpp`
  - 新增 `extractFrontmatter` 私有方法
  - `parse()` 入口先调用预处理剥离，再送 cmark-gfm
  - 对 cmark 产生的所有 AST 节点统一补偿 `startLine/endLine` 偏移
  - 把 `Frontmatter` 节点 unshift 到 AST 根节点的 children 头部
- `src/preview/PreviewLayout.h`
  - `LayoutBlock::Type` 新增 `Frontmatter`
  - `LayoutBlock` 新增 `frontmatterEntries` / `frontmatterKeyColumnWidth` / `frontmatterRawText` 字段
- `src/preview/PreviewLayout.cpp`
  - 新增 `layoutFrontmatter` 私有方法
  - `layoutBlock` 分发里加 `Frontmatter` 分支
  - 高度估计函数加对应分支（或直接在 `layoutFrontmatter` 里定稿）
- `src/preview/PreviewPainter.cpp`
  - 新增 `paintFrontmatter` 私有方法
  - `paintBlock` 分发加 `Frontmatter` 分支
  - 遵守 INV-8：所有字体通过 layout getter 获取，不硬编码
- `src/core/Theme.h` / `.cpp`
  - 新增 `accentColor` / `frontmatterBackground` / `frontmatterBorder` / `frontmatterKeyForeground` / `frontmatterValueForeground` 字段
  - `Theme::light()` 与 `Theme::dark()` 填充默认值（实际 accent 的读取由 MainWindow 或 Theme 工厂函数负责）
- `src/app/MainWindow.cpp`
  - 在构造 Theme 时，读取 `QGuiApplication::palette().color(QPalette::Highlight)` 填入 `accentColor`；未设置时 fallback 到 `#0078D4`
  - 主题切换时重新混合 `frontmatterBackground` / `frontmatterBorder`
- `src/preview/PreviewWidget.cpp`
  - 选区 → 文本转换遇到 `Frontmatter` 块时输出 `frontmatterRawText`（INV-13）
  - TOC 面板收集标题时跳过 `Frontmatter`（INV-15，应该自动满足，因为该块不是 Heading，但应有断言）

### Spec 变更

- `specs/模块-preview/10-Frontmatter渲染.md`（新增，本 PR）
- `specs/模块-preview/README.md`（新增清单行，本 PR）
- `specs/横切关注点/30-主题系统.md`（**不在本 Plan 直接改**，但 Plan 完成前需要由主线程追加 `accentColor` / frontmatter 系列字段。本 Plan 的 Step 3 对此作前置依赖标记）

### 禁区（本 Plan 不碰）

- `src/editor/*`
- `src/app/ShortcutsDialog.*`
- `specs/横切关注点/30-主题系统.md` / `60-国际化.md` / `80-字体系统.md`
- `specs/模块-preview/02-布局引擎.md` / `03-绘制管线.md`（已由其他 Plan 更新）
- `specs/模块-preview/09-链接点击与导航.md`
- `specs/模块-app/*`
- `CLAUDE.md` / `README.md`
- `translations/*`

## 步骤拆分

### Step 1：Parser 层——预处理剥离（纯函数，低风险）

- [ ] Step 1.1：在 `MarkdownAst.h` 添加 `Frontmatter` 枚举值和 `frontmatterEntries` / `frontmatterRawText` 字段
- [ ] Step 1.2：在 `MarkdownParser.h` 声明 `extractFrontmatter(const QString&, QString& outBody, AstNodePtr& outNode)`
- [ ] Step 1.3：实现 `extractFrontmatter`：按 §5.2 伪代码，处理 BOM、首行空白容忍、注释/空行忽略、首次冒号切分、无结束标记回退
- [ ] Step 1.4：`parse()` 调用 `extractFrontmatter`，成功则把 body 送 cmark，节点 unshift 到 children；记录 `frontmatterLineCount` 用于行号补偿
- [ ] Step 1.5：在 `convertNode` 递归时对 `startLine/endLine` 加 offset，保证预览 ↔ 编辑器的行映射不漂
- [ ] Step 1.6：写 parser 单元测试：首行是/不是 `---`、缺少结束 `---`、嵌入注释、嵌入数组、value 含冒号、BOM 前缀、空行忽略

### Step 2：Theme 层——新增字段（前置依赖，需等 30-主题系统.md 先动）

> **阻塞前置**：本 Step 开工前需要主线程先在 `specs/横切关注点/30-主题系统.md` 追加 `accentColor` / frontmatter 系列字段。本 Plan 只负责实现，不修改该 Spec。

- [ ] Step 2.1：`Theme.h` 新增 5 个字段
- [ ] Step 2.2：`Theme.cpp` 的 `light()` / `dark()` 填充默认 fallback 值（accent fallback 到 `#0078D4`）
- [ ] Step 2.3：在 `MainWindow` 构造 theme 时从 `QGuiApplication::palette()` 读 `QPalette::Highlight`
- [ ] Step 2.4：实现 `blendColor(accent, bg, ratio)` 工具函数（核心算法 §5.4）
- [ ] Step 2.5：`frontmatterBackground` / `frontmatterBorder` 在主题构造时通过 `blendColor` 生成
- [ ] Step 2.6：挂到系统 palette 变化信号上（Spec 30 的 INV-5），主题切换时重算混合色

### Step 3：Layout 层——布局计算（中风险，依赖 Theme 和字体度量）

- [ ] Step 3.1：`PreviewLayout.h` 新增 `LayoutBlock::Frontmatter` 枚举 + 字段
- [ ] Step 3.2：实现 `layoutFrontmatter(const AstNode*, qreal maxWidth)`，按 §5.3 伪代码计算列宽、总行数、总高度
- [ ] Step 3.3：列宽上限 50% 的边界检查
- [ ] Step 3.4：value 按字符换行的实现——不用 `QTextLayout`（会按词换行），自己手写字符循环 + 宽度累加
- [ ] Step 3.5：在 `buildFromAst` 分发里加 `Frontmatter` 分支
- [ ] Step 3.6：`QFontMetricsF` 严格带 `m_device`（`横切关注点/40-高DPI适配.md` INV-2）
- [ ] Step 3.7：布局单元测试：列宽计算、换行后总高度、50% 上限触发、超长 key ellipsis 宽度

### Step 4：Painter 层——绘制实现（中风险，视觉）

- [ ] Step 4.1：实现 `paintFrontmatter(QPainter*, const LayoutBlock&, absX, absY, ctx)`
- [ ] Step 4.2：圆角矩形背景 + 边框（与 `paintCodeBlock` 视觉一致，圆角半径由字体度量派生）
- [ ] Step 4.3：遍历 entries，绘制 key（第一列，宽度超限时 ellipsis）和 value（第二列，按字符换行，与布局保持严格一致）
- [ ] Step 4.4：所有 `QFont` 从 `PreviewLayout::monoFont()` 读取，**禁止**硬编码字号（INV-9，遵守 `03-绘制管线.md` INV-8）
- [ ] Step 4.5：`QFontMetricsF` 带 `p->device()`（INV-1）
- [ ] Step 4.6：`paintBlock` 分发加 `Frontmatter` 分支
- [ ] Step 4.7：绘制单元测试（pixel snapshot 或矩形命中验证）

### Step 5：Widget 层——选区/复制/TOC 联动（低风险，小改动）

- [ ] Step 5.1：`PreviewWidget` 在"选区 → 剪贴板文本"的转换代码里，遇到 `LayoutBlock::Frontmatter` 时直接 append `frontmatterRawText`，不拼接 run（INV-13）
- [ ] Step 5.2：TOC 面板收集逻辑断言跳过 `Frontmatter`（INV-15）——理论上自动满足，但加一条 assert 避免未来重构踩坑
- [ ] Step 5.3：手动验证查找（Ctrl+F）能命中 frontmatter 文本

### Step 6：端到端验证（高优先级）

- [ ] Step 6.1：准备测试文档 `tmp/frontmatter-smoke.md` 覆盖 T-1 到 T-19 所有场景（超长 key、超长 value、数组、注释、无 key 行、value 含冒号、缺结束标记……）
- [ ] Step 6.2：在 1x DPI 下人工观察视觉
- [ ] Step 6.3：在 1.25x / 1.5x DPI 下复测（T-17）
- [ ] Step 6.4：Ctrl+滚轮连续放大/缩小 4 次，确认 key/value 字号都跟随（T-13）
- [ ] Step 6.5：切换深/浅色主题，确认背景混合色即时更新（T-10/T-11/T-12）
- [ ] Step 6.6：选中整个 frontmatter，Ctrl+C → 粘贴到编辑器，确认是原始 YAML 文本（T-14）
- [ ] Step 6.7：打开首行不是 `---` 的文档，确认无回归（T-2）
- [ ] Step 6.8：打开文档中间含独立 `---` 的文档，确认仍是水平分割线（T-3）
- [ ] Step 6.9：打开首行 `---` 但缺结束的文档，确认回退为普通渲染（T-16）
- [ ] Step 6.10：Release 编译后运行 `installer\dist\SimpleMarkdown.exe` 冒烟

## 风险与回滚

### 风险 1：预处理剥离引发行号映射漂移

**影响**：用户点击预览区跳转到编辑器对应行时，跳到错位的行；搜索高亮错位。

**缓解**：
- Step 1.5 专门处理偏移补偿，`frontmatterLineCount` 必须精确（含起止 `---` 两行）
- Step 6 手动复测"点击预览区 Heading → 编辑器跳转对应行"的交互
- 写一个 parser 层单元测试：输入"frontmatter + 正文"，断言第一个正文 Heading 的 `startLine` 等于在原文中的真实行号

### 风险 2：cmark-gfm 误识首行 `---` 为 setext H2 底线

**影响**：如果预处理剥离有 bug 放过了 frontmatter，cmark 会把它当 setext 标题的底线，上面一行的 `id: ...` 会变成 H2 标题。

**缓解**：
- INV-1 严格化——起始 `---` 必须在文档首个**非空**行
- 单元测试 T-2：文档不是 frontmatter 时的样本必须多样（首行是 H1、空行开头、setext 标题…）

### 风险 3：`accentColor` 依赖 `specs/横切关注点/30-主题系统.md` 变更

**影响**：本 Plan 不能直接改 30-主题系统.md，必须等主线程先追加字段。如果主线程未及时响应，Step 2 阻塞。

**缓解**：
- 本 Plan status 先是 draft，待主线程完成主题系统扩展后升级为 in_progress
- 临时方案：可以在 `Theme.h` 本地直接加字段并在 Theme::light/dark 里硬编码 fallback，后续再与主线程的主题系统集成（但违反"spec 先行"，不推荐）

### 风险 4：value 按字符换行在 CJK 字符下视觉突兀

**影响**：中文 value（例如 `owners: 张三, 李四`）按字符换行可能把"张三"从中间切开。

**缓解**：
- 先按字符换行做 MVP（INV-12），视觉差但严格对齐
- 后续 Spec 可扩展为"优先 CJK 边界，fallback 任意字符"
- 在 Step 6.1 的测试样本里加含中文的 value，人审决定是否可接受

### 风险 5：50% 混合色在深色主题下不好看

**影响**：深色 accent + 深色 bg 混合后可能变成"稍浅一点的深灰"，辨识度低。

**缓解**：
- 参照 §8.5，若人审觉得对比度不够，把深色模式的混合比例调到 0.6 或 0.7
- INV-8 规定"50%"，但这是规范层面的语义——若需调整，本 Plan 会同步更新 Spec（改 INV-8 的比例数字）

### 回滚方案

本 Plan 的改动分布在 parser、layout、painter、theme、widget 多个模块。主要风险集中在 parser 的预处理剥离——如果线上发现回归，可以做**按扩展名灰度**或**开关**回滚：

1. 在 `MarkdownParser` 加一个 `bool m_frontmatterEnabled = true` 开关
2. 线上问题时全局 `false`，parser 直接跳过预处理 → 文档按 cmark 原样处理，行为回到 Plan 之前
3. 其余 layout/painter/theme 的改动是纯新增分支，关闭后这些新分支不会被触发，无需单独回滚

如果问题是 Theme 字段本身（例如 palette 读取崩溃），`git revert` 对应 commit 即可。

## 验证清单

- [ ] Spec INV-1 到 INV-15 在代码中都有对应实现点（review 时逐条核对；INV-16 不在 MVP）
- [ ] Spec T-1 到 T-19 手动验证全部通过
- [ ] 1x / 1.25x / 1.5x DPI 三档屏幕测试（见 `CLAUDE.md` 测试清单）
- [ ] 深色 + 浅色两种主题都验证
- [ ] `tests/preview/FrontmatterRenderTest.cpp` 全部通过
- [ ] `grep -En 'QFont[[:space:]]*\("[^"]+"[[:space:]]*,[[:space:]]*[0-9]' src/preview/PreviewPainter.cpp` 没有新增违规
- [ ] `grep -rn 'QFontMetricsF(' src/preview/PreviewPainter.cpp` 新增调用都带 `p->device()`
- [ ] 行号映射回归测试：点击预览区 Heading → 编辑器光标跳到正确行
- [ ] Ctrl+F 能搜索到 frontmatter 内文本
- [ ] `CHANGELOG.md` 加 `feat: 预览区 YAML frontmatter 渲染为信息表样式`
- [ ] Spec `status` 从 `draft` 升级为 `stable`，`last_reviewed` 更新
- [ ] 本 Plan `status` 从 `draft` → `in_progress` → `completed`，一周后归档到 `plans/归档/`

## 不在本 Plan 范围

- frontmatter 内嵌套 YAML（数组/对象/多行字符串）的完整语法支持
- frontmatter 内 value 是可点击链接（Spec INV-16 的 future）
- frontmatter 的 key/value 可编辑（预览区是只读的，编辑仍走编辑器）
- 自定义 frontmatter 的视觉主题（例如用户自选 accent）
- TOML / JSON frontmatter（只做 YAML 样式）
- 跨文档 frontmatter 索引/搜索面板

以上留作后续独立 Spec / Plan。
