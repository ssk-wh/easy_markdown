# SimpleMarkdown UI 现代化设计规格

## 概述

对 SimpleMarkdown 编辑器进行全面界面美化，采用"柔和科技"设计语言：低饱和度配色、半透明质感、圆润形态。同时新增跟随系统深色模式功能。

**约束:** 项目使用 Qt 5 (`find_package(Qt5)`)，所有方案必须兼容 Qt 5.12+。

## 设计语言

### 配色体系

**主题色 (Accent):** Indigo
- Light 模式: `#6366f1`
- Dark 模式: `#a5b4fc`

**代码强调色:** Pink/Purple
- Light 模式: `#e879a0`
- Dark 模式: `#f0abfc`

### Light 主题色板

| 用途 | 字段名 | 颜色值 |
|------|--------|--------|
| 窗口/应用背景 | `windowBg` | `#f2f3f7` |
| 编辑器背景 | `editorBg` | `#fafbfe` |
| 编辑器正文 | `editorFg` | `#1d1d1f` |
| 当前行高亮 | `editorCurrentLine` | `rgba(99,102,241,0.04)` |
| 选区色 | `editorSelection` | `rgba(99,102,241,0.15)` |
| 行号 | `editorLineNumber` | `#c5c6cc` |
| 活跃行号 | `editorLineNumberActive` | `#6366f1` |
| gutter 背景 | `editorGutterBg` | `#f5f6fa` |
| gutter 分隔线 | `editorGutterLine` | `rgba(0,0,0,0.04)` |
| 光标 | `editorCursor` | `#6366f1` |
| 搜索匹配 | `editorSearchMatch` | `rgba(250,204,21,0.25)` |
| 预编辑背景 | `editorPreeditBg` | `rgba(99,102,241,0.08)` |
| 标签栏背景 | `tabBarBg` | `rgba(255,255,255,0.72)` |
| 标签栏分隔线 | `tabBarBorder` | `rgba(0,0,0,0.06)` |
| 活跃标签背景 | `tabActiveBg` | `rgba(99,102,241,0.1)` |
| 活跃标签文字 | `tabActiveFg` | `#6366f1` |
| 非活跃标签文字 | `tabInactiveFg` | `#8e8e93` |
| 搜索栏背景 | `searchBarBg` | `rgba(255,255,255,0.85)` |
| 搜索栏边框 | `searchBarBorder` | `rgba(0,0,0,0.08)` |
| 搜索栏输入背景 | `searchBarInputBg` | `rgba(0,0,0,0.03)` |
| 分割条颜色 | `splitterHandle` | `rgba(0,0,0,0.06)` |
| 分割条悬停色 | `splitterHandleHover` | `rgba(99,102,241,0.3)` |

**语法高亮 (Light):**

| 元素 | 字段名 | 前景色 | 背景色 |
|------|--------|--------|--------|
| 标题 | `syntaxHeading` | `#6366f1` | — |
| 行内代码 | `syntaxCode` / `syntaxCodeBg` | `#e879a0` | `rgba(232,121,160,0.08)` |
| 代码块内容 | `syntaxCodeBlock` / `syntaxCodeBlockBg` | `#16a34a` | `rgba(22,163,74,0.04)` |
| 围栏标记 | `syntaxFence` / `syntaxFenceBg` | `#a1a1aa` | `rgba(0,0,0,0.02)` |
| 链接 | `syntaxLink` | `#6366f1` | — |
| 列表标记 | `syntaxList` | `#a855f7` | — |
| 引用 | `syntaxBlockQuote` | `#8b8fa3` | — |

**预览区 (Light):**

| 元素 | 字段名 | 值 |
|------|--------|-----|
| 背景 | `previewBg` | `#ffffff` |
| 正文色 | `previewFg` | `#3a3a3c` |
| 标题色 | `previewHeading` | `#1d1d1f` |
| 标题分隔线 | `previewHeadingSeparator` | `rgba(99,102,241,0.2)` |
| 链接色 | `previewLink` | `#6366f1` |
| 内联代码背景 | `previewCodeBg` | `rgba(99,102,241,0.06)` |
| 内联代码前景 | `previewCodeFg` | `#6366f1` |
| 代码块背景 | `previewCodeBlockBg` | `#f8f9fc` |
| 代码块边框 | `previewCodeBorder` | `rgba(0,0,0,0.06)` |
| 代码块文字 | (复用 `previewFg`) | — |
| 引用左边框 | `previewBlockQuoteBorder` | `rgba(99,102,241,0.3)` |
| 引用背景 | `previewBlockQuoteBg` | `rgba(99,102,241,0.03)` |
| 表格边框 | `previewTableBorder` | `rgba(0,0,0,0.08)` |
| 表头背景 | `previewTableHeaderBg` | `#f5f6fa` |
| 水平线 | `previewHr` | `rgba(0,0,0,0.08)` |
| 图片占位背景 | `previewImagePlaceholderBg` | `#f5f6fa` |
| 图片占位边框 | `previewImagePlaceholderBorder` | `rgba(0,0,0,0.08)` |
| 图片占位文字 | `previewImagePlaceholderText` | `#8e8e93` |

