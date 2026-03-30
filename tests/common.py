"""
SimpleMarkdown 验证框架 - 通用工具库

提供所有验证脚本共用的基础类和工具函数。
"""

import subprocess
import time
import json
import os
import sys
from pathlib import Path
from enum import Enum
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict

# 添加项目根目录到 Python 路径
PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT))


class TestStatus(Enum):
    """测试状态枚举"""
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"
    ERROR = "ERROR"


@dataclass
class TestResult:
    """单个测试结果"""
    test_id: str
    name: str
    status: TestStatus
    message: str = ""
    metrics: Dict = None
    timestamp: str = ""
    dpi: Optional[float] = None

    def __post_init__(self):
        if self.metrics is None:
            self.metrics = {}
        if not self.timestamp:
            self.timestamp = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())

    def to_dict(self):
        """转换为字典"""
        return {
            "test_id": self.test_id,
            "name": self.name,
            "status": self.status.value,
            "message": self.message,
            "metrics": self.metrics,
            "timestamp": self.timestamp,
            "dpi": self.dpi
        }

    def to_json(self):
        """转换为 JSON 字符串"""
        return json.dumps(self.to_dict(), indent=2, ensure_ascii=False)

    def __str__(self):
        icon = "✓" if self.status == TestStatus.PASS else "✗"
        return f"{icon} {self.test_id} ({self.name}): {self.status.value}"


class VerificationTest:
    """验证测试基类"""

    def __init__(self, test_id: str, name: str):
        self.test_id = test_id
        self.name = name
        self.app = None
        self.result = None

    def setup(self):
        """准备环境（子类覆盖）"""
        pass

    def test_main(self) -> TestResult:
        """主要验证逻辑（必须由子类实现）"""
        raise NotImplementedError

    def teardown(self):
        """清理环境（子类覆盖）"""
        if self.app:
            self.app.quit()

    def run(self) -> TestResult:
        """运行测试"""
        try:
            self.setup()
            self.result = self.test_main()
            return self.result
        except Exception as e:
            return TestResult(
                test_id=self.test_id,
                name=self.name,
                status=TestStatus.ERROR,
                message=f"Error: {str(e)}"
            )
        finally:
            self.teardown()

    def assert_equal(self, actual, expected, tolerance=0):
        """断言相等"""
        if tolerance == 0:
            return actual == expected
        else:
            return abs(actual - expected) <= tolerance

    def assert_in_range(self, value, min_val, max_val):
        """断言在范围内"""
        return min_val <= value <= max_val


class ApplicationProxy:
    """应用程序代理 - 用于控制和监控 SimpleMarkdown"""

    def __init__(self, exe_path: str = None, timeout: int = 3000):
        if exe_path is None:
            exe_path = str(PROJECT_ROOT / "build" / "app" / "SimpleMarkdown.exe")

        self.exe_path = exe_path
        self.process = None
        self.timeout = timeout
        self.temp_files = []  # 追踪临时文件以便清理

    def launch(self, file_path: str = None):
        """启动应用，可选打开指定文件"""
        if not Path(self.exe_path).exists():
            raise FileNotFoundError(f"应用未找到: {self.exe_path}")

        cmd = [self.exe_path]
        if file_path:
            cmd.append(str(Path(file_path).absolute()))

        # [高 DPI 修复] 设置环境以包含Qt库路径，否则高 DPI 屏上可能无法启动
        env = os.environ.copy()
        qt_path = "D:\\Qt\\Qt5.12.9\\5.12.9\\msvc2017_64\\bin"
        if qt_path not in env.get('PATH', ''):
            env['PATH'] = qt_path + ";" + env.get('PATH', '')

        self.process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            env=env
        )
        time.sleep(2)  # 等待应用启动

        # [诊断] 检查应用是否已经退出，如果退出则输出错误信息
        if not self.is_running():
            try:
                stdout, stderr = self.process.communicate(timeout=1)
                if stdout:
                    print(f"[应用输出] {stdout[:500]}")
                if stderr:
                    print(f"[应用错误] {stderr[:500]}")
                print(f"[应用退出码] {self.process.returncode}")
            except subprocess.TimeoutExpired:
                pass

        return self

    def open_file(self, file_path: str):
        """打开文件（通过文件路径）"""
        file_path = str(Path(file_path).absolute())

        if self.is_running():
            # 应用已运行，尝试通过 LocalSocket 发送文件路径
            try:
                import socket
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect(("127.0.0.1", 9999))  # 简化版，实际应用可能使用命名管道
                sock.send(file_path.encode('utf-8'))
                sock.close()
            except:
                # 如果 IPC 失败，至少记录错误但不中断
                print(f"警告: 无法通过 IPC 打开文件 {file_path}")
        else:
            # 应用未运行，通过命令行启动
            self.launch(file_path)

        time.sleep(1)  # 等待文件打开
        return self

    def open_file_from_text(self, markdown_content: str, filename: str = "temp.md"):
        """从文本内容打开文件（自动创建临时文件）"""
        # 创建临时文件
        temp_dir = Path(self.exe_path).parent / "temp_test"
        temp_dir.mkdir(exist_ok=True)

        temp_file = temp_dir / filename
        temp_file.write_text(markdown_content, encoding='utf-8')
        self.temp_files.append(temp_file)

        # 打开文件
        self.open_file(str(temp_file))
        return self

    def quit(self):
        """关闭应用并清理临时文件"""
        # 清理临时文件
        for temp_file in self.temp_files:
            try:
                if temp_file.exists():
                    temp_file.unlink()
            except:
                pass

        # 关闭应用
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.process.kill()

    def is_running(self) -> bool:
        """检查应用是否运行"""
        return self.process and self.process.poll() is None


