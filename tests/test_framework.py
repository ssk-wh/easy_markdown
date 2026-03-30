"""
SimpleMarkdown 自动化测试框架

提供测试基础类、日志管理、断言工具等
"""

import os
import sys
import time
import logging
import json
from pathlib import Path
from dataclasses import dataclass, asdict
from typing import Dict, List, Optional, Callable, Any
from enum import Enum
from datetime import datetime

# 项目路径
PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT))

from tests.common import ApplicationProxy, ScreenCapture, TestStatus, TestResult


class TestFrameworkLogger:
    """测试框架日志记录器"""

    def __init__(self, log_dir: str = "tests/logs"):
        self.log_dir = Path(log_dir)
        self.log_dir.mkdir(parents=True, exist_ok=True)

        self.test_logs: Dict[str, List[str]] = {}
        self.current_test = None

    def start_test(self, test_name: str):
        """开始一个测试的日志记录"""
        self.current_test = test_name
        self.test_logs[test_name] = []
        self.log(f"[START] {test_name} at {datetime.now().isoformat()}")

    def log(self, message: str, level: str = "INFO"):
        """记录日志消息"""
        if self.current_test:
            timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            msg = f"[{timestamp}] [{level}] {message}"
            self.test_logs[self.current_test].append(msg)

    def info(self, message: str):
        """记录信息级别日志"""
        self.log(message, "INFO")

    def warning(self, message: str):
        """记录警告级别日志"""
        self.log(message, "WARN")

    def error(self, message: str):
        """记录错误级别日志"""
        self.log(message, "ERROR")

    def debug(self, message: str):
        """记录调试级别日志"""
        self.log(message, "DEBUG")

    def end_test(self, status: TestStatus, message: str = ""):
        """结束测试日志记录"""
        if self.current_test:
            self.log(f"[END] {self.current_test} - {status.value}", "INFO")
            if message:
                self.log(f"Message: {message}", "INFO")

            # 保存日志到文件
            self._save_test_log(self.current_test)
            self.current_test = None

    def _save_test_log(self, test_name: str):
        """保存单个测试的日志到文件"""
        if test_name in self.test_logs:
            log_file = self.log_dir / f"{test_name}.log"
            with open(log_file, 'w', encoding='utf-8') as f:
                f.write('\n'.join(self.test_logs[test_name]))

    def get_test_log(self, test_name: str) -> str:
        """获取测试日志内容"""
        if test_name in self.test_logs:
            return '\n'.join(self.test_logs[test_name])

        log_file = self.log_dir / f"{test_name}.log"
        if log_file.exists():
            return log_file.read_text(encoding='utf-8')

        return ""


class TestAssertions:
    """测试断言工具"""

    def __init__(self, logger: TestFrameworkLogger):
        self.logger = logger
        self.assertions_made = 0
        self.assertions_failed = 0

    def assert_equal(self, actual: Any, expected: Any, message: str = ""):
        """断言相等"""
        self.assertions_made += 1
        if actual != expected:
            self.assertions_failed += 1
            msg = f"Assertion failed: {actual} != {expected}"
            if message:
                msg = f"{msg} ({message})"
            self.logger.error(msg)
            raise AssertionError(msg)
        self.logger.debug(f"✓ Assert {actual} == {expected}")

    def assert_true(self, condition: bool, message: str = ""):
        """断言为真"""
        self.assertions_made += 1
        if not condition:
            self.assertions_failed += 1
            msg = f"Assertion failed: condition is False"
            if message:
                msg = f"{msg} ({message})"
            self.logger.error(msg)
            raise AssertionError(msg)
        self.logger.debug(f"✓ Assert condition is True")

    def assert_false(self, condition: bool, message: str = ""):
        """断言为假"""
        self.assertions_made += 1
        if condition:
            self.assertions_failed += 1
            msg = f"Assertion failed: condition is True"
            if message:
                msg = f"{msg} ({message})"
            self.logger.error(msg)
            raise AssertionError(msg)
        self.logger.debug(f"✓ Assert condition is False")

    def assert_in_range(self, value: float, min_val: float, max_val: float, message: str = ""):
        """断言在范围内"""
        self.assertions_made += 1
        if not (min_val <= value <= max_val):
            self.assertions_failed += 1
            msg = f"Assertion failed: {value} not in range [{min_val}, {max_val}]"
            if message:
                msg = f"{msg} ({message})"
            self.logger.error(msg)
            raise AssertionError(msg)
        self.logger.debug(f"✓ Assert {value} in range [{min_val}, {max_val}]")

    def assert_close(self, actual: float, expected: float, tolerance: float = 1.0, message: str = ""):
        """断言近似相等（在容差范围内）"""
        self.assertions_made += 1
        if abs(actual - expected) > tolerance:
            self.assertions_failed += 1
            msg = f"Assertion failed: {actual} not close to {expected} (tolerance: {tolerance})"
            if message:
                msg = f"{msg} ({message})"
            self.logger.error(msg)
            raise AssertionError(msg)
        self.logger.debug(f"✓ Assert {actual} ≈ {expected} (tolerance: {tolerance})")


