"""
渲染验证工具

提供像素级验证、布局验证、坐标验证等功能
"""

import sys
from pathlib import Path
from typing import Optional, Tuple, List, Dict, Any

# 项目路径
PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT))

from tests.common import ScreenCapture


class PixelRegion:
    """像素区域 - 表示一个矩形区域"""

    def __init__(self, x: int, y: int, width: int, height: int):
        self.x = x
        self.y = y
        self.width = width
        self.height = height

    @property
    def x1(self) -> int:
        """左上角 X 坐标"""
        return self.x

    @property
    def y1(self) -> int:
        """左上角 Y 坐标"""
        return self.y

    @property
    def x2(self) -> int:
        """右下角 X 坐标"""
        return self.x + self.width

    @property
    def y2(self) -> int:
        """右下角 Y 坐标"""
        return self.y + self.height

    def overlaps_with(self, other: 'PixelRegion') -> bool:
        """检查是否与另一个区域重叠"""
        return not (self.x2 <= other.x1 or self.x1 >= other.x2 or
                    self.y2 <= other.y1 or self.y1 >= other.y2)

    def contains_point(self, x: int, y: int) -> bool:
        """检查是否包含点"""
        return self.x1 <= x < self.x2 and self.y1 <= y < self.y2

    def distance_to(self, other: 'PixelRegion') -> float:
        """计算到另一个区域的距离（中心点距离）"""
        cx1 = (self.x1 + self.x2) / 2
        cy1 = (self.y1 + self.y2) / 2
        cx2 = (other.x1 + other.x2) / 2
        cy2 = (other.y1 + other.y2) / 2

        return ((cx1 - cx2) ** 2 + (cy1 - cy2) ** 2) ** 0.5

    def __repr__(self):
        return f"PixelRegion({self.x}, {self.y}, {self.width}x{self.height})"


