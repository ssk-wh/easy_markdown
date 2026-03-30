# 打包诊断和修复指南

## 根本原因分析

### 问题
安装包中的 SimpleMarkdown.exe 无法启动

### 根本原因
**dist 目录中的 exe 是 3 月 27 日编译的旧版本**，不包含 3 月 30 日的最新代码修复

```
timeline：
- 2026-03-27 15:30  → build_tools/app/SimpleMarkdown.exe 编译（旧版本）
- 2026-03-30 10:35  → 收集依赖到 dist/ 时，copy 旧版本 exe
- 2026-03-30 14:21  → NSIS 打包，打包了旧版本 exe
- ⚠️ 期间的代码改动（高 DPI 修复、标记功能、坐标系统）未被重新编译
```

### 为什么旧版本无法启动？
可能原因（需要在 Windows 上运行看具体错误）：
1. 旧版本本身有 bug
2. 依赖库版本不匹配
3. 缺少必要的动态链接库
4. 运行环境差异

## 修复步骤（用户在 Windows 上完成）

### Step 1: 重新编译最新代码

```batch
cd D:\iflytek_projects\simple_markdown

REM 清理旧的编译
build_on_win.bat clean

REM 编译最新版本
build_on_win.bat release

REM 验证编译成功
dir /b build\app\SimpleMarkdown.exe
REM 应该显示文件存在
```

**关键**：编译出的 exe 时间戳应该接近当前时间（不是 3 月 27 日）

### Step 2: 重新收集依赖

```bash
cd D:\iflytek_projects\simple_markdown

REM 运行收集脚本，会自动 copy 最新编译的 exe
python installer\collect_dist.py

REM 验证：
REM 1. 检查 dist\SimpleMarkdown.exe 的时间戳（应该接近当前时间）
REM 2. 确保所有 DLL 都存在
dir installer\dist
```

### Step 3: 重新打包安装程序

```bash
cd D:\iflytek_projects\simple_markdown\installer
makensis SimpleMarkdown.nsi

REM 验证新安装包生成
dir /b SimpleMarkdown-0.1.0-Setup.exe
```

### Step 4: 测试新安装包

```batch
REM 运行新生成的安装包
SimpleMarkdown-0.1.0-Setup.exe

REM 或在 dist 目录直接测试 exe
cd installer\dist
SimpleMarkdown.exe

REM 应该能够启动并显示应用界面
```

## 验证检查清单

编译后，必须验证以下时间戳：

| 文件 | 时间 | 说明 |
|------|------|------|
| build/app/SimpleMarkdown.exe | 应为最新 | 重新编译的产物 |
| installer/dist/SimpleMarkdown.exe | 应为最新 | copy 后应与上面一致 |
| installer/SimpleMarkdown-0.1.0-Setup.exe | 应为最新 | 新打包的安装程序 |

**如果时间戳是 3 月 27 日**，说明没有使用最新编译的 exe，问题还会出现。

## 为什么之前没发现这个问题？

1. **之前打包用的 build_tools 中的 exe** - 是 3 月 27 日编译的
2. **代码修改后没有立即重新编译** - 在 bash 环境中没有 MSVC 编译器
3. **打包时没有验证 exe 时间戳** - 应该检查是否为最新编译

## 规范化修复流程

为了避免以后再出现这个问题，建立以下规范：

### 规范 1：编译 → 打包流程清单

```
□ 代码修改完成
□ 本地运行 build_on_win.bat release
  - 检查：build\app\SimpleMarkdown.exe 时间戳为最新
□ 运行 python installer\collect_dist.py
  - 检查：installer\dist\SimpleMarkdown.exe 时间戳与上面一致
□ 运行 NSIS 打包：makensis installer\SimpleMarkdown.nsi
  - 检查：SimpleMarkdown-0.1.0-Setup.exe 生成
□ 测试安装包：
  - 运行 installer\dist\SimpleMarkdown.exe
  - 验证应用启动并正常工作
□ 提交 git：
  - 包含代码改动
  - 可选：更新 CHANGELOG.md
```

### 规范 2：自动化验证脚本

创建 `verify_packaging.bat`：

```batch
@echo off
setlocal enabledelayedexpansion

REM 检查 exe 时间戳
for /F %%A in ('powershell -Command "(Get-Item 'build\app\SimpleMarkdown.exe').LastWriteTime.ToString('yyyy-MM-dd HH:mm')"') do set BUILD_TIME=%%A
for /F %%A in ('powershell -Command "(Get-Item 'installer\dist\SimpleMarkdown.exe').LastWriteTime.ToString('yyyy-MM-dd HH:mm')"') do set DIST_TIME=%%A

echo Build time: %BUILD_TIME%
echo Dist time:  %DIST_TIME%

if "%BUILD_TIME%"=="%DIST_TIME%" (
    echo OK: dist 中的 exe 是最新编译的
    exit /b 0
) else (
    echo ERROR: dist 中的 exe 已过期，需要重新运行 collect_dist.py
    exit /b 1
)
```

### 规范 3：CHANGELOG 记录

修改时应立即更新版本号和 CHANGELOG：

```markdown
## [0.1.1] - 2026-03-30

### Fixed
- 列表序号基线对齐（坐标系统统一）
- 高 DPI 屏幕行间距不均匀（高度估计改进）
- DPI 切换时块重合问题（混合字体高度调整）

### Added
- 文本标记功能（荧光笔式高亮）
- TOC 中显示标记条目

### Changed
- 高度估计算法（考虑混合字体，保证 DPI 一致性）
```

然后打包时使用新版本号：
```
APP_VERSION=0.1.1
OUTPUT=SimpleMarkdown-0.1.1-Setup.exe
```

## 关键教训

1. **每次代码改动后必须重新编译** - 不能使用旧的二进制文件打包
2. **时间戳是版本检查的金指标** - 3 月 27 日的 exe + 3 月 30 日的代码 = 不匹配
3. **打包前要验证** - 不仅检查文件存在，还要检查时间戳
4. **多层验证** - build → dist → installer 每一层都检查

---

**立即行动**：运行上述 Step 1-4，生成最新的安装包