class TestCase:
    """自动化测试基类"""

    def __init__(self, test_id: str, test_name: str,
                 logger: Optional[TestFrameworkLogger] = None,
                 exe_path: Optional[str] = None,
                 timeout: int = 3000,
                 test_method_name: Optional[str] = None):
        self.test_id = test_id
        self.test_name = test_name
        self.timeout = timeout
        self.test_method_name = test_method_name  # 允许指定要运行的测试方法

        # 日志和断言
        self.logger = logger or TestFrameworkLogger()
        self.assertions = TestAssertions(self.logger)

        # 应用实例
        self.exe_path = exe_path or str(PROJECT_ROOT / "build" / "app" / "SimpleMarkdown.exe")
        self.app: Optional[ApplicationProxy] = None

        # 测试结果
        self.result: Optional[TestResult] = None
        self._start_time = 0
        self._end_time = 0

    def setUp(self):
        """测试前准备（子类覆盖）"""
        pass

    def tearDown(self):
        """测试后清理（子类覆盖）"""
        if self.app and self.app.is_running():
            self.app.quit()

    def run_test(self) -> TestResult:
        """运行测试"""
        self.logger.start_test(self.test_id)
        self._start_time = time.time()

        try:
            # 调用 setUp
            self.setUp()
            self.logger.info("setUp completed")

            # 获取要运行的测试方法
            test_method = None

            # 如果指定了测试方法名，使用指定的
            if self.test_method_name:
                if hasattr(self, self.test_method_name):
                    attr = getattr(self, self.test_method_name)
                    if callable(attr):
                        test_method = attr
                    else:
                        raise NotImplementedError(f"test_method_name '{self.test_method_name}' is not callable")
                else:
                    raise NotImplementedError(f"test_method_name '{self.test_method_name}' not found")
            else:
                # 否则自动发现第一个以 test_ 开头的方法（除了 run_test 和 test_impl）
                # 按定义顺序而非字母顺序来避免歧义
                for attr_name in sorted(dir(self)):
                    if attr_name.startswith('test_') and attr_name not in ('run_test', 'test_impl'):
                        attr = getattr(self, attr_name)
                        if callable(attr):
                            test_method = attr
                            break

            if not test_method:
                raise NotImplementedError(f"No test method found for {self.test_id}")

            test_method()
            self.logger.info("test method completed successfully")

            # 创建结果
            duration = time.time() - self._start_time
            self.result = TestResult(
                test_id=self.test_id,
                name=self.test_name,
                status=TestStatus.PASS,
                message="Test passed",
                metrics={
                    "duration_ms": int(duration * 1000),
                    "assertions_made": self.assertions.assertions_made,
                    "assertions_failed": self.assertions.assertions_failed
                }
            )

        except AssertionError as e:
            duration = time.time() - self._start_time
            self.result = TestResult(
                test_id=self.test_id,
                name=self.test_name,
                status=TestStatus.FAIL,
                message=str(e),
                metrics={
                    "duration_ms": int(duration * 1000),
                    "assertions_made": self.assertions.assertions_made,
                    "assertions_failed": self.assertions.assertions_failed
                }
            )

        except Exception as e:
            duration = time.time() - self._start_time
            self.result = TestResult(
                test_id=self.test_id,
                name=self.test_name,
                status=TestStatus.ERROR,
                message=f"Unexpected error: {str(e)}",
                metrics={
                    "duration_ms": int(duration * 1000),
                    "assertions_made": self.assertions.assertions_made,
                    "assertions_failed": self.assertions.assertions_failed
                }
            )
            self.logger.error(f"Exception: {str(e)}")

        finally:
            # 调用 tearDown
            try:
                self.tearDown()
                self.logger.info("tearDown completed")
            except Exception as e:
                self.logger.warning(f"tearDown error: {str(e)}")

            # 记录结果
            if self.result:
                self.logger.end_test(self.result.status, self.result.message)

        return self.result

    def test_impl(self):
        """测试实现（必须由子类覆盖）"""
        raise NotImplementedError("test_impl must be implemented by subclass")

    def launch_app(self) -> ApplicationProxy:
        """启动应用"""
        self.logger.info(f"Launching app: {self.exe_path}")
        self.app = ApplicationProxy(exe_path=self.exe_path, timeout=self.timeout)
        self.app.launch()
        self.logger.info("App launched successfully")
        return self.app

    def wait_for_condition(self, condition_func: Callable[[], bool],
                          timeout: float = 5.0, check_interval: float = 0.1):
        """等待条件满足"""
        start = time.time()
        while time.time() - start < timeout:
            if condition_func():
                return True
            time.sleep(check_interval)

        self.logger.error(f"Condition not met within {timeout} seconds")
        return False

    def take_screenshot(self, name: str = "") -> Optional[Any]:
        """截图"""
        screenshot_dir = Path("tests/screenshots")
        screenshot_dir.mkdir(parents=True, exist_ok=True)

        filename = f"{self.test_id}_{name}_{datetime.now().strftime('%H%M%S')}.png"
        path = screenshot_dir / filename

        self.logger.info(f"Taking screenshot: {filename}")
        screenshot = ScreenCapture.take_screenshot(str(path))

        if screenshot:
            self.logger.debug(f"Screenshot saved: {path}")
        else:
            self.logger.error("Failed to take screenshot")

        return screenshot

    def get_render_blocks(self) -> Optional[Dict]:
        """获取应用输出的渲染块坐标信息（测试模式）"""
        import json
        import tempfile

        # 在 Windows 临时目录下查找 render_blocks.json
        render_blocks_path = Path(tempfile.gettempdir()) / "render_blocks.json"

        if render_blocks_path.exists():
            try:
                with open(render_blocks_path, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    self.logger.info(f"Loaded render blocks: {len(data.get('blocks', []))} blocks found")
                    return data
            except Exception as e:
                self.logger.error(f"Failed to read render_blocks.json: {str(e)}")
                return None
        else:
            self.logger.warning(f"render_blocks.json not found at {render_blocks_path}")
            return None


class MarkdownFixture:
    """Markdown 测试文件加载器"""

    def __init__(self, fixtures_dir: str = None):
        if fixtures_dir is None:
            # 使用绝对路径确保工作目录无关
            fixtures_dir = str(PROJECT_ROOT / "tests" / "markdown_fixtures")
        self.fixtures_dir = Path(fixtures_dir)
        self.fixtures_dir.mkdir(parents=True, exist_ok=True)

    def load(self, filename: str) -> str:
        """加载 Markdown 文件内容"""
        filepath = self.fixtures_dir / filename
        if not filepath.exists():
            raise FileNotFoundError(f"Fixture not found: {filepath}")

        return filepath.read_text(encoding='utf-8')

    def create(self, filename: str, content: str):
        """创建 Markdown 测试文件"""
        filepath = self.fixtures_dir / filename
        filepath.write_text(content, encoding='utf-8')

    def list_all(self) -> List[str]:
        """列出所有测试文件"""
        return [f.name for f in self.fixtures_dir.glob("*.md")]


class TestSuite:
    """测试套件 - 管理多个测试"""

    def __init__(self, logger: Optional[TestFrameworkLogger] = None):
        self.logger = logger or TestFrameworkLogger()
        self.tests: List[TestCase] = []
        self.results: List[TestResult] = []

    def add_test(self, test: TestCase):
        """添加测试"""
        self.tests.append(test)

    def add_tests(self, tests: List[TestCase]):
        """添加多个测试"""
        self.tests.extend(tests)

    def run_all(self, stop_on_error: bool = False) -> Dict:
        """运行所有测试"""
        print(f"Running {len(self.tests)} tests...")

        passed = 0
        failed = 0
        errors = 0

        for i, test in enumerate(self.tests, 1):
            print(f"[{i}/{len(self.tests)}] {test.test_id}...", end=" ", flush=True)

            result = test.run_test()
            self.results.append(result)

            if result.status == TestStatus.PASS:
                print("[PASS]")
                passed += 1
            elif result.status == TestStatus.FAIL:
                print("[FAIL]")
                failed += 1
                if stop_on_error:
                    break
            else:
                print("[ERROR]")
                errors += 1
                if stop_on_error:
                    break

        return {
            "total": len(self.results),
            "passed": passed,
            "failed": failed,
            "errors": errors,
            "results": [r.to_dict() for r in self.results]
        }

    def get_summary(self) -> Dict:
        """获取测试摘要"""
        passed = sum(1 for r in self.results if r.status == TestStatus.PASS)
        failed = sum(1 for r in self.results if r.status == TestStatus.FAIL)
        errors = sum(1 for r in self.results if r.status == TestStatus.ERROR)

        return {
            "total": len(self.results),
            "passed": passed,
            "failed": failed,
            "errors": errors,
            "pass_rate": f"{passed / len(self.results) * 100:.1f}%" if self.results else "N/A"
        }
