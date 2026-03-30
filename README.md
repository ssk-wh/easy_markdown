# SimpleMarkdown

轻量、高性能的跨平台 Markdown 编辑器，基于 C++17 / Qt 5.15 全自绘渲染。

## 界面预览

应用采用分屏布局：左侧为 Markdown 源码编辑区（带行号和语法高亮），右侧为实时渲染预览区。顶部支持多 Tab 同时打开多个文件，菜单栏提供文件操作和主题切换。

## 功能特性

### 编辑器
- 基于 QPainter 全自绘文本编辑器，极低资源占用
- 行号显示 + Markdown 语法高亮（标题、粗体、斜体、代码、链接、列表、引用）
- 光标闪烁、选区高亮
- 撤销/重做（Ctrl+Z/Y）、剪切/复制/粘贴（Ctrl+X/C/V）、全选（Ctrl+A）
- Tab 缩进 / Shift+Tab 反缩进、Enter 自动缩进
- 搜索替换（Ctrl+F / Ctrl+H）
- IME 中文输入支持

### 预览
- 基于 QPainter 全自绘 Markdown 预览渲染
- 支持：标题（H1-H6）、段落、粗体、斜体、行内代码、围栏代码块、引用块、有序/无序列表、GFM 表格、本地图片、分割线、链接、删除线
- 编辑后 30ms 防抖自动刷新
- 编辑器-预览滚动同步

### 多 Tab 与文件管理
- 同时打开多个文件，Tab 显示文件名和修改标记
- 新建、打开、保存、另存为
- 拖拽 .md/.markdown/.txt 文件到窗口自动打开
- 最近文件列表（最多 10 个）

### 主题
- 亮色 / 暗色主题，菜单一键切换

## 性能指标

| 指标 | 目标 |
|------|------|
| 冷启动 | < 0.5 秒 |
| 空载内存 | < 30 MB |
| 打字延迟 | < 3 ms |
| 预览更新 | < 80 ms |

## 项目结构

```
simple_markdown/
├── app/                    # 应用入口（main、MainWindow）
├── src/
│   ├── core/               # 核心数据模型（PieceTable、Document、UndoStack、Selection、Theme）
│   ├── editor/             # 自绘编辑器（EditorWidget、布局、绘制、输入、语法高亮、搜索栏）
│   ├── parser/             # Markdown 解析（cmark-gfm 封装、AST、ParseScheduler）
│   ├── preview/            # 自绘预览（PreviewWidget、布局、绘制、图片缓存、代码块渲染）
│   └── sync/               # 编辑器-预览滚动同步
├── 3rdparty/               # 第三方库（cmark-gfm、googletest）
├── docs/                   # 项目文档
│   ├── requirements.md     # 需求文档
│   ├── architecture.md     # 架构文档
│   └── build.md            # 构建说明
├── installer/              # 打包配置
│   └── dist/               # 打包产物（.exe、DLL 等）
├── resources/              # 资源文件（图标、样式表等）
├── tools/                  # 工具脚本
├── debian/                 # Debian 包配置
├── .github/                # GitHub Actions 工作流配置
├── .claude/                # Claude Code 本地配置
└── .superpowers/           # Claude Code superpowers 配置
```

## 文件说明

### 根目录文件

#### 编译和打包

| 文件 | 说明 |
|------|------|
| `CMakeLists.txt` | CMake 构建配置（依赖、源文件、链接库） |
| `build_on_win.bat` | Windows 一键编译脚本（发布版本；自动复制 Qt DLL） |
| `build_on_linux.sh` | Linux 一键编译脚本 |
| `build_pack_on_win.bat` | Windows 编译+打包脚本（生成 .exe 和 .nsi） |
| `collect_dist.py` | 收集编译产物和依赖库到 `installer/dist/`（NSIS 打包前必须执行） |
| `copy_qt_dlls.py` | 自动复制 Qt 运行时库到编译目录 |
| `SimpleMarkdown.nsi` | NSIS 安装包脚本（定义安装流程、快捷方式、卸载逻辑） |

