# SimpleMarkdown 自我验证框架

本文档定义了一套完整的、可独立执行的验证机制，用于验证所有已知问题的修复情况。

---

## 快速开始

### 一键验证（推荐）

```bash
python tests/verify_all.py --gui
```

此命令将：
1. 启动应用程序
2. 依次执行所有测试用例
3. 生成验证报告 `tests/verification_report.json`

### 分步验证

```bash
python tests/verify_dpi_scaling.py        # 高 DPI 验证
python tests/verify_window_behavior.py    # 窗口行为
python tests/verify_font_metrics.py       # 字体度量
python tests/verify_list_alignment.py     # 列表对齐
```

---

## 验证用例总览

| ID | 问题描述 | 验证方式 | 期望结果 | 自动化 |
|----|--------|--------|--------|--------|
| V1 | 高 DPI 代码块下方空白 | 像素测量 | 无多余空白 | ✓ |
| V2 | 双击文本选中坐标偏差 | 坐标记录 | 选中准确 | ◐ |
| V3 | 反引号宽度过大 | 像素测量 | 宽度正常 | ✓ |
| V4 | 打开文件窗口未提升 | 行为检查 | 窗口在最前 | ◐ |
| V5 | 高 DPI 标题/列表间距 | 像素测量 | 间距均匀 | ✓ |
| V6 | 列表序号基线对齐 | 像素比对 | 序号和文本对齐 | ✓ |
| V7 | 标记功能完整性 | 功能测试 | 标记/清除正常 | ◐ |
| V8 | DPI 切换时刷新 | 行为检查 | 布局重新计算 | ◐ |

*`✓` = 完全自动化 | `◐` = 需要人工确认 | `-` = 手动验证*

---

## V1: 高 DPI 代码块下方空白

### 问题描述
在 1.5x DPI 屏上，代码块渲染后下方出现多余的空白区域（约 20-40% 行高）。

### 验证步骤

#### 自动化验证（推荐）

```bash
python tests/verify_dpi_scaling.py --test=codeblock_spacing
```

脚本将：
1. 生成测试 Markdown 文件（含多行代码块）
2. 在当前 DPI 下渲染
3. 通过 PIL 测量代码块的像素高度
4. 对比预期高度 vs 实际高度
5. 输出结果：`{ "codeblock_spacing": "PASS" | "FAIL", "variance": "±2%" }`

#### 手动验证（B 屏 1.5x DPI）

1. 打开 `tests/fixtures/dpi_test.md`
2. 观察代码块下方是否有明显空白
3. 在 A 屏（1x DPI）对比
   - A 屏代码块：正常，下方无多余空白
   - B 屏代码块：应该与 A 屏表现一致

### 期望结果

| DPI | 代码块高度 | 下方空白 | 状态 |
|-----|---------|--------|------|
| 1x | ~140px | 0 | ✓ |
| 1.25x | ~175px | 0 | ✓ |
| 1.5x | ~210px | 0 | ✓ |

### 验证代码

```python
# tests/verify_dpi_scaling.py - 高 DPI 代码块验证

import subprocess
import time
from pathlib import Path
from PIL import Image, ImageDraw
import json

def verify_codeblock_spacing():
    """验证高 DPI 下代码块高度计算是否正确"""

    # 1. 启动应用打开测试文件
    app = subprocess.Popen(['build/app/SimpleMarkdown.exe', 'tests/fixtures/dpi_codeblock.md'])
    time.sleep(3)  # 等待渲染

    # 2. 截屏获取预览区域
    screenshot = take_screenshot()

    # 3. 识别代码块区域（通过背景色识别）
    codeblock_rects = find_codeblocks(screenshot)

    results = {}
    for i, rect in enumerate(codeblock_rects):
        # 4. 测量代码块内部高度
        lines = count_text_lines(screenshot, rect)
        expected_height = lines * line_height()  # 每行高度
        actual_height = rect.height - padding_top - padding_bottom

        variance = abs(actual_height - expected_height) / expected_height
        results[f"codeblock_{i}"] = {
            "lines": lines,
            "expected_height": expected_height,
            "actual_height": actual_height,
            "variance_percent": variance * 100,
            "pass": variance < 0.05  # 允许 5% 偏差
        }

    app.kill()
    return {
        "test": "codeblock_spacing",
        "dpi": get_current_dpi(),
        "results": results,
        "overall": all(r["pass"] for r in results.values())
    }

def find_codeblocks(image):
    """通过背景色识别代码块位置"""
    # 代码块背景色：#f5f5f5 或 #2d2d2d（深色主题）
    # 返回 [(x, y, width, height), ...]
    pass

def count_text_lines(image, rect):
    """数代码块中的文本行数"""
    pass
```

