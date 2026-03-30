"""
DPI 模拟与屏幕切换工具

提供 DPI 配置、动态切换、验证等功能
"""

import sys
import time
from pathlib import Path
from typing import Optional, Tuple

# 项目路径
PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT))

from tests.common import DPIHelper


class DPISimulator:
    """DPI 模拟器 - 模拟不同的 DPI 配置"""

    # 常见的 DPI 配置
    DPI_CONFIGS = {
        "1x": 1.0,
        "1.25x": 1.25,
        "1.5x": 1.5,
        "2x": 2.0,
    }

    def __init__(self):
        self.current_dpi = self._get_actual_dpi()
        self.dpi_history = [self.current_dpi]

    @staticmethod
    def _get_actual_dpi() -> float:
        """获取实际 DPI"""
        return DPIHelper.get_current_dpi()

    def set_dpi(self, dpi_config: str) -> bool:
        """
        设置 DPI（模拟）

        在实际运行中，可以：
        1. 修改 Windows 显示设置（通过 Windows API）
        2. 或在虚拟机中调整屏幕 DPI
        3. 或使用多屏并在不同 DPI 的屏幕间移动窗口

        当前实现为简化版本，直接记录 DPI 配置
        """
        if dpi_config not in self.DPI_CONFIGS:
            raise ValueError(f"Unknown DPI config: {dpi_config}. Must be one of {list(self.DPI_CONFIGS.keys())}")

        target_dpi = self.DPI_CONFIGS[dpi_config]
        self.current_dpi = target_dpi
        self.dpi_history.append(target_dpi)

        return True

    def get_current_dpi(self) -> float:
        """获取当前 DPI"""
        return self.current_dpi

    def get_dpi_ratio(self) -> float:
        """获取 DPI 比率（相对于 96 DPI）"""
        return self.current_dpi

    def switch_to_1x(self) -> bool:
        """切换到 1x DPI"""
        return self.set_dpi("1x")

    def switch_to_1_25x(self) -> bool:
        """切换到 1.25x DPI"""
        return self.set_dpi("1.25x")

    def switch_to_1_5x(self) -> bool:
        """切换到 1.5x DPI"""
        return self.set_dpi("1.5x")

    def switch_to_2x(self) -> bool:
        """切换到 2x DPI"""
        return self.set_dpi("2x")

    def verify_dpi_consistency(self, expected_dpi: float, tolerance: float = 0.05) -> bool:
        """
        验证 DPI 一致性

        用于验证应用在指定 DPI 下正确运行
        """
        actual_dpi = self.get_current_dpi()
        diff = abs(actual_dpi - expected_dpi)
        is_consistent = diff <= tolerance

        return is_consistent

    def get_dpi_history(self) -> list:
        """获取 DPI 变化历史"""
        return self.dpi_history

    def reset_history(self):
        """重置历史记录"""
        self.dpi_history = [self.current_dpi]


class ScreenConfiguration:
    """屏幕配置 - 定义不同屏幕的参数"""

    def __init__(self, dpi: float, width: int, height: int, name: str = ""):
        self.dpi = dpi
        self.width = width
        self.height = height
        self.name = name or f"{dpi}x_{width}x{height}"

    def get_logical_dpi(self) -> float:
        """获取逻辑 DPI（相对于 96 DPI）"""
        return self.dpi / 96.0

    def get_physical_pixels(self, logical_width: int) -> int:
        """将逻辑像素转换为物理像素"""
        return int(logical_width * self.get_logical_dpi())

    def __repr__(self):
        return f"ScreenConfig({self.name}, {self.dpi} DPI, {self.width}x{self.height})"


