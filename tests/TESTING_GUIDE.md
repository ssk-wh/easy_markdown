# SimpleMarkdown 自动化测试框架 - 使用指南

## 概述

SimpleMarkdown 自动化测试框架提供了一套完整的测试工具，用于验证应用在不同环境下（DPI、分辨率、系统缩放）的 Markdown 渲染正确性。

## 框架结构

```
tests/
├── automated_tests.py           # 主入口脚本
├── test_framework.py            # 测试基础类（TestCase、TestSuite）
├── test_scenarios.py            # 测试场景（40+ 测试用例）
├── dpi_simulator.py             # DPI 模拟工具
├── rendering_validator.py       # 渲染验证工具
├── report_generator.py          # 报告生成器
├── markdown_fixtures/           # Markdown 测试文件
│   ├── headings.md
│   ├── lists_ordered.md
│   ├── lists_unordered.md
│   ├── code_blocks.md
│   ├── inline_code.md
│   ├── tables.md
│   ├── quotes.md
│   ├── special_chars.md
│   ├── mixed_content.md
│   └── nested_structures.md
├── logs/                        # 测试日志输出目录
└── reports/                     # 测试报告输出目录
```

## 快速开始

### 1. 运行所有测试

```bash
cd tests
python automated_tests.py --dpi all --category all
```

### 2. 运行特定类别的测试

```bash
# 仅运行渲染测试
python automated_tests.py --dpi 1x --category rendering

# 仅运行 DPI 测试
python automated_tests.py --dpi all --category dpi

# 运行鼠标交互和边界情况测试
python automated_tests.py --category interaction,edge
```

### 3. 详细输出

```bash
python automated_tests.py --dpi 1.5x --category all --verbose
```

## CLI 选项

### `--dpi` - 设置 DPI 配置

- `1x` - 标准 DPI (96 DPI)
- `1.25x` - 中等缩放 (120 DPI)
- `1.5x` - 高缩放 (144 DPI)
- `all` - 所有配置（默认）

### `--category` - 选择测试类别

- `rendering` - 渲染格式测试（15+ 测试）
- `dpi` - DPI 适配测试（10+ 测试）
- `interaction` - 鼠标交互测试（6+ 测试）
- `edge` - 边界情况测试（5+ 测试）
- `all` - 所有类别（默认）

多个类别可用逗号分隔：`--category rendering,dpi`

### `--verbose` - 详细输出

启用详细日志输出，便于调试

### `--log-dir` - 指定日志目录

默认：`tests/logs`

### `--report-dir` - 指定报告目录

默认：`tests/reports`

## 测试类别说明

### 1. RenderingTests（15+ 测试）

验证各种 Markdown 格式的正确渲染：

- H1-H6 标题
- 有序/无序列表
- 列表嵌套和对齐
- 代码块高度一致性
- 行内代码 padding
- 表格对齐和边框
- 块引用缩进
- 文本格式（粗体、斜体、删除线）
- 链接和混合内容

### 2. DPITests（10+ 测试）

验证在不同 DPI 下的适配性：

- 1x、1.25x、1.5x DPI 初始化
- DPI 动态切换（A→B→A）
- 行高一致性验证
- 坐标精度（鼠标命中）
- 分辨率独立性
- 多次 DPI 切换

### 3. MouseInteractionTests（6+ 测试）

验证鼠标相关的功能：

- 单击选中
- 双击选中单词
- 拖拽选中范围
- 右键菜单
- 标记功能
- 清除标记

### 4. EdgeCaseTests（5+ 测试）

验证边界情况的处理：

- 超长代码行
- Unicode 和 emoji
- 空内容块
- 深层嵌套（6+ 层）
- 多标签页切换状态保存

## 测试输出

### 日志文件

每个测试生成一个日志文件，存放在 `tests/logs/` 目录下：

```
test_001.log
test_002.log
...
```

日志格式：
```
[14:23:45.123] [INFO] test_rendering_001 started
[14:23:45.234] [INFO] Launching app: ...
[14:23:46.456] [INFO] setUp completed
[14:23:47.789] [DEBUG] Loading markdown content
[14:23:48.012] [INFO] test_impl completed successfully
[14:23:48.100] [INFO] tearDown completed
[14:23:48.100] [END] test_rendering_001 - PASS
```

### 报告文件

生成的报告文件在 `tests/reports/` 目录下：

1. **results.json** - 完整的 JSON 测试结果
2. **summary.txt** - 文本摘要
3. **report.html** - 可视化 HTML 报告

### HTML 报告

使用浏览器打开 `tests/reports/report.html` 查看可视化报告：

- 测试统计（总数、通过、失败、错误）
- 成功率和进度条
- 测试详情表格（ID、名称、状态、耗时）
- 响应式设计

## 测试数据

### Markdown 测试文件

`tests/markdown_fixtures/` 目录包含 10 个测试文件，覆盖常见的 Markdown 格式：