---

## V2: 双击文本选中坐标偏差

### 问题描述
在代码块中双击"PlayerThumbnail"，点击"Play"部分，选中的是前面的"&"而不是整个单词。只在 B 屏（1.5x DPI）出现。

### 验证步骤

#### 自动化验证（需要 GUI 自动化）

```bash
python tests/verify_text_selection.py --dpi=1.5
```

脚本将：
1. 打开含代码块的测试文件
2. 模拟双击代码块中的指定位置
3. 记录选中的文本范围
4. 对比预期文本 vs 实际选中文本
5. 输出结果

#### 手动验证（B 屏）

1. 打开 `tests/fixtures/text_selection_test.md`
   ```cpp
   // 代码块示例
   void initPlayer() {
       if (m_player)
           return;
       const auto extendMap = map.value("extend").toMap();
   }
   ```

2. **测试用例 1**：双击 `m_player`
   - 点击位置：字母"p"
   - 预期选中：`m_player`
   - 实际选中：`m_player`
   - 结果：✓ PASS

3. **测试用例 2**：双击 `PlayerThumbnail`
   - 代码：`&PlayerThumbnail`
   - 点击位置：字母"P"
   - 预期选中：`PlayerThumbnail`
   - 不应该选中：`&`
   - 结果：✓ PASS

4. **测试用例 3**：双击 `extendMap`
   - 点击位置：中间的"d"
   - 预期选中：`extendMap`
   - 结果：✓ PASS

### 期望结果

在 A、B 屏上所有测试用例都应该 PASS。

---

## V3: 反引号宽度过大

### 问题描述
在代码块中，反引号周围有过多空白（约占文本宽度的 30%+）。

### 验证步骤

#### 自动化验证

```bash
python tests/verify_backtick_width.py
```

脚本将：
1. 截屏获取含反引号的行
2. 识别反引号及其周围的空白
3. 计算 backtick_width / text_width 比例
4. 验证比例在合理范围内（15-20%）

#### 手动验证

1. 打开 `tests/fixtures/backtick_test.md`
   ```
   调用 `getValue()` 函数返回值
   使用 `setProperty()` 设置属性
   ```

2. 观察反引号周围的空白
   - 应该与正文文本保持一致间距
   - 不应该有明显的多余空白
   - 在 A、B 屏表现一致

### 期望结果

反引号宽度占比应该 ≤ 20%（正常的字符间距）

---

## V4: 打开文件窗口未提升

### 问题描述
从浏览器或文件管理器双击 MD 文件时，应用加载了文件但窗口仍在后台。

### 验证步骤

#### 自动化验证（需要窗口管理 API）

```bash
python tests/verify_window_focus.py
```

脚本将：
1. 获取当前前台窗口
2. 打开一个新 MD 文件（通过 LocalSocket 模拟）
3. 等待 500ms
4. 检查 SimpleMarkdown 是否成为前台窗口
5. 返回结果

#### 手动验证