#### 配置文件

| 文件 | 说明 |
|------|------|
| `.nsis-config.json` | NSIS 打包配置（版本号、输出路径等） |
| `.gitignore` | Git 忽略规则（编译产物、临时文件等） |
| `.gitmodules` | Git 子模块配置（3rdparty 库） |
| `.claudeignore` | Claude Code 忽略规则（避免扫描构建产物、依赖库） |

#### 文档

| 文件 | 说明 |
|------|------|
| `README.md` | 项目总览（功能、构建、技术栈） |
| `CHANGELOG.md` | 版本变更日志 |
| `CLAUDE.md` | 开发指南（DPI 适配、坐标系统、打包规范、已知问题） |

### 核心目录

#### `src/` - 源代码

- **core/** - 核心数据结构和管理
  - PieceTable：高效文本数据结构
  - Document：文档模型
  - UndoStack：撤销/重做栈
  - Selection：文本选区
  - Theme：主题配置

- **editor/** - 编辑器模块
  - EditorWidget：文本编辑窗口
  - EditorLayout：编辑器布局计算（行、列、光标位置）
  - EditorPainter：基于 QPainter 的编辑器渲染
  - SyntaxHighlighter：Markdown 语法高亮
  - SearchBar：搜索替换面板

- **parser/** - Markdown 解析
  - MarkdownParser：cmark-gfm 的 C++ 包装
  - AST 节点定义（Block、Inline）
  - ParseScheduler：后台解析调度器

- **preview/** - 预览模块
  - PreviewWidget：预览窗口
  - PreviewLayout：预览布局计算（块高度、坐标）
  - PreviewPainter：基于 QPainter 的预览渲染
  - ImageCache：图片缓存管理

- **sync/** - 滚动同步
  - EditorPreviewSync：编辑器-预览位置映射

#### `app/` - 应用入口
- main.cpp：应用入口
- MainWindow：主窗口（多标签页、菜单栏、布局管理）

#### `3rdparty/` - 第三方库
- **cmark-gfm/** - GitHub Flavored Markdown 解析器
- **googletest/** - 单元测试框架

#### `docs/` - 文档
- requirements.md：需求规格说明
- architecture.md：架构设计文档
- build.md：详细构建步骤

#### `installer/` - 打包
- collect_dist.py：依赖收集脚本
- SimpleMarkdown.nsi：NSIS 脚本
- dist/：打包产物目录
  - SimpleMarkdown.exe：应用可执行文件
  - *.dll：Qt 和系统依赖库
  - platforms/qwindows.dll：Qt 平台插件

#### `resources/` - 资源文件
- 应用图标、样式表、配置文件

#### `tools/` - 工具脚本
- 构建辅助工具、测试脚本

#### `debian/` - Linux 打包
- Debian/Ubuntu 包配置文件

### 配置目录

#### `.github/` - GitHub Actions
- 持续集成工作流（编译、测试、发布）

#### `.claude/` - Claude Code 本地配置
- task.md：项目任务列表（待办/进行中/已完成）
- 其他本地开发配置

#### `.superpowers/` - Claude Code superpowers
- 自定义开发工作流和脚本配置

## 构建

### 前置依赖
- C++17 编译器（MSVC 2019+ / GCC 9+ / Clang 10+）
- Qt 5.15
- CMake >= 3.16

### 编译步骤

```bash
git clone --recursive <repo-url>
cd simple_markdown
cmake -S . -B build
cmake --build build --config Release
```

### 运行测试

```bash
cd build
ctest -C Release --output-on-failure
```

详细构建说明参见 [docs/build.md](docs/build.md)。

## 技术栈

- C++17 / Qt 5.15 / CMake
- cmark-gfm（Markdown 解析）
- Google Test（单元测试）

## 文档

- [需求文档](docs/requirements.md)
- [架构文档](docs/architecture.md)
- [构建说明](docs/build.md)
