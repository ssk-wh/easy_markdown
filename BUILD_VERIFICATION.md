# 编译和验证指南

## 当前状态

✅ **代码修复已完成并在代码库中：**

### 1. 列表序号对齐修复（PreviewPainter.cpp）
- **位置**：第 172-194 行（List 块处理）
- **修改**：确保列表项使用正确的绝对坐标 `itemAbsY = absY + child.bounds.y()`
- **相同修复应用于**：BlockQuote 块（第 165 行）、Table 块（第 211 行）
- **注释标记**：第 176-178 行有详细说明

### 2. 高 DPI 行间距修复（PreviewLayout.cpp）
- **位置**：第 372-399 行（estimateParagraphHeight 函数）
- **修改**：使用 `m_lineHeight` 而不是动态调整每个 run 的高度
- **注释标记**：第 377-382 行标注为 `[高 DPI 修复]`

## 编译步骤

### 前置条件检查

在开始编译前，请确保已安装：

```batch
REM 检查 Visual Studio 或编译工具链
REM 项目使用 MSVC（C++ 编译器），需要：
REM - Visual Studio 2017 或更高版本，或
REM - Build Tools for Visual Studio

REM 检查 Qt5 安装
echo %Qt5_DIR%
REM 应该输出 Qt5 安装路径，如 D:\Qt\Qt5.12.9\5.12.9\msvc2017_64
```

### 编译命令

```batch
cd D:\iflytek_projects\simple_markdown

REM 方法 1: 使用改进的编译脚本（推荐）
build_on_win.bat release

REM 或者方法 2: 手动编译
REM (如果脚本失败，这提供了更多控制)

REM 2a. 设置 MSVC 环境（需要找到 vcvarsall.bat）
REM 通常位于：
REM C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat
REM C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat
REM D:\ 盘上如果有安装也会生效

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM 2b. 清理旧缓存（如果之前失败）
rmdir /s /q build
mkdir build

REM 2c. 运行 CMake 配置
cd build
cmake .. -G "NMake Makefiles" -DCMAKE_PREFIX_PATH="D:\Qt\Qt5.12.9\5.12.9\msvc2017_64"

REM 2d. 编译
nmake
```

### 编译可能遇到的问题

| 问题 | 解决方案 |
|------|--------|
| "cl.exe 找不到" | 需要运行 vcvarsall.bat 设置 MSVC 环境 |
| "Qt5 找不到" | 检查 Qt5_DIR 是否设置正确，或在 cmake 命令中使用 `-DCMAKE_PREFIX_PATH` |
| "NMake 找不到" | 需要安装 Visual Studio Build Tools 或 MSVC |
| CMakeCache.txt 错误 | 运行 `build_on_win.bat clean` 清理缓存 |

### 编译成功的标志

```
================================================
   Build succeeded: build\app\SimpleMarkdown.exe
   Size: XXXXX bytes
================================================
```

## 验证步骤

编译成功后，按照 VERIFICATION_STEPS.md 中的步骤进行手动验证。

### 关键验证项

#### A 屏（1x DPI）验证

打开 test_list.md（见 VERIFICATION_STEPS.md 中的模板）：

```
build\app\SimpleMarkdown.exe test_list.md
```

检查项：
- [ ] 列表序号 (1. 2. 3.) 与文本基线对齐，序号不偏下
- [ ] 行间距均匀，没有某行特别大的间距
- [ ] 嵌套列表结构正确显示

#### B 屏（1.5x DPI）验证

1. 在 A 屏（1x DPI）打开应用
2. 将窗口拖动到 B 屏（1.5x DPI）
3. 观察：
   - [ ] 序号仍然与文本对齐（不应该突然偏下）
   - [ ] 行间距不增大（不应该看起来"空了一行"）
   - [ ] 文本清晰可读

### 如果验证失败

1. 检查代码中的注释是否存在（搜索 `[高 DPI 修复]` 或 `itemAbsY`）
2. 运行 `git log --oneline -3` 确认提交包含修复
3. 清理缓存重新编译：
   ```batch
   build_on_win.bat clean
   build_on_win.bat release
   ```

## 相关文件

- **FIX_SUMMARY.md** - 修复的技术细节和根本原因分析
- **VERIFICATION_STEPS.md** - 详细的手动测试步骤
- **CLAUDE.md** - 项目经验记录和最佳实践

## 下一步

验证成功后，可以考虑：
1. 将修复提交到版本控制（如果尚未提交）
2. 投入时间实现自动化测试（见 tests/ 目录）
3. 制作安装包（使用 `/nsis-pack`）
