# EasyMarkdown

轻量、高性能的跨平台 Markdown 编辑器。

## 特性

- 分屏模式：左侧 Markdown 源码编辑，右侧实时渲染预览
- 全自绘渲染（QPainter），极低资源占用
- 多 Tab 支持
- 拖拽文件到窗口自动打开
- 代码块语法高亮
- GFM 表格、图片支持
- 搜索替换
- 亮色/暗色主题
- 最近文件列表

## 构建

### 前置依赖

- C++17 编译器（MSVC 2019+ / GCC 9+ / Clang 10+）
- Qt 5.15
- CMake >= 3.16

### 编译步骤

```bash
git clone --recursive <repo-url>
cd easy_markdown
cmake -S . -B build
cmake --build build
```

### 运行测试

```bash
cd build
ctest -C Debug --output-on-failure
```

## 技术栈

- C++17 / Qt 5.15 / CMake
- cmark-gfm (Markdown 解析)
- KSyntaxHighlighting (代码高亮)
- Google Test (单元测试)