class RenderingValidator:
    """渲染验证器"""

    def __init__(self, logger=None):
        self.logger = logger
        self.validation_results: List[Dict[str, Any]] = []

    def log(self, message: str):
        """记录日志"""
        if self.logger:
            self.logger.info(message)

    def validate_no_overlap(self, regions: List[PixelRegion]) -> bool:
        """
        验证没有重叠

        检查一个列表中的所有区域是否相互不重叠
        """
        self.log(f"Validating no overlaps for {len(regions)} regions...")

        overlaps = []
        for i, region1 in enumerate(regions):
            for j, region2 in enumerate(regions[i+1:], start=i+1):
                if region1.overlaps_with(region2):
                    overlaps.append((i, j, region1, region2))

        if overlaps:
            for i, j, r1, r2 in overlaps:
                self.log(f"  ✗ Overlap detected: region {i} overlaps with region {j}")

            self.validation_results.append({
                "check": "no_overlap",
                "passed": False,
                "overlaps": len(overlaps),
                "details": [{"region1": i, "region2": j} for i, j, _, _ in overlaps]
            })
            return False

        self.log(f"  ✓ No overlaps detected")
        self.validation_results.append({
            "check": "no_overlap",
            "passed": True,
            "regions_checked": len(regions)
        })
        return True

    def validate_alignment(self, regions: List[PixelRegion],
                          alignment_type: str = "vertical_spacing") -> bool:
        """
        验证对齐

        alignment_type:
            - vertical_spacing: 验证垂直间距均匀
            - horizontal_spacing: 验证水平间距均匀
            - baseline: 验证基线对齐
        """
        self.log(f"Validating {alignment_type} for {len(regions)} regions...")

        if alignment_type == "vertical_spacing":
            return self._validate_vertical_spacing(regions)
        elif alignment_type == "horizontal_spacing":
            return self._validate_horizontal_spacing(regions)
        elif alignment_type == "baseline":
            return self._validate_baseline_alignment(regions)

        return False

    def _validate_vertical_spacing(self, regions: List[PixelRegion]) -> bool:
        """验证垂直间距"""
        if len(regions) < 2:
            return True

        # 按 Y 坐标排序
        sorted_regions = sorted(regions, key=lambda r: r.y1)

        spacings = []
        for i in range(len(sorted_regions) - 1):
            current = sorted_regions[i]
            next_region = sorted_regions[i + 1]
            spacing = next_region.y1 - current.y2

            spacings.append(spacing)

        if not spacings:
            return True

        # 检查间距是否均匀（允许 ±2px 的偏差）
        avg_spacing = sum(spacings) / len(spacings)
        tolerance = 2.0

        inconsistent = []
        for i, spacing in enumerate(spacings):
            if abs(spacing - avg_spacing) > tolerance:
                inconsistent.append((i, spacing))

        if inconsistent:
            for i, spacing in inconsistent:
                self.log(f"  ✗ Spacing inconsistency at index {i}: {spacing} (expected ~{avg_spacing})")

            self.validation_results.append({
                "check": "vertical_spacing",
                "passed": False,
                "avg_spacing": avg_spacing,
                "inconsistencies": len(inconsistent)
            })
            return False

        self.log(f"  ✓ Vertical spacing is uniform (~{avg_spacing:.1f}px)")
        self.validation_results.append({
            "check": "vertical_spacing",
            "passed": True,
            "avg_spacing": avg_spacing
        })
        return True

    def _validate_horizontal_spacing(self, regions: List[PixelRegion]) -> bool:
        """验证水平间距"""
        if len(regions) < 2:
            return True

        # 按 X 坐标排序
        sorted_regions = sorted(regions, key=lambda r: r.x1)

        spacings = []
        for i in range(len(sorted_regions) - 1):
            current = sorted_regions[i]
            next_region = sorted_regions[i + 1]
            spacing = next_region.x1 - current.x2

            spacings.append(spacing)

        if not spacings:
            return True

        avg_spacing = sum(spacings) / len(spacings)
        tolerance = 2.0

        inconsistent = []
        for i, spacing in enumerate(spacings):
            if abs(spacing - avg_spacing) > tolerance:
                inconsistent.append((i, spacing))

        if inconsistent:
            self.validation_results.append({
                "check": "horizontal_spacing",
                "passed": False,
                "avg_spacing": avg_spacing,
                "inconsistencies": len(inconsistent)
            })
            return False

        self.log(f"  ✓ Horizontal spacing is uniform (~{avg_spacing:.1f}px)")
        self.validation_results.append({
            "check": "horizontal_spacing",
            "passed": True,
            "avg_spacing": avg_spacing
        })
        return True

    def _validate_baseline_alignment(self, regions: List[PixelRegion]) -> bool:
        """验证基线对齐（所有区域的底部应该在同一水平线上）"""
        if len(regions) < 2:
            return True

        baselines = [r.y2 for r in regions]
        avg_baseline = sum(baselines) / len(baselines)
        tolerance = 2.0

        misaligned = []
        for i, baseline in enumerate(baselines):
            if abs(baseline - avg_baseline) > tolerance:
                misaligned.append((i, baseline))

        if misaligned:
            for i, baseline in misaligned:
                self.log(f"  ✗ Baseline misaligned at region {i}: {baseline} (expected ~{avg_baseline})")

            self.validation_results.append({
                "check": "baseline_alignment",
                "passed": False,
                "avg_baseline": avg_baseline,
                "misaligned_count": len(misaligned)
            })
            return False

        self.log(f"  ✓ Baseline alignment is correct (~{avg_baseline:.1f}px)")
        self.validation_results.append({
            "check": "baseline_alignment",
            "passed": True,
            "baseline_y": avg_baseline
        })
        return True

    def validate_minimum_spacing(self, regions: List[PixelRegion],
                                 min_spacing: float = 5.0,
                                 direction: str = "vertical") -> bool:
        """
        验证最小间距

        确保相邻块之间的间距至少为指定值
        """
        self.log(f"Validating minimum {direction} spacing ({min_spacing}px) for {len(regions)} regions...")

        if len(regions) < 2:
            return True

        if direction == "vertical":
            sorted_regions = sorted(regions, key=lambda r: r.y1)
        else:
            sorted_regions = sorted(regions, key=lambda r: r.x1)

        violations = []
        for i in range(len(sorted_regions) - 1):
            current = sorted_regions[i]
            next_region = sorted_regions[i + 1]

            if direction == "vertical":
                spacing = next_region.y1 - current.y2
            else:
                spacing = next_region.x1 - current.x2

            if spacing < min_spacing:
                violations.append((i, spacing))

        if violations:
            for i, spacing in violations:
                self.log(f"  ✗ Insufficient spacing at index {i}: {spacing}px < {min_spacing}px")

            self.validation_results.append({
                "check": f"minimum_{direction}_spacing",
                "passed": False,
                "violations": len(violations)
            })
            return False

        self.log(f"  ✓ All {direction} spacings >= {min_spacing}px")
        self.validation_results.append({
            "check": f"minimum_{direction}_spacing",
            "passed": True,
            "regions_checked": len(regions)
        })
        return True

    def validate_no_whitespace_gaps(self, regions: List[PixelRegion],
                                    max_gap_ratio: float = 0.2) -> bool:
        """
        验证没有过多空白间隙

        检查相邻块之间是否有过多的空白（超过块高度的指定比例）
        """
        self.log(f"Validating no excessive whitespace gaps (ratio <= {max_gap_ratio})...")

        if len(regions) < 2:
            return True

        sorted_regions = sorted(regions, key=lambda r: r.y1)

        violations = []
        for i in range(len(sorted_regions) - 1):
            current = sorted_regions[i]
            next_region = sorted_regions[i + 1]

            spacing = next_region.y1 - current.y2
            current_height = current.height

            gap_ratio = spacing / current_height if current_height > 0 else 0

            if gap_ratio > max_gap_ratio:
                violations.append((i, gap_ratio, spacing, current_height))

        if violations:
            for i, ratio, spacing, height in violations:
                self.log(f"  ✗ Excessive gap at index {i}: {ratio:.2f} (spacing={spacing}px, height={height}px)")

            self.validation_results.append({
                "check": "whitespace_gaps",
                "passed": False,
                "violations": len(violations)
            })
            return False

        self.log(f"  ✓ All whitespace gaps are within limits")
        self.validation_results.append({
            "check": "whitespace_gaps",
            "passed": True,
            "regions_checked": len(regions)
        })
        return True

    def validate_height_consistency(self, regions: List[PixelRegion],
                                   tolerance_percent: float = 5.0) -> bool:
        """
        验证高度一致性

        检查多个相同类型的块（如代码块行）的高度是否一致
        """
        self.log(f"Validating height consistency (tolerance: {tolerance_percent}%)...")

        if len(regions) < 2:
            return True

        heights = [r.height for r in regions]
        avg_height = sum(heights) / len(heights)
        tolerance = avg_height * tolerance_percent / 100

        inconsistent = []
        for i, height in enumerate(heights):
            if abs(height - avg_height) > tolerance:
                inconsistent.append((i, height))

        if inconsistent:
            for i, height in inconsistent:
                self.log(f"  ✗ Height inconsistency at region {i}: {height}px (expected ~{avg_height:.1f}px)")

            self.validation_results.append({
                "check": "height_consistency",
                "passed": False,
                "avg_height": avg_height,
                "inconsistencies": len(inconsistent)
            })
            return False

        self.log(f"  ✓ Height consistency verified (avg: {avg_height:.1f}px)")
        self.validation_results.append({
            "check": "height_consistency",
            "passed": True,
            "avg_height": avg_height
        })
        return True

    def get_validation_summary(self) -> Dict[str, Any]:
        """获取验证摘要"""
        passed = sum(1 for r in self.validation_results if r.get("passed", False))
        total = len(self.validation_results)

        return {
            "total_checks": total,
            "passed": passed,
            "failed": total - passed,
            "pass_rate": f"{passed / total * 100:.1f}%" if total > 0 else "N/A",
            "details": self.validation_results
        }