### Dark 主题色板

| 用途 | 字段名 | 颜色值 |
|------|--------|--------|
| 窗口/应用背景 | `windowBg` | `#131318` |
| 编辑器背景 | `editorBg` | `#1a1a22` |
| 编辑器正文 | `editorFg` | `#e5e5ea` |
| 当前行高亮 | `editorCurrentLine` | `rgba(129,140,248,0.06)` |
| 选区色 | `editorSelection` | `rgba(129,140,248,0.2)` |
| 行号 | `editorLineNumber` | `#3a3a48` |
| 活跃行号 | `editorLineNumberActive` | `#a5b4fc` |
| gutter 背景 | `editorGutterBg` | `#1a1a22` |
| gutter 分隔线 | `editorGutterLine` | `rgba(255,255,255,0.04)` |
| 光标 | `editorCursor` | `#a5b4fc` |
| 搜索匹配 | `editorSearchMatch` | `rgba(250,204,21,0.15)` |
| 预编辑背景 | `editorPreeditBg` | `rgba(129,140,248,0.12)` |
| 标签栏背景 | `tabBarBg` | `rgba(30,30,38,0.72)` |
| 标签栏分隔线 | `tabBarBorder` | `rgba(255,255,255,0.06)` |
| 活跃标签背景 | `tabActiveBg` | `rgba(129,140,248,0.15)` |
| 活跃标签文字 | `tabActiveFg` | `#a5b4fc` |
| 非活跃标签文字 | `tabInactiveFg` | `#5a5a6e` |
| 搜索栏背景 | `searchBarBg` | `rgba(30,30,38,0.85)` |
| 搜索栏边框 | `searchBarBorder` | `rgba(255,255,255,0.08)` |
| 搜索栏输入背景 | `searchBarInputBg` | `rgba(255,255,255,0.06)` |
| 分割条颜色 | `splitterHandle` | `rgba(255,255,255,0.06)` |
| 分割条悬停色 | `splitterHandleHover` | `rgba(129,140,248,0.3)` |

**语法高亮 (Dark):**

| 元素 | 前景色 | 背景色 |
|------|--------|--------|
| 标题 | `#a5b4fc` | — |
| 行内代码 | `#f0abfc` | `rgba(240,171,252,0.1)` |
| 代码块内容 | `#4ade80` | `rgba(74,222,128,0.06)` |
| 围栏标记 | `#4a4a5c` | `rgba(255,255,255,0.02)` |
| 链接 | `#a5b4fc` | — |
| 列表标记 | `#c084fc` | — |
| 引用 | `#7a7a8e` | — |

**预览区 (Dark):**

| 元素 | 值 |
|------|-----|
| 背景 | `#16161e` |
| 正文色 | `#b0b0be` |
| 标题色 | `#f5f5f7` |
| 标题分隔线 | `rgba(129,140,248,0.25)` |
| 链接色 | `#a5b4fc` |
| 内联代码背景 | `rgba(129,140,248,0.1)` |
| 内联代码前景 | `#a5b4fc` |
| 代码块背景 | `rgba(255,255,255,0.04)` |
| 代码块边框 | `rgba(255,255,255,0.08)` |
| 引用左边框 | `rgba(129,140,248,0.3)` |
| 引用背景 | `rgba(129,140,248,0.04)` |
| 表格边框 | `rgba(255,255,255,0.08)` |
| 表头背景 | `rgba(255,255,255,0.04)` |
| 水平线 | `rgba(255,255,255,0.08)` |
| 图片占位背景 | `rgba(255,255,255,0.04)` |
| 图片占位边框 | `rgba(255,255,255,0.08)` |
| 图片占位文字 | `#5a5a6e` |

### 排版 (常量，不加入 Theme)

- **编辑器正文字号:** 11pt (等宽字体，由 QFontDatabase::systemFont 提供)
- **编辑器行高:** 默认 (QTextLayout 自动)
- **预览正文字号:** 10pt, Segoe UI
- **预览行高:** 1.5 (inline run) / 1.85 (段落间距由 layout 控制)
- **标题 letter-spacing:** -0.3px (仅预览区)
- **代码字体:** Consolas, 9pt
- **行号字体:** 比正文小 1pt

### 形态 (Shape)

- **标签页:** 圆角 8px 胶囊形
- **搜索栏外框:** 圆角 12px
- **搜索栏输入框:** 圆角 8px
- **代码块 (预览):** 圆角 8px
- **引用块 (预览):** 左边框 3px + 右侧圆角 8px
- **内联代码:** 圆角 4px
- **分割条宽度:** 1px (hover 时 3px)

## 功能变更

### 1. 主题模式三选一

View 菜单中 Light Theme / Dark Theme 改为三个选项：
- **Auto (Follow System)** — 默认选中，监听系统深色模式切换事件
- **Light**
- **Dark**

**实现方式 (Qt 5 兼容):**

在 `Theme` 中新增枚举:
```cpp
enum class ThemeMode { Auto, Light, Dark };
```