class ScreenCapture:
    """屏幕截图工具"""

    @staticmethod
    def take_screenshot(output_path: str = None) -> 'Image':
        """
        截取屏幕

        Args:
            output_path: 可选，保存截图到文件

        Returns:
            PIL Image 对象
        """
        try:
            from PIL import ImageGrab
            screenshot = ImageGrab.grab()

            if output_path:
                output_path = Path(output_path)
                output_path.parent.mkdir(parents=True, exist_ok=True)
                screenshot.save(output_path)

            return screenshot
        except ImportError:
            print("需要安装 Pillow: pip install pillow")
            return None

    @staticmethod
    def get_pixel_color(image, x: int, y: int) -> Tuple[int, int, int]:
        """获取像素颜色 (R, G, B)"""
        return image.getpixel((x, y))[:3]

    @staticmethod
    def find_color_region(image, color: Tuple[int, int, int], tolerance: int = 10) -> List[Tuple]:
        """
        找到特定颜色的区域

        Returns:
            [(x, y, width, height), ...]
        """
        pixels = image.load()
        width, height = image.size
        regions = []

        # 简化版本：扫描找到颜色
        for y in range(height):
            for x in range(width):
                px = pixels[x, y][:3]
                if all(abs(px[i] - color[i]) <= tolerance for i in range(3)):
                    # 找到一个匹配的像素
                    pass

        return regions


class DPIHelper:
    """DPI 相关工具"""

    @staticmethod
    def get_current_dpi() -> float:
        """获取当前屏幕 DPI 缩放比例"""
        if sys.platform == 'win32':
            try:
                import ctypes
                # Windows DPI 缩放
                user32 = ctypes.windll.user32
                hdc = user32.GetDC(0)
                dpi = ctypes.windll.kernel32.GetDpiForWindow(hdc)

                # 标准 96 DPI
                if dpi == 0:
                    dpi = 96

                return dpi / 96.0
            except:
                return 1.0
        else:
            # Linux/macOS 简化处理
            return 1.0

    @staticmethod
    def simulate_dpi_change(dpi_ratio: float):
        """模拟 DPI 改变（需要 Qt 事件系统支持）"""
        # 发送 QGuiApplication::screenGeometryChanged 信号
        pass


