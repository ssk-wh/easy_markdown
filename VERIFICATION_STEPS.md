# 修复验证步骤 (手动验证)

## 概述

本文档提供了验证列表序号对齐和高 DPI 行间距修复的具体步骤。

**修复内容：**
- ✅ 修复：统一布局与渲染的坐标系，修复嵌套块的位置错误
- ✅ 改进：build_on_win.bat 脚本，增强健壮性和错误检测

---

## 第一步：编译

### 1.1 清理旧缓存（如果之前编译失败过）

```batch
cd D:\iflytek_projects\simple_markdown
build_on_win.bat clean
```

### 1.2 编译应用

```batch
build_on_win.bat release
```

**预期结果：**
```
================================================
   Build succeeded: build\app\SimpleMarkdown.exe
   Size: XXXXX bytes
================================================
```

**如果失败：**
- 脚本会自动检测并清理错误的 CMakeCache.txt
- 如果仍然失败，运行：`build_on_win.bat --clean release`

---

## 第二步：验证修复

### 2.1 准备测试文件

创建一个测试 Markdown 文件 `test_list.md`：

```markdown
# 有序列表测试

1. 第一项
2. 第二项
3. 第三项

## 嵌套列表

- 无序项 A
  - 嵌套项 A1
  - 嵌套项 A2
- 无序项 B

## 引用中的列表

> 这是一个引用
>
> 1. 引用内的有序列表
> 2. 第二项
```

### 2.2 验证项清单

#### ✅ A 屏验证（1x DPI 标准屏）

在你的主屏（通常是 1x DPI）上运行：

```batch
D:\iflytek_projects\simple_markdown\build\app\SimpleMarkdown.exe test_list.md
```

**检查清单：**

- [ ] **列表序号对齐**
  - 打开应用后，查看有序列表（1. 2. 3.）
  - 序号与后面的文本是否在同一基线上
  - **预期：** 序号和文本对齐，没有序号偏下的现象

- [ ] **行间距均匀**
  - 检查列表项之间的间距
  - 每行之间的间距是否相等
  - **预期：** 行间距均匀，没有某一行特别高的情况

- [ ] **嵌套结构**
  - 检查引用内的列表和嵌套列表
  - 缩进是否正确，序号是否对齐
  - **预期：** 嵌套结构显示正确

#### ✅ B 屏验证（1.5x DPI 高 DPI 屏）

1. 在 A 屏（1x DPI）打开应用
2. **将窗口从 A 屏拖动到 B 屏（1.5x DPI）**
3. 在 B 屏上观察应用显示

**检查清单：**

- [ ] **序号仍然对齐**
  - 拖到 B 屏后，序号与文本是否仍然对齐
  - **预期：** 对齐状态保持不变

- [ ] **行间距不增大**
  - B 屏上的行间距与 A 屏上是否相同比例
  - 是否还有"空了一行"的现象
  - **预期：** 行间距正常，没有明显增大

- [ ] **文本清晰**
  - 文字是否清晰可读
  - 反引号（如果有代码）周围的空白是否合理
  - **预期：** 显示清晰，布局正确

---

## 第三步：问题排查

### 如果序号仍然不对齐

1. 确认编译使用了最新代码：
   ```batch
   git log --oneline -1
   ```
   应该看到：`fix: 统一布局与渲染的坐标系，修复嵌套块的位置错误`

2. 清理缓存重新编译：
   ```batch
   build_on_win.bat clean
   build_on_win.bat release
   ```

3. 检查代码修改是否生效：
   ```bash
   git diff HEAD~1 src/preview/PreviewPainter.cpp | grep -A 5 "itemAbsY"
   ```

### 如果行间距在高 DPI 屏增大

1. 确认高度估计修复被应用：
   ```bash
   git diff HEAD~1 src/preview/PreviewLayout.cpp | grep -A 3 "lineHeight"
   ```

2. 检查 updateMetrics 是否在 resize 时被调用：
   - 从 A 屏拖到 B 屏时，应用是否立即重新布局
   - **预期：** 是的，拖动过程中应该能看到变化

---

## 第四步：记录测试结果

### 如果验证全部通过 ✅

运行以下命令记录成功：

```bash
git log --oneline -5
# 应该看到修复相关的 commit

# 创建验证记录
echo "验证日期：$(date)" > VERIFICATION_PASSED.txt
echo "修复状态：所有测试通过" >> VERIFICATION_PASSED.txt
echo "" >> VERIFICATION_PASSED.txt
echo "检查清单：" >> VERIFICATION_PASSED.txt
echo "- A 屏：列表序号对齐 ✓" >> VERIFICATION_PASSED.txt
echo "- B 屏：高 DPI 行间距正常 ✓" >> VERIFICATION_PASSED.txt
echo "- 嵌套结构渲染正确 ✓" >> VERIFICATION_PASSED.txt
```

### 如果有问题 ⚠️

请记录：

```
问题描述：
- 具体现象是什么
- 在 A 屏还是 B 屏出现
- 列表内容是什么

重现步骤：
1. 打开应用，加载哪个文件
2. 观察哪个列表项
3. 看到什么结果

期望结果：
- 应该看到什么

实际结果：
- 实际看到什么
```

---

## 相关文件

- **代码修改：** `src/preview/PreviewPainter.cpp`, `src/preview/PreviewLayout.cpp`
- **编译脚本：** `build_on_win.bat`
- **修复文档：** `CLAUDE.md`, `FIX_SUMMARY.md`
- **验证规范：** `VERIFICATION.md`

---

## 总结

| 阶段 | 状态 | 说明 |
|------|------|------|
| 代码修改 | ✅ | 两个问题的修复已完成并提交 |
| 脚本改进 | ✅ | build_on_win.bat 已改进，可自动修复缓存问题 |
| **编译验证** | ⏳ | 需要在本地运行 `build_on_win.bat release` |
| **功能验证** | ⏳ | 需要按本文档步骤手动测试 |
| 自动化测试 | ⏳ | 等修复验证通过后再考虑 |

**下一步：** 在本地运行编译和验证步骤，反馈结果。