Auto 模式检测系统主题 (Windows):
- 读取注册表 `HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`
- 值为 0 = 深色模式, 值为 1 = 浅色模式
- 使用 `QTimer` 每 2 秒轮询注册表变化 (Qt 5 无原生深色模式信号)
- 轮询仅在 Auto 模式下启用，手动模式时停止定时器

```cpp
// Windows 注册表检测
#ifdef _WIN32
#include <QSettings>
bool isSystemDarkMode() {
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                   QSettings::NativeFormat);
    return reg.value("AppsUseLightTheme", 1).toInt() == 0;
}
#endif
```

### 2. QTabWidget 样式

通过 QSS 实现。`applyTheme()` 中根据当前 Theme 动态生成 QSS 字符串并 `setStyleSheet()`：

- `QTabWidget::pane` — 无边框, 背景透明
- `QTabBar` — 背景 `tabBarBg`, 底部边框 `tabBarBorder`
- `QTabBar::tab` — padding 5px 16px, 圆角 8px, 颜色 `tabInactiveFg`, 无边框
- `QTabBar::tab:selected` — 背景 `tabActiveBg`, 颜色 `tabActiveFg`, font-weight 500
- `QTabBar::close-button` — 主题适配的关闭按钮颜色

### 3. QSplitter 样式

- 宽度 1px, 背景 `splitterHandle`
- hover 时宽度 3px, 背景 `splitterHandleHover`
- 通过 QSS: `QSplitter::handle`

### 4. SearchBar 视觉升级

- 外框圆角增大到 12px
- 背景使用 `searchBarBg` (半透明色模拟毛玻璃，不使用真实高斯模糊，Qt 5 不支持 backdrop-filter)
- 阴影通过 `QGraphicsDropShadowEffect` 添加 (blur=24, color=rgba(0,0,0,0.06/0.3))
- 输入框圆角 8px, 背景 `searchBarInputBg`
- 按钮使用文字 + 统一圆角样式
- `setTheme()` 方法动态更新 QSS, 深色/浅色均适配

### 5. 编辑器区域

- gutter 分隔线从实线改为 `editorGutterLine` (极淡)
- 当前行高亮使用 `editorCurrentLine` (极淡主题色)
- 光标颜色使用 `editorCursor` (主题色)
- 活跃行号使用 `editorLineNumberActive` (主题色)
- 预编辑背景使用 `editorPreeditBg` (淡主题色, 不再是刺眼黄色)

### 6. 预览区域

- H1/H2 分隔线宽度改为 2px, 颜色 `previewHeadingSeparator`
- 代码块圆角增大到 8px (drawRoundedRect 参数)
- 引用块改为左边框 3px (`previewBlockQuoteBorder`) + 背景 (`previewBlockQuoteBg`) + 右侧圆角 8px
- 内联代码圆角 4px

### 7. 窗口背景

通过 QPalette 设置 MainWindow 的背景色:
```cpp
QPalette pal = palette();
pal.setColor(QPalette::Window, theme.windowBg);
setPalette(pal);
```

## 不变更的部分

- 编辑器核心逻辑 (PieceTable, Selection, UndoStack 等)
- 解析器和 AST
- 文件操作和拖放
- 滚动同步
- 快捷键
- 排版参数 (字号/行高保持当前值，不加入 Theme)
- SyntaxHighlighter 的高亮逻辑 (仅通过已有的 `setTheme` → `setupFormats` 路径更新颜色，不修改代码结构)

## 涉及文件

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `src/core/Theme.h` | 修改 | 新增 ThemeMode 枚举; 新增字段: windowBg, tabBarBg, tabBarBorder, tabActiveBg, tabActiveFg, tabInactiveFg, searchBarBg, searchBarBorder, searchBarInputBg, splitterHandle, splitterHandleHover, previewCodeBlockBg; 新增 editorLineNumberActive |
| `src/core/Theme.cpp` | 修改 | 重写 light()/dark() 全部配色值 |
| `app/MainWindow.h` | 修改 | 新增 ThemeMode 成员, 系统主题检测定时器, applyThemeMode() |
| `app/MainWindow.cpp` | 修改 | View 菜单三选一; Auto 模式注册表轮询; applyTheme 中生成 QSS (标签/分割条); 窗口背景 QPalette |
| `src/editor/EditorWidget.cpp` | 修改 | paintEvent: 活跃行号用主题色; gutter 分隔线用新颜色 |
| `src/editor/EditorPainter.cpp` | 修改 | 当前行高亮用新色; 光标用主题色 |
| `src/editor/SearchBar.cpp` | 修改 | 新增 setTheme() 方法; QSS 动态更新; 添加阴影效果 |
| `src/editor/SearchBar.h` | 修改 | 新增 setTheme(const Theme&) 声明 |
| `src/preview/PreviewPainter.cpp` | 修改 | 代码块圆角 8px; 引用块新样式; H1/H2 分隔线 2px |
| `src/editor/SyntaxHighlighter.cpp` | 无需修改 | 已通过 Theme 字段间接更新，无结构变更 |
| `src/preview/PreviewWidget.cpp` | 无需修改 | setTheme 路径已覆盖，背景色由 Theme.previewBg 控制 |