class MultiScreenEnvironment:
    """多屏环境 - 模拟在不同 DPI 屏幕间移动窗口"""

    # 常见的屏幕配置
    SCREENS = {
        "1x_1920x1080": ScreenConfiguration(96, 1920, 1080, "1x_1920x1080"),
        "1_25x_1920x1080": ScreenConfiguration(120, 1920, 1080, "1.25x_1920x1080"),
        "1_5x_1920x1080": ScreenConfiguration(144, 1920, 1080, "1.5x_1920x1080"),
        "2x_1920x1080": ScreenConfiguration(192, 1920, 1080, "2x_1920x1080"),

        # 笔记本屏幕配置
        "1x_1366x768": ScreenConfiguration(96, 1366, 768, "1x_1366x768"),
        "1_25x_1366x768": ScreenConfiguration(120, 1366, 768, "1.25x_1366x768"),
        "1_5x_1366x768": ScreenConfiguration(144, 1366, 768, "1.5x_1366x768"),
    }

    def __init__(self):
        self.current_screen: Optional[ScreenConfiguration] = None
        self.screen_switches = []

    def set_screen(self, screen_key: str) -> ScreenConfiguration:
        """切换到指定屏幕"""
        if screen_key not in self.SCREENS:
            raise ValueError(f"Unknown screen: {screen_key}")

        screen = self.SCREENS[screen_key]
        self.current_screen = screen
        self.screen_switches.append((screen_key, time.time()))

        return screen

    def switch_from_to(self, from_screen: str, to_screen: str) -> Tuple[ScreenConfiguration, ScreenConfiguration]:
        """从一个屏幕切换到另一个屏幕"""
        self.set_screen(from_screen)
        time.sleep(0.5)  # 模拟窗口拖动延迟

        self.set_screen(to_screen)

        return self.SCREENS[from_screen], self.SCREENS[to_screen]

    def get_current_screen(self) -> Optional[ScreenConfiguration]:
        """获取当前屏幕配置"""
        return self.current_screen

    def get_screen_switches(self) -> list:
        """获取屏幕切换历史"""
        return self.screen_switches


class DPIValidationHelper:
    """DPI 验证帮助器 - 验证 DPI 相关的计算"""

    @staticmethod
    def calculate_logical_height(physical_height: float, dpi_ratio: float) -> float:
        """
        根据物理像素和 DPI 计算逻辑高度

        应该满足：physical = logical * dpi_ratio
        所以：logical = physical / dpi_ratio
        """
        if dpi_ratio <= 0:
            raise ValueError(f"Invalid DPI ratio: {dpi_ratio}")
        return physical_height / dpi_ratio

    @staticmethod
    def calculate_physical_height(logical_height: float, dpi_ratio: float) -> float:
        """
        根据逻辑像素和 DPI 计算物理高度

        应该满足：physical = logical * dpi_ratio
        """
        return logical_height * dpi_ratio

    @staticmethod
    def validate_dpi_consistency(height_1x: float, height_other: float,
                                 dpi_ratio: float, tolerance: float = 0.5) -> bool:
        """
        验证不同 DPI 下的高度一致性

        在逻辑像素下应该相同：
        - height_1x 是 1x DPI 下的物理高度
        - height_other 是其他 DPI 下的物理高度
        - 转换为逻辑高度后应该相同

        logical_1x = height_1x / 1.0
        logical_other = height_other / dpi_ratio

        应该满足：abs(logical_1x - logical_other) <= tolerance
        """
        logical_1x = height_1x / 1.0
        logical_other = height_other / dpi_ratio

        diff = abs(logical_1x - logical_other)
        is_consistent = diff <= tolerance

        return is_consistent

    @staticmethod
    def calculate_line_height_consistency(
        font_size_base: float,
        dpi_1x: float,
        dpi_other: float,
        multiplier: float = 1.4
    ) -> Tuple[float, float]:
        """
        计算不同 DPI 下的行高

        Expected behavior:
        - 在 1x DPI 下：line_height = font_size * multiplier
        - 在其他 DPI 下：逻辑行高应该与 1x 相同

        Returns:
            (logical_line_height, physical_line_height_other)
        """
        logical_line_height = font_size_base * multiplier
        physical_line_height_other = logical_line_height * dpi_other

        return logical_line_height, physical_line_height_other
