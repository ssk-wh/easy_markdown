"""
SimpleMarkdown 自动化测试主入口

运行方式：
  python automated_tests.py --dpi 1x --category rendering
  python automated_tests.py --dpi all --category all --verbose
  python automated_tests.py --dpi 1.5x --category dpi,interaction
"""

import sys
import argparse
import json
from pathlib import Path
from datetime import datetime
from typing import List

# 项目路径
PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT))

from tests.test_framework import TestFrameworkLogger, TestSuite
from tests.test_scenarios import create_all_tests, RenderingTests, DPITests, MouseInteractionTests, EdgeCaseTests
from tests.report_generator import ReportGenerator, generate_summary_text


class TestRunner:
    """测试运行器"""

    def __init__(self, log_dir: str = "tests/logs", report_dir: str = "tests/reports"):
        self.logger = TestFrameworkLogger(log_dir=log_dir)
        self.report_dir = Path(report_dir)
        self.report_dir.mkdir(parents=True, exist_ok=True)

        self.suite = TestSuite(logger=self.logger)

    def add_tests_by_category(self, categories: List[str], dpi_configs: List[str]):
        """根据类别和 DPI 配置添加测试"""
        all_tests = create_all_tests()

        for test in all_tests:
            # 检查类别
            category_match = False
            if "all" in categories:
                category_match = True
            else:
                if isinstance(test, RenderingTests) and "rendering" in categories:
                    category_match = True
                elif isinstance(test, DPITests) and "dpi" in categories:
                    category_match = True
                elif isinstance(test, MouseInteractionTests) and "interaction" in categories:
                    category_match = True
                elif isinstance(test, EdgeCaseTests) and "edge" in categories:
                    category_match = True

            # 检查 DPI 配置
            dpi_match = False
            if "all" in dpi_configs or isinstance(test, (RenderingTests, MouseInteractionTests, EdgeCaseTests)):
                # DPI 测试才需要匹配 DPI 配置
                if isinstance(test, DPITests):
                    if "all" in dpi_configs:
                        dpi_match = True
                    # 其他类型的测试不需要特定的 DPI 配置
                else:
                    dpi_match = True
            else:
                dpi_match = True

            if category_match and dpi_match:
                self.suite.add_test(test)

        print(f"Added {len(self.suite.tests)} tests to suite")

    def run_tests(self, verbose: bool = False) -> dict:
        """运行所有测试"""
        print("\n" + "=" * 60)
        print("  SimpleMarkdown 自动化测试框架")
        print("=" * 60 + "\n")

        start_time = datetime.now()
        print(f"开始时间: {start_time.strftime('%Y-%m-%d %H:%M:%S')}\n")

        results = self.suite.run_all(stop_on_error=False)

        end_time = datetime.now()
        duration = (end_time - start_time).total_seconds()

        print(f"\n完成时间: {end_time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"耗时: {duration:.1f} 秒\n")

        # 获取摘要
        summary = self.suite.get_summary()

        print("=" * 60)
        print("  测试摘要")
        print("=" * 60)
        print(f"总数:  {summary['total']}")
        print(f"通过:  {summary['passed']}")
        print(f"失败:  {summary['failed']}")
        print(f"错误:  {summary['errors']}")
        print(f"成功率: {summary['pass_rate']}")
        print("=" * 60 + "\n")

        # 返回包含额外信息的结果
        results["duration_seconds"] = duration
        results["start_time"] = start_time.isoformat()
        results["end_time"] = end_time.isoformat()
        results["summary"] = summary

        return results

    def generate_reports(self, results: dict):
        """生成测试报告"""
        print("Generating reports...\n")

        # 生成 JSON 报告
        json_report_path = self.report_dir / "results.json"
        with open(json_report_path, 'w', encoding='utf-8') as f:
            json.dump(results, f, indent=2, ensure_ascii=False)
        print(f"[OK] JSON results: {json_report_path}")

        # 生成文本摘要
        text_summary_path = self.report_dir / "summary.txt"
        summary_text = generate_summary_text(results)
        with open(text_summary_path, 'w', encoding='utf-8') as f:
            f.write(summary_text)
        print(f"[OK] Text summary: {text_summary_path}")

        # 生成 HTML 报告
        html_report_path = self.report_dir / "report.html"
        generator = ReportGenerator(results, str(self.report_dir))
        generator.generate_html(html_report_path)
        print(f"[OK] HTML report: {html_report_path}")

        print("\nReports generated successfully!")
        print(f"Report directory: {self.report_dir}\n")


def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description="SimpleMarkdown 自动化测试框架",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python automated_tests.py --dpi 1x --category rendering
  python automated_tests.py --dpi all --category all --verbose
  python automated_tests.py --dpi 1.5x --category dpi
        """
    )

    parser.add_argument(
        "--dpi",
        choices=["1x", "1.25x", "1.5x", "all"],
        default="all",
        help="DPI 配置 (默认: all)"
    )

    parser.add_argument(
        "--category",
        default="all",
        help="测试类别，逗号分隔 (rendering, dpi, interaction, edge, all) (默认: all)"
    )

    parser.add_argument(
        "--verbose",
        action="store_true",
        help="详细输出"
    )

    parser.add_argument(
        "--log-dir",
        default="tests/logs",
        help="日志目录 (默认: tests/logs)"
    )

    parser.add_argument(
        "--report-dir",
        default="tests/reports",
        help="报告目录 (默认: tests/reports)"
    )

    args = parser.parse_args()

    # 解析类别
    if args.category == "all":
        categories = ["all"]
    else:
        categories = [c.strip() for c in args.category.split(",")]

    # 解析 DPI
    if args.dpi == "all":
        dpi_configs = ["all"]
    else:
        dpi_configs = [args.dpi]

    # 创建运行器
    runner = TestRunner(log_dir=args.log_dir, report_dir=args.report_dir)

    # 添加测试
    runner.add_tests_by_category(categories, dpi_configs)

    if not runner.suite.tests:
        print("错误: 没有匹配的测试！")
        print(f"  类别: {categories}")
        print(f"  DPI: {dpi_configs}")
        sys.exit(1)

    # 运行测试
    results = runner.run_tests(verbose=args.verbose)

    # 生成报告
    runner.generate_reports(results)

    # 返回退出码
    if results["failed"] > 0 or results["errors"] > 0:
        sys.exit(1)
    else:
        sys.exit(0)


if __name__ == "__main__":
    main()