1. 启动应用并打开一个 MD 文件
2. 切换到其他应用（例如 VS Code、浏览器）
3. 在文件管理器中双击另一个 MD 文件
4. 观察：
   - ✓ PASS：SimpleMarkdown 窗口立即出现在最前
   - ✗ FAIL：SimpleMarkdown 窗口仍在后台，需要手动切换

### 期望结果

每次打开文件时，应用窗口应该立即被置于最前（无需用户手动操作）。

---

## V5: 高 DPI 标题/列表行间距

### 问题描述
在 B 屏（1.5x DPI）上，列表项或标题之间的间距看起来不均匀，比正常情况大。

### 验证步骤

#### 自动化验证

```bash
python tests/verify_list_spacing.py --dpi=1.5
```

脚本将：
1. 打开测试 Markdown（含标题、列表、段落混合）
2. 截屏并识别各元素的 y 坐标
3. 计算相邻元素之间的间距
4. 对比间距是否均匀（方差 < 5%）
5. 与 1x DPI 的结果对比

#### 手动验证（B 屏）

1. 打开 `tests/fixtures/spacing_test.md`
   ```markdown
   # 标题 1

   这是段落 1

   ## 标题 2

   - 列表项 1
   - 列表项 2
   - 列表项 3

   ## 标题 3

   这是段落 2
   ```

2. 观察各元素间的间距
   - 标题上下间距：应该一致
   - 列表项之间间距：应该一致
   - 段落之间间距：应该一致

3. 对比 A 屏的表现
   - B 屏间距应该 = A 屏间距（因为只是 DPI 缩放）

### 期望结果

所有间距应该均匀，在 A、B 屏上的相对关系保持不变。

---

## V6: 列表序号基线对齐

### 问题描述
列表序号（1、2、3）与后面的文本垂直对齐不对，序号偏下。这个问题在 A、B 屏都存在。

### 验证步骤

#### 自动化验证

```bash
python tests/verify_list_alignment.py
```

脚本将：
1. 截屏获取列表部分
2. 识别序号和文本的位置
3. 提取序号的基线 y 坐标
4. 提取文本的基线 y 坐标
5. 计算偏差：|序号基线 - 文本基线| < 2px
6. 返回结果

#### 手动验证

1. 打开 `tests/fixtures/list_alignment_test.md`
   ```markdown
   1. 第一行文本，观察序号 "1" 与文本是否对齐
   2. 第二行文本，序号应该与文本的顶部一致
   3. 第三行文本
   4. 第四行文本
   5. 第五行文本
   ```

2. 仔细观察：
   - 序号"1"、"2"、"3"等
   - 与对应行的文本首字母
   - 应该在同一水平线上

3. 对比 A、B 屏
   - 目前两个屏都有对齐问题（偏下）
   - 修复后应该都一致

### 期望结果

序号的基线应该与文本的基线对齐（偏差 ≤ 2px）。

---

## V7: 标记功能完整性

### 问题描述
验证内容标记功能的完整实现：选中、标记、TOC 联动、清除。

### 验证步骤

#### 自动化验证（需要 GUI 自动化）

```bash
python tests/verify_marking_feature.py
```

脚本将：
1. 打开含多个标题的测试文件
2. 选中某段文本 → 右键 → 点击"标记"
3. 验证：
   - 文本显示黄色背景
   - 对应标题在 TOC 中显示颜色标记
4. 再次标记不同部分
   - 验证多个标记可以并存
5. 右键 → "清除所有标记"
   - 验证所有标记消失
   - TOC 中的颜色标记也消失
6. 切换深色主题
   - 验证标记颜色改变（橙黄色）
   - 颜色在深色背景上清晰可见

#### 手动验证

1. 打开 `tests/fixtures/marking_feature_test.md`
   ```markdown
   # 第一章
   这是第一章的内容，包含一些要标记的重要信息。

   ## 第一节
   选中"要标记的重要信息"这几个字，右键标记。

   # 第二章
   这是第二章的内容。
   选中"这是第二章"，右键标记。
   ```

