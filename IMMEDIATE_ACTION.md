# 立即行动清单 - 安装包修复

## 问题总结

✅ **根本原因已确认**：安装包中的 exe 是 3 月 27 日编译的旧版本，不包含 3 月 30 日的最新代码修复

```
构建时间对比：
- build_tools/SimpleMarkdown.exe     → 2026-03-27 15:30  (旧)
- installer/dist/SimpleMarkdown.exe   → 2026-03-30 14:21  (copy 的旧版本)
- 最新代码修改提交时间                 → 2026-03-30      (未被编译)
```

**为什么无法启动**：打包的 exe 可能有 bug 或依赖缺失，需要用最新编译的版本

---

## 修复步骤（用户在 Windows 上执行）

### 命令序列（复制粘贴执行）

```batch
REM ====== Step 1: 重新编译最新代码 ======
cd D:\iflytek_projects\simple_markdown

REM 清理旧的编译产物
build_on_win.bat clean

REM 编译最新版本（包含 3 月 30 日的所有修复）
build_on_win.bat release

REM 验证：应该看到 "Build succeeded: build\app\SimpleMarkdown.exe"


REM ====== Step 2: 重新收集依赖到 dist ======
REM collect_dist.py 会自动 copy 最新编译的 exe
python installer\collect_dist.py

REM 验证：应该看到 "Collection complete!"


REM ====== Step 3: 重新打包 ======
cd installer
makensis SimpleMarkdown.nsi

REM 验证：应该看到 "Output: SimpleMarkdown-0.1.0-Setup.exe"


REM ====== Step 4: 测试新安装包 ======
cd ..
installer\dist\SimpleMarkdown.exe

REM 应该看到应用界面弹出
```

---

## 验证检查清单

完成上述步骤后，检查以下内容：

### 1. 时间戳检查

```batch
REM 查看 exe 时间戳，应该都是最新的（今天的日期）
dir /T:W /B build\app\SimpleMarkdown.exe
dir /T:W /B installer\dist\SimpleMarkdown.exe
dir /T:W /B installer\SimpleMarkdown-0.1.0-Setup.exe

REM 如果显示日期是 "03-27" 或更旧，说明没有重新编译，需要重复 Step 1
```

### 2. 应用启动测试

```batch
REM 直接运行最新的 exe
installer\dist\SimpleMarkdown.exe

REM 期望：
REM ✓ 应用窗口出现
REM ✓ 可以打开/编辑 Markdown 文件
REM ✓ 预览区显示正确（包含之前修复的列表对齐、高 DPI 等功能）
```

### 3. 安装包测试

```batch
REM 运行新生成的安装包
installer\SimpleMarkdown-0.1.0-Setup.exe

REM 按照安装向导完成安装
REM 期望：安装完成后能启动应用，正常显示界面
```

---

## 如果仍然无法启动

如果按照上述步骤操作后仍然无法启动，可能是以下原因：

| 现象 | 检查方法 |
|------|--------|
| exe 时间戳仍然是 3 月 27 日 | 编译可能失败，查看 build_on_win.bat 的输出 |
| dist 中的 exe 仍然是旧的 | 运行 `python installer\collect_dist.py` 后检查时间戳 |
| 应用启动但立即崩溃 | 运行时是否有弹窗报错（缺少 DLL、配置错误等） |
| 安装包包含文件损坏 | 重新运行 `collect_dist.py` 检查文件完整性 |

---

## 为什么需要这么多步骤？

这是编译系统的标准流程：

1. **编译** - 将源代码转换为 exe
2. **收集依赖** - 找出所有需要的 DLL 和资源文件
3. **打包** - 将 exe 和依赖打成一个安装程序

**关键**：每个步骤都依赖前一个步骤的输出
- 如果跳过第 1 步，打包时仍会使用旧 exe
- 如果跳过第 2 步，安装包会缺少某些 DLL

---

## 预防措施（防止以后再出现）

已添加到 CLAUDE.md 的规范：

```
✓ 编译后立即检查 exe 时间戳
✓ 收集依赖后检查 dist 中的 exe 时间戳
✓ 打包前运行 dist/SimpleMarkdown.exe 验证能启动
✓ 版本号变更应与代码修改同步
✓ 提交时包含 CHANGELOG 说明变更内容
```

详见 `PACKAGING_DIAGNOSTIC.md`

---

**预计耗时**：10-15 分钟（取决于网络和编译速度）

**完成后**：可以将 `installer/SimpleMarkdown-0.1.0-Setup.exe` 分发给用户安装使用