| 文件 | 用途 |
|------|------|
| headings.md | H1-H6 标题渲染 |
| lists_ordered.md | 有序列表（嵌套） |
| lists_unordered.md | 无序列表（多符号和深嵌套） |
| code_blocks.md | 代码块（多语言和长行） |
| inline_code.md | 行内代码和混合格式 |
| tables.md | 表格（多大小和对齐） |
| quotes.md | 块引用（嵌套） |
| special_chars.md | Unicode、emoji 和符号 |
| mixed_content.md | 混合内容（列表+代码+表格） |
| nested_structures.md | 复杂嵌套结构 |

### 自定义测试

可以在 `markdown_fixtures/` 中添加新的 `.md` 文件，测试框架会自动加载：

```python
from tests.test_framework import MarkdownFixture

fixture = MarkdownFixture()
content = fixture.load("my_test.md")
```

## 测试框架 API

### TestCase 类

基础测试类，子类应继承并实现 `test_impl()` 方法：

```python
class MyTest(TestCase):
    def setUp(self):
        """测试前准备"""
        self.app = self.launch_app()

    def test_impl(self):
        """测试实现"""
        self.assertions.assert_true(True, "Test passed")

    def tearDown(self):
        """测试后清理"""
        if self.app:
            self.app.quit()
```

### 常用方法

```python
# 启动应用
self.app = self.launch_app()

# 等待条件
self.wait_for_condition(lambda: self.app.is_rendered(), timeout=5.0)

# 截图
screenshot = self.take_screenshot(name="my_screenshot")

# 断言
self.assertions.assert_equal(actual, expected, message="optional")
self.assertions.assert_true(condition, message="optional")
self.assertions.assert_in_range(value, min_val, max_val)
self.assertions.assert_close(actual, expected, tolerance=1.0)

# 日志
self.logger.info("Information message")
self.logger.warning("Warning message")
self.logger.error("Error message")
self.logger.debug("Debug message")
```

### DPISimulator 类

模拟 DPI 配置：

```python
from tests.dpi_simulator import DPISimulator

simulator = DPISimulator()
simulator.set_dpi("1.5x")
dpi = simulator.get_current_dpi()  # 1.5
simulator.switch_to_1x()
```

### RenderingValidator 类

验证渲染正确性：

```python
from tests.rendering_validator import RenderingValidator, PixelRegion

validator = RenderingValidator()

# 创建区域
regions = [
    PixelRegion(10, 20, 100, 50),
    PixelRegion(10, 80, 100, 50),
]

# 验证
validator.validate_no_overlap(regions)
validator.validate_alignment(regions, alignment_type="vertical_spacing")
validator.validate_minimum_spacing(regions, min_spacing=5.0)
```

## 常见任务

### 运行特定 DPI 的渲染测试

```bash
python automated_tests.py --dpi 1.5x --category rendering
```

### 运行所有 DPI 测试并生成详细日志

```bash
python automated_tests.py --dpi all --category dpi --verbose
```

### 运行鼠标交互测试

```bash
python automated_tests.py --category interaction
```

### 查看 HTML 报告

```bash
# 在浏览器中打开
start tests/reports/report.html

# 或在 Linux/macOS 上
open tests/reports/report.html
```

### 添加新的测试

1. 在 `test_scenarios.py` 中创建新的测试类或方法
2. 实现 `test_impl()` 方法
3. 添加必要的 Markdown 测试文件
4. 在 `create_all_tests()` 函数中注册新测试

## 成功标准

✅ **渲染正确性**: 100% 无重叠、无错位、无多余空白
✅ **DPI 一致性**: 布局一致（高度差 < 0.5px）
✅ **坐标精度**: 鼠标命中精度 > 95%
✅ **标记功能**: 100% 通过率
✅ **性能**: 每个测试 < 500ms
✅ **测试覆盖**: PASS 率 ≥ 95%

## 故障排查

### 测试无法启动应用

检查应用路径：
```python
DEFAULT_EXE = "build/app/SimpleMarkdown.exe"
# 确保编译后的可执行文件存在
```

### DPI 测试失败

确保在目标 DPI 的屏幕上运行，或使用虚拟机调整 DPI

### 渲染验证失败

1. 查看生成的截图
2. 检查 `tests/logs/` 中的详细日志
3. 确保应用已正确编译和部署

## 更新和扩展

### 添加新的验证规则

在 `RenderingValidator` 中添加新方法：

```python
def validate_custom_rule(self, data):
    """自定义验证规则"""
    # 实现验证逻辑
    pass
```

### 支持新的 DPI 配置

在 `DPISimulator.DPI_CONFIGS` 中添加：

```python
DPI_CONFIGS = {
    "1x": 1.0,
    "2x": 2.0,  # 新增
}
```

## 联系和支持

有问题或需要帮助？

1. 查看具体测试的日志文件
2. 检查 HTML 报告中的失败详情
3. 参考本文档的相应部分

---

**最后更新**: 2026-03-30
**框架版本**: 1.0
**测试用例总数**: 40+