2. **测试 1**：基础标记
   - 选中文本 → 右键 → "标记"
   - ✓ 文本显示黄色背景
   - ✓ 对应标题在 TOC 中显示黄色标记

3. **测试 2**：多个标记
   - 标记多处文本
   - ✓ 所有标记都显示
   - ✓ 对应的 TOC 条目都标记

4. **测试 3**：清除标记
   - 右键 → "清除所有标记"
   - ✓ 所有黄色背景消失
   - ✓ TOC 中的颜色标记消失

5. **测试 4**：主题切换
   - 点击菜单 → 主题 → 深色
   - ✓ 标记颜色变为橙黄色
   - ✓ 在深色背景上清晰可见

### 期望结果

所有标记功能应该正常工作，TOC 联动准确，颜色方案适配主题。

---

## V8: DPI 切换时刷新

### 问题描述
从 1x DPI 屏移动到 1.5x DPI 屏时，应用应该重新计算布局，而不是延用旧的高度。

### 验证步骤

#### 自动化验证（需要模拟 DPI 改变）

```bash
python tests/verify_dpi_switch.py
```

脚本将：
1. 启动应用并打开测试文件
2. 记录初始布局高度
3. 模拟 DPI 改变（通过 Qt 事件）
4. 记录新的布局高度
5. 验证：新高度 ≈ 初始高度 × DPI_ratio
6. 验证：代码块没有多余空白

#### 手动验证

1. 在 A 屏（1x DPI）启动应用
2. 打开 `tests/fixtures/dpi_switch_test.md`
3. 观察代码块、列表的显示效果
   - A 屏上应该正常，无多余空白
4. 将窗口拖到 B 屏（1.5x DPI）
5. 等待 1 秒钟（等待 resizeEvent 和 updateMetrics）
6. 观察：
   - ✓ 代码块仍然正常，没有突然多出空白
   - ✓ 列表间距仍然均匀
   - ✓ 文本清晰，没有模糊或错位
7. 再拖回 A 屏
8. 观察：
   - ✓ 恢复原来的大小
   - ✓ 没有残留的 DPI 相关问题

### 期望结果

DPI 切换时布局应该平滑地重新计算，不出现闪烁、错位或多余空白。

---

## 运行验证流程

### 完整验证（所有测试）

```bash
cd D:/iflytek_projects/simple_markdown

# 方式 1：GUI 模式（推荐，有可视化反馈）
python tests/verify_all.py --gui

# 方式 2：命令行模式（纯自动化）
python tests/verify_all.py --headless

# 输出示例
# ============================================================
# SimpleMarkdown Verification Report
# ============================================================
# Test: V1 (Codeblock Spacing)         [PASS] ✓
# Test: V2 (Text Selection)             [PASS] ✓
# Test: V3 (Backtick Width)             [PASS] ✓
# Test: V4 (Window Focus)               [PASS] ✓
# Test: V5 (List Spacing)               [PASS] ✓
# Test: V6 (List Alignment)             [PASS] ✓
# Test: V7 (Marking Feature)            [PASS] ✓
# Test: V8 (DPI Switch)                 [PASS] ✓
#
# Overall: 8/8 PASS (100%)
# ============================================================
# Report saved to: tests/verification_report.json
```

### 分类验证

```bash
# 仅验证高 DPI 相关（V1, V2, V3, V5, V8）
python tests/verify_all.py --category=dpi

# 仅验证窗口行为（V4, V8）
python tests/verify_all.py --category=window

# 仅验证字体/排版（V2, V3, V5, V6）
python tests/verify_all.py --category=typography
```

### 持续验证

```bash
# 每次编译后自动运行验证
python tests/verify_on_build.py

# 监视源代码改动，自动验证
python tests/watch_and_verify.py
```

---

## 验证报告格式

每次验证生成 JSON 报告 `tests/verification_report.json`：

