# SimpleMarkdown

轻量、高性能的跨平台 Markdown 编辑器，基于 C++17 / Qt 5 全自绘渲染。

## 功能特性

### 编辑器
- 基于 QPainter 全自绘文本编辑器，极低资源占用
- 行号显示 + Markdown 语法高亮（标题、粗体、斜体、代码、链接、列表、引用）
- 撤销/重做、剪切/复制/粘贴、全选
- Tab 缩进 / Shift+Tab 反缩进、Enter 自动缩进
- Markdown 格式化快捷键（Ctrl+B/I/K/D/E）
- 搜索替换（Ctrl+F / Ctrl+H）
- IME 中文输入支持

### 预览
- 基于 QPainter 全自绘 Markdown 预览渲染
- 支持：标题（H1-H6）、段落、粗体、斜体、行内代码、围栏代码块、引用块、有序/无序列表、GFM 表格、本地图片、分割线、链接、删除线
- 编辑后 30ms 防抖自动刷新
- 编辑器-预览滚动同步
- 内容标记功能（黄色荧光笔效果）

### 多 Tab 与文件管理
- 同时打开多个文件，Tab 显示文件名和修改标记（*）
- 自定义 Tab 关闭按钮，适配深色/浅色主题
- Tab 右键菜单（关闭当前/其他/左侧/右侧）
- 拖拽 .md/.markdown/.txt 文件到窗口自动打开
- 最近文件列表（显示完整路径）
- 文档外部修改检测，弹窗提示重新加载
- 单实例模式
- 会话状态实时持久化

### 主题与界面
- 亮色 / 暗色主题，支持跟随系统
- 深色模式下所有弹窗标题栏自动跟随
- 目录（TOC）面板导航

## 项目结构

```
simple_markdown/
├── src/
│   ├── app/                # 应用入口（main、MainWindow、ChangelogDialog）
│   ├── core/               # 核心数据模型（PieceTable、Document、UndoStack、Selection、Theme）
│   ├── editor/             # 自绘编辑器（布局、绘制、输入、语法高亮、搜索栏）
│   ├── parser/             # Markdown 解析（cmark-gfm 封装、AST、ParseScheduler）
│   ├── preview/            # 自绘预览（布局、绘制、图片缓存、代码块渲染）
│   └── sync/               # 编辑器-预览滚动同步
├── 3rdparty/               # 第三方库（cmark-gfm、googletest）
├── resources/              # 资源文件（图标等）
├── installer/              # Windows 安装包配置（NSIS）
├── debian/                 # Debian 打包配置
├── scripts/                # 工具脚本
├── docs/                   # 项目文档
└── tools/                  # 辅助工具
```

## 构建与打包

### 前置依赖
- C++17 编译器（MSVC 2019+ / GCC 9+ / Clang 10+）
- Qt 5.12+
- CMake >= 3.16

### Windows

```batch
build_on_win.bat release    # 编译
pack_on_win.bat             # 收集依赖 + NSIS 打包
```

### Linux

```bash
./build_on_linux.sh release  # 编译
./pack_on_linux.sh           # 生成 debian changelog + 构建 deb 包
```

### 手动编译

```bash
git clone --recursive <repo-url>
cd simple_markdown
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 技术栈

- C++17 / Qt 5 / CMake
- cmark-gfm（Markdown 解析）
- Google Test（单元测试）

## 文档

- [需求文档](docs/requirements.md)
- [架构文档](docs/architecture.md)
- [构建说明](docs/build.md)
- [变更日志](CHANGELOG.md)