class TestFixtures:
    """测试固件 - 生成测试用的 Markdown 文件"""

    FIXTURES_DIR = PROJECT_ROOT / "tests" / "fixtures"

    @classmethod
    def setup_fixtures(cls):
        """设置所有测试固件"""
        cls.FIXTURES_DIR.mkdir(parents=True, exist_ok=True)

        cls.create_dpi_codeblock_test()
        cls.create_text_selection_test()
        cls.create_backtick_test()
        cls.create_spacing_test()
        cls.create_list_alignment_test()
        cls.create_marking_feature_test()

    @classmethod
    def create_dpi_codeblock_test(cls):
        """创建 DPI 代码块测试文件"""
        content = """# DPI 代码块测试

这是高 DPI 下代码块的测试。代码块下方不应该有多余的空白。

## 代码块 1

```cpp
void initPlayer() {
    if (m_player)
        return;

    const auto extendMap = map.value("extend").toMap();
    m_player->setPlayerDocCtxId(ctxId);
    m_player->triggerAction("initPage");
}
```

## 代码块 2

```python
def calculate_metrics(font_size, dpi_ratio):
    base_height = font_size * 1.5
    actual_height = base_height * dpi_ratio
    return actual_height

result = calculate_metrics(14, 1.5)
```

## 代码块 3

```javascript
function findCodeblocks(image) {
    const blocks = [];
    for (let y = 0; y < image.height; y++) {
        for (let x = 0; x < image.width; x++) {
            // 识别代码块背景色
        }
    }
    return blocks;
}
```

这段文本在代码块后面，用来检查代码块下方是否有多余空白。
如果有额外的空行，说明高度计算有误。
"""
        filepath = cls.FIXTURES_DIR / "dpi_codeblock.md"
        filepath.write_text(content, encoding='utf-8')

    @classmethod
    def create_text_selection_test(cls):
        """创建文本选中测试文件"""
        content = """# 文本选中测试

## 测试用例 1

代码块中的变量选中测试：

```cpp
void example() {
    int m_player = 0;
    const auto extendMap = map.value("extend").toMap();
    QWidget* bgWidget = new QWidget(this);
    bgWidget->show();
}
```

双击 `m_player` - 应该选中整个单词，不包括前面的符号。

## 测试用例 2

带符号的标识符：

```cpp
void initPlayer() {
    if (m_player)
        return;

    const auto& PlayerThumbnail = getThumbnail();
    const auto extendMap = map.value("extend").toMap();
}
```

双击 `PlayerThumbnail` - 应该只选中 PlayerThumbnail，不包括前面的 `&`。

## 测试用例 3

长变量名：

```cpp
void processUserDataExtended() {
    QMap<QString, QVariant> userDataExtended;
    userDataExtended.insert("name", "test");

    ProcessorFactory processorFactory;
    processorFactory.initialize();
}
```

双击任何位置 - 应该准确选中对应的单词。
"""
        filepath = cls.FIXTURES_DIR / "text_selection_test.md"
        filepath.write_text(content, encoding='utf-8')

    @classmethod
    def create_backtick_test(cls):
        """创建反引号测试文件"""
        content = """# 反引号宽度测试

反引号（代码标记）周围的空白应该与正常文本间距一致。

## 内联代码示例

调用 `getValue()` 函数返回值。

使用 `setProperty()` 设置属性值。

在 `onInit()` 回调中初始化。

常见的 `m_player` 变量。

返回 `nullptr` 作为默认值。

## 对比测试

这是普通文本中间有 `getValue()` 代码。

函数名 `processUserData()` 在句子中。

变量 `userDataExtended` 和 `m_player` 应该等宽。

## 验证项

- 反引号宽度占比应该 <= 20%
- 在 A、B 屏表现一致
- 不应该看到明显的多余空白
"""
        filepath = cls.FIXTURES_DIR / "backtick_test.md"
        filepath.write_text(content, encoding='utf-8')

    @classmethod
    def create_spacing_test(cls):
        """创建间距测试文件"""
        content = """# 间距测试 - 验证行间距均匀性

这是第一段文本。在高 DPI 屏上，段落之间的间距应该均匀。

## 一级标题

这是标题下的段落。

标题和段落之间的距离应该保持一致。

### 二级标题

二级标题的间距也应该匀称。

### 另一个二级标题

这个二级标题前后的间距应该与上面的一致。

## 列表间距测试

- 列表项 1
- 列表项 2
- 列表项 3
- 列表项 4
- 列表项 5

列表项之间的间距应该均匀。

### 有序列表

1. 第一项
2. 第二项
3. 第三项
4. 第四项

## 混合内容

这段文本在列表后面。

### 另一个标题

确保所有的间距都是一致的。

在 A 屏和 B 屏上，相对间距应该保持不变。
"""
        filepath = cls.FIXTURES_DIR / "spacing_test.md"
        filepath.write_text(content, encoding='utf-8')

    @classmethod
    def create_list_alignment_test(cls):
        """创建列表对齐测试文件"""
        content = """# 列表对齐测试

## 序号与文本对齐

1. 第一行文本，观察序号 "1" 与文本是否对齐
2. 第二行文本，序号应该与文本的首字母在同一水平线上
3. 第三行文本，这很重要用来检查垂直对齐
4. 第四行文本，序号应该不能偏上或偏下
5. 第五行文本，确保所有序号的对齐方式一致

## 对比文本

正常段落中的文本应该与列表序号在同一基线上（或接近）。

## 无序列表

- 项目 1，检查虚线符号与文本对齐
- 项目 2，符号应该与文本首字母对齐
- 项目 3，不应该有垂直偏差
- 项目 4，在 A、B 屏都应该一致

## 嵌套列表

1. 顶级项目 1
   - 子项目 1-1
   - 子项目 1-2
2. 顶级项目 2
   - 子项目 2-1
3. 顶级项目 3

嵌套项目的对齐也应该保持一致。
"""
        filepath = cls.FIXTURES_DIR / "list_alignment_test.md"
        filepath.write_text(content, encoding='utf-8')

    @classmethod
    def create_marking_feature_test(cls):
        """创建标记功能测试文件"""
        content = """# 标记功能测试

## 第一章 - 基础概念

这是第一章的内容。选中"基础概念"这四个字，右键选择"标记"，应该显示黄色背景。

### 第一节 - 重要内容

这个部分包含要标记的重要信息。选中"重要信息"，标记后应该在 TOC 中看到"第一节"显示颜色标记。

## 第二章 - 进阶主题

这是第二章的内容，介绍一些进阶知识点。

### 第二节 - 实现细节

关键的实现细节应该被强调出来。选中"实现细节"并标记。

## 第三章 - 高级应用

复杂的应用场景需要特别关注。

### 第三节 - 性能优化

性能优化是关键部分。标记"性能优化"看看是否显示正确。

## 验证检查清单

标记后应该验证：

1. 被标记的文本显示黄色（浅色主题）或橙黄色（深色主题）
2. 对应的标题在 TOC 中显示颜色标记
3. 多个标记可以同时存在
4. 右键"清除所有标记"可以删除所有标记
5. 清除后 TOC 中的标记也消失
6. 切换主题后颜色正确改变

完成验证后，应该得到一个完整的标记功能工作报告。
"""
        filepath = cls.FIXTURES_DIR / "marking_feature_test.md"
        filepath.write_text(content, encoding='utf-8')