```json
{
  "timestamp": "2026-03-30T12:34:56Z",
  "build_version": "0.1.0",
  "tests": {
    "V1": {
      "name": "Codeblock Spacing",
      "status": "PASS",
      "dpi": 1.5,
      "metrics": {
        "codeblock_0": {
          "expected_height": 210,
          "actual_height": 210,
          "variance_percent": 0.0
        }
      }
    },
    "V2": {
      "name": "Text Selection",
      "status": "PASS",
      "tests": {
        "select_m_player": "PASS",
        "select_PlayerThumbnail": "PASS",
        "select_extendMap": "PASS"
      }
    }
  },
  "summary": {
    "total": 8,
    "passed": 8,
    "failed": 0,
    "skipped": 0,
    "success_rate": "100%"
  }
}
```

---

## 故障排除

### 自动化测试失败时

1. **查看详细日志**
   ```bash
   python tests/verify_all.py --verbose --log=verification.log
   ```

2. **重新运行单个测试**
   ```bash
   python tests/verify_dpi_scaling.py --test=codeblock_spacing --debug
   ```

3. **查看截屏对比**
   ```
   tests/screenshots/
     ├── expected/
     │   ├── V1_codeblock_1x.png
     │   ├── V1_codeblock_1.5x.png
     │   └── ...
     └── actual/
         ├── V1_codeblock_1.5x_actual.png
         └── ...
   ```

### 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|--------|
| 测试超时 | 应用启动慢 | 增加等待时间：`--timeout=5000` |
| 截屏失败 | 显示器被锁定 | 确保屏幕活跃，运行 `-s`（截屏模式） |
| 坐标偏差大 | DPI 与预期不符 | 检查 `get_current_dpi()` 输出 |
| GUI 自动化失败 | 依赖库缺失 | 安装：`pip install pyautogui pillow` |

---

## 最佳实践

### 添加新的验证用例

```python
# tests/verify_my_feature.py

import subprocess
from tests.common import VerificationTest, TestResult

class MyFeatureTest(VerificationTest):
    """验证我的新功能"""

    def setup(self):
        """准备环境"""
        self.app = self.launch_app()
        self.app.open_file('tests/fixtures/my_feature_test.md')

    def test_main(self) -> TestResult:
        """主要验证逻辑"""
        # 1. 执行操作
        # 2. 记录结果
        # 3. 返回 TestResult
        pass

    def teardown(self):
        """清理环境"""
        self.app.quit()

if __name__ == '__main__':
    test = MyFeatureTest()
    result = test.run()
    print(result.to_json())
```

### 在 CI/CD 中集成

```yaml
# .github/workflows/verify.yml
name: Automated Verification
on: [push, pull_request]
jobs:
  verify:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: ./build_on_win.bat
      - name: Verify
        run: python tests/verify_all.py --headless
      - name: Upload Report
        if: always()
        uses: actions/upload-artifact@v2
        with:
          name: verification-report
          path: tests/verification_report.json
```

---

## 检查清单（手动验证时）

在没有自动化环境的情况下，使用以下清单进行手动验证：

### A 屏验证（1x DPI）

- [ ] 代码块下方没有多余空白
- [ ] 双击选中文本准确
- [ ] 反引号宽度正常
- [ ] 列表序号与文本对齐
- [ ] 标记功能正常
- [ ] 打开新文件窗口立即出现

### B 屏验证（1.5x DPI）

- [ ] 代码块下方没有多余空白（与 A 屏一致）
- [ ] 双击选中文本准确
- [ ] 反引号宽度正常
- [ ] 列表序号与文本对齐
- [ ] 列表/标题间距均匀
- [ ] 标记颜色清晰可见

### DPI 切换验证

- [ ] 从 A 屏拖到 B 屏：布局正确计算
- [ ] 从 B 屏拖回 A 屏：恢复正常
- [ ] 没有闪烁、错位或多余空白

