"""
测试报告生成器

生成 HTML、JSON 和文本格式的测试报告
"""

import json
from pathlib import Path
from typing import Dict, List, Any
from datetime import datetime


class ReportGenerator:
    """报告生成器"""

    def __init__(self, results: Dict[str, Any], report_dir: str = "tests/reports"):
        self.results = results
        self.report_dir = Path(report_dir)
        self.report_dir.mkdir(parents=True, exist_ok=True)

    def generate_html(self, output_file: Path = None) -> str:
        """生成 HTML 报告"""
        if output_file is None:
            output_file = self.report_dir / "report.html"
        else:
            output_file = Path(output_file)

        html_content = self._build_html()

        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(html_content)

        return str(output_file)

    def _build_html(self) -> str:
        """构建 HTML 内容"""
        results = self.results
        summary = results.get("summary", {})

        # 统计信息
        total = summary.get("total", 0)
        passed = summary.get("passed", 0)
        failed = summary.get("failed", 0)
        errors = summary.get("errors", 0)
        pass_rate = summary.get("pass_rate", "N/A")

        # 计算通过率百分比
        if total > 0:
            pass_percent = (passed / total) * 100
        else:
            pass_percent = 0

        # 测试结果列表
        test_results = results.get("results", [])

        # 构建 HTML
        html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SimpleMarkdown 自动化测试报告</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            background: #f5f5f5;
            color: #333;
        }}

        .container {{
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }}

        header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            border-radius: 8px;
            margin-bottom: 30px;
        }}

        header h1 {{
            font-size: 32px;
            margin-bottom: 10px;
        }}

        header p {{
            opacity: 0.9;
            font-size: 14px;
        }}

        .summary {{
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 20px;
            margin-bottom: 30px;
        }}

        .summary-card {{
            background: white;
            padding: 20px;
            border-radius: 8px;
            border-left: 4px solid #ccc;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}

        .summary-card.passed {{
            border-left-color: #28a745;
        }}

        .summary-card.failed {{
            border-left-color: #dc3545;
        }}

        .summary-card.error {{
            border-left-color: #ffc107;
        }}

        .summary-card.total {{
            border-left-color: #667eea;
        }}

        .summary-card h3 {{
            font-size: 12px;
            text-transform: uppercase;
            color: #999;
            margin-bottom: 10px;
            letter-spacing: 1px;
        }}

        .summary-card .value {{
            font-size: 28px;
            font-weight: bold;
            color: #333;
        }}

        .progress-bar {{
            width: 100%;
            height: 8px;
            background: #eee;
            border-radius: 4px;
            overflow: hidden;
            margin-bottom: 10px;
        }}

        .progress-fill {{
            height: 100%;
            background: linear-gradient(90deg, #28a745 0%, #20c997 100%);
            width: {pass_percent}%;
        }}

        .test-results {{
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}

        .test-results h2 {{
            margin-bottom: 20px;
            font-size: 20px;
        }}

        .test-table {{
            width: 100%;
            border-collapse: collapse;
        }}

        .test-table th {{
            background: #f8f9fa;
            padding: 12px;
            text-align: left;
            border-bottom: 2px solid #dee2e6;
            font-weight: 600;
            font-size: 13px;
            color: #495057;
        }}

        .test-table td {{
            padding: 12px;
            border-bottom: 1px solid #dee2e6;
        }}

        .test-table tr:hover {{
            background: #f8f9fa;
        }}

        .status {{
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 12px;
            font-weight: 600;
            display: inline-block;
        }}

        .status.pass {{
            background: #d4edda;
            color: #155724;
        }}

        .status.fail {{
            background: #f8d7da;
            color: #721c24;
        }}

        .status.error {{
            background: #fff3cd;
            color: #856404;
        }}

        .test-id {{
            font-family: 'Courier New', monospace;
            font-size: 12px;
            color: #667eea;
        }}

        footer {{
            margin-top: 30px;
            padding: 20px;
            text-align: center;
            color: #999;
            font-size: 12px;
        }}

        @media (max-width: 768px) {{
            .summary {{
                grid-template-columns: repeat(2, 1fr);
            }}
        }}
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>SimpleMarkdown 自动化测试报告</h1>
            <p>生成于 {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>
        </header>

        <div class="summary">
            <div class="summary-card total">
                <h3>总数</h3>
                <div class="value">{total}</div>
            </div>
            <div class="summary-card passed">
                <h3>通过</h3>
                <div class="value">{passed}</div>
            </div>
            <div class="summary-card failed">
                <h3>失败</h3>
                <div class="value">{failed}</div>
            </div>
            <div class="summary-card error">
                <h3>错误</h3>
                <div class="value">{errors}</div>
            </div>
        </div>

        <div class="test-results">
            <h2>成功率: {pass_rate}</h2>
            <div class="progress-bar">
                <div class="progress-fill"></div>
            </div>

            <table class="test-table">
                <thead>
                    <tr>
                        <th style="width: 20%">测试 ID</th>
                        <th style="width: 40%">测试名称</th>
                        <th style="width: 15%">状态</th>
                        <th style="width: 15%">耗时</th>
                        <th style="width: 10%">消息</th>
                    </tr>
                </thead>
                <tbody>
"""

        # 添加测试结果行
        for result in test_results:
            test_id = result.get("test_id", "")
            name = result.get("name", "")
            status = result.get("status", "UNKNOWN")
            metrics = result.get("metrics", {})
            duration_ms = metrics.get("duration_ms", 0)
            message = result.get("message", "")

            # 决定状态样式
            status_class = status.lower()
            status_text = status

            # 截断消息
            if len(message) > 50:
                message = message[:47] + "..."

            html += f"""
                    <tr>
                        <td><span class="test-id">{test_id}</span></td>
                        <td>{name}</td>
                        <td><span class="status {status_class}">{status_text}</span></td>
                        <td>{duration_ms}ms</td>
                        <td>{message}</td>
                    </tr>
"""

        html += """
                </tbody>
            </table>
        </div>

        <footer>
            <p>SimpleMarkdown 自动化测试框架 - 生成于 """ + datetime.now().strftime("%Y-%m-%d %H:%M:%S") + """</p>
        </footer>
    </div>
</body>
</html>
"""

        return html

    def generate_text_summary(self, output_file: Path = None) -> str:
        """生成文本摘要"""
        if output_file is None:
            output_file = self.report_dir / "summary.txt"
        else:
            output_file = Path(output_file)

        summary_text = generate_summary_text(self.results)

        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(summary_text)

        return str(output_file)


def generate_summary_text(results: Dict[str, Any]) -> str:
    """生成文本摘要内容"""
    summary = results.get("summary", {})

    total = summary.get("total", 0)
    passed = summary.get("passed", 0)
    failed = summary.get("failed", 0)
    errors = summary.get("errors", 0)
    pass_rate = summary.get("pass_rate", "N/A")

    duration = results.get("duration_seconds", 0)
    start_time = results.get("start_time", "")
    end_time = results.get("end_time", "")

    text = f"""
========================================================
     SimpleMarkdown 自动化测试报告
========================================================

测试摘要
--------
总数:      {total}
通过:      {passed}
失败:      {failed}
错误:      {errors}
成功率:    {pass_rate}

时间信息
--------
开始时间:  {start_time}
结束时间:  {end_time}
耗时:      {duration:.1f} 秒

测试详情
--------
"""

    test_results = results.get("results", [])

    # 统计各状态的测试
    passed_tests = [r for r in test_results if r.get("status") == "PASS"]
    failed_tests = [r for r in test_results if r.get("status") == "FAIL"]
    error_tests = [r for r in test_results if r.get("status") == "ERROR"]

    if passed_tests:
        text += f"\n✓ 通过 ({len(passed_tests)}):\n"
        for test in passed_tests:
            text += f"  - {test.get('test_id')}: {test.get('name')}\n"

    if failed_tests:
        text += f"\n✗ 失败 ({len(failed_tests)}):\n"
        for test in failed_tests:
            text += f"  - {test.get('test_id')}: {test.get('name')}\n"
            if test.get('message'):
                text += f"    消息: {test.get('message')}\n"

    if error_tests:
        text += f"\n⚠ 错误 ({len(error_tests)}):\n"
        for test in error_tests:
            text += f"  - {test.get('test_id')}: {test.get('name')}\n"
            if test.get('message'):
                text += f"    消息: {test.get('message')}\n"

    text += """
========================================================
"""

    return text