class VerificationReport:
    """验证报告生成器"""

    def __init__(self):
        self.tests: Dict[str, TestResult] = {}
        self.start_time = time.time()

    def add_result(self, result: TestResult):
        """添加测试结果"""
        self.tests[result.test_id] = result

    def get_summary(self) -> Dict:
        """获取摘要"""
        total = len(self.tests)
        passed = sum(1 for r in self.tests.values() if r.status == TestStatus.PASS)
        failed = sum(1 for r in self.tests.values() if r.status == TestStatus.FAIL)
        errors = sum(1 for r in self.tests.values() if r.status == TestStatus.ERROR)
        skipped = sum(1 for r in self.tests.values() if r.status == TestStatus.SKIP)

        return {
            "total": total,
            "passed": passed,
            "failed": failed,
            "errors": errors,
            "skipped": skipped,
            "success_rate": f"{(passed / total * 100):.1f}%" if total > 0 else "0%",
            "duration_seconds": time.time() - self.start_time
        }

    def to_json(self) -> str:
        """转换为 JSON"""
        report = {
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "tests": {k: v.to_dict() for k, v in self.tests.items()},
            "summary": self.get_summary()
        }
        return json.dumps(report, indent=2, ensure_ascii=False)

    def save(self, filepath: str):
        """保存报告到文件"""
        Path(filepath).parent.mkdir(parents=True, exist_ok=True)
        Path(filepath).write_text(self.to_json(), encoding='utf-8')
        print(f"✓ 报告已保存: {filepath}")

    def print_summary(self):
        """打印摘要"""
        summary = self.get_summary()
        print("\n" + "=" * 60)
        print("SimpleMarkdown Verification Report")
        print("=" * 60)

        for test_id, result in self.tests.items():
            icon = "✓" if result.status == TestStatus.PASS else "✗"
            print(f"{icon} {test_id:5} {result.name:30} [{result.status.value:5}]")

        print("=" * 60)
        print(f"Total: {summary['total']} | Passed: {summary['passed']} | "
              f"Failed: {summary['failed']} | Success Rate: {summary['success_rate']}")
        print("=" * 60 + "\n")


# 初始化测试固件
def init_testing_environment():
    """初始化测试环境"""
    TestFixtures.setup_fixtures()
    print("✓ 测试环境已初始化")


if __name__ == '__main__':
    init_testing_environment()
