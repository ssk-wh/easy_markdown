#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
调试脚本：检查 render_blocks.json 生成问题
"""

import sys
import time
import tempfile
import json
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT))

from tests.common import ApplicationProxy, ScreenCapture

def debug_render_blocks():
    """调试 render_blocks.json 生成"""

    print("=== Debug: render_blocks.json generation ===\n")

    # 0. 杀掉现有进程
    import subprocess
    try:
        subprocess.run(["taskkill", "/IM", "SimpleMarkdown.exe", "/F"],
                      capture_output=True, timeout=5)
        print("[0] Killed existing SimpleMarkdown processes")
        import time
        time.sleep(1)
    except:
        pass

    # 1. 清除之前的 json 和调试文件
    temp_path = Path(tempfile.gettempdir()) / "render_blocks.json"
    debug_path = Path(tempfile.gettempdir()) / "render_blocks_debug.txt"
    if temp_path.exists():
        temp_path.unlink()
    if debug_path.exists():
        debug_path.unlink()
    print(f"[1] Cleaned old files\n")

    # 2. 启动应用（需要设置Qt依赖库路径）
    exe_path = str(PROJECT_ROOT / "build" / "app" / "SimpleMarkdown.exe")
    print(f"[2] Launching app: {exe_path}")

    # 设置环境变量以包含Qt库路径
    import os
    import subprocess
    env = os.environ.copy()
    qt_path = "D:\\Qt\\Qt5.12.9\\5.12.9\\msvc2017_64\\bin"
    if qt_path not in env.get('PATH', ''):
        env['PATH'] = qt_path + ";" + env.get('PATH', '')

    # 直接启动应用（以便控制环境和诊断）
    app_process = subprocess.Popen(
        [exe_path],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=env
    )
    time.sleep(3)

    # 检查应用是否成功启动
    is_running = app_process.poll() is None
    if not is_running:
        print("    App crashed or failed to start")
        try:
            stdout, stderr = app_process.communicate(timeout=1)
            if stdout:
                print(f"    [stdout]: {stdout[:200]}")
            if stderr:
                print(f"    [stderr]: {stderr[:200]}")
        except:
            pass
    else:
        print("    App launched successfully")

    # 创建ApplicationProxy并绑定进程
    app = ApplicationProxy(exe_path=exe_path, timeout=3000)
    app.process = app_process
    print(f"    Is running: {is_running}\n")

    # 3. 创建测试 markdown 文件
    md_content = """# Test Code Block

This is a test markdown file.

```cpp
#include <iostream>
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```

End of file.
"""

    temp_md_dir = PROJECT_ROOT / "build" / "app" / "temp_test"
    temp_md_dir.mkdir(parents=True, exist_ok=True)
    temp_md_file = temp_md_dir / "test_debug.md"
    temp_md_file.write_text(md_content, encoding='utf-8')
    print(f"[3] Created test file: {temp_md_file}\n")

    # 4. 打开文件
    print(f"[4] Opening file in app...")
    app.open_file(str(temp_md_file))
    print("    File opening command sent")
    time.sleep(1)

    # 等待应用加载和解析markdown，并验证应用仍在运行
    for i in range(10):
        if app.is_running():
            print(f"    App still running (check {i+1}/10)")
            time.sleep(0.5)
        else:
            print(f"    App crashed!")
            break
    print("    Waited for file loading\n")

    # 5. 取截图（触发绘制）
    print(f"[5] Taking screenshot (should trigger painting)...")
    screenshots_dir = PROJECT_ROOT / "tests" / "screenshots"
    screenshots_dir.mkdir(parents=True, exist_ok=True)
    screenshot_path = screenshots_dir / "debug_1.png"
    screenshot = ScreenCapture.take_screenshot(str(screenshot_path))
    print(f"    Screenshot saved: {screenshot_path}")
    time.sleep(1)
    print(f"    Screenshot object: {screenshot}\n")

    # 6. 检查调试文件
    print(f"[6] Checking for debug output...")
    debug_path = Path(tempfile.gettempdir()) / "render_blocks_debug.txt"
    if debug_path.exists():
        print(f"    DEBUG FILE FOUND: {debug_path}")
        with open(debug_path) as f:
            content = f.read()
            print(f"    Content:\n{content}")
    else:
        print(f"    DEBUG FILE NOT FOUND (paint() may not be called)")

    # 6b. 检查 render_blocks.json
    print(f"\n[6b] Checking for render_blocks.json...")
    if temp_path.exists():
        print(f"    FOUND at: {temp_path}")
        with open(temp_path) as f:
            data = json.load(f)
            blocks = data.get('blocks', [])
            print(f"    Blocks count: {len(blocks)}")
            print(f"    Viewport: {data.get('viewport_width')}x{data.get('viewport_height')}")
            if blocks:
                print(f"\n    First block:")
                for key, val in blocks[0].items():
                    print(f"      {key}: {val}")
    else:
        print(f"    NOT FOUND at: {temp_path}")
        print(f"\n    Temp directory render_blocks* files:")
        temp_dir = Path(tempfile.gettempdir())
        found_any = False
        for item in temp_dir.glob("render_blocks*"):
            print(f"      {item}")
            found_any = True
        if not found_any:
            print(f"      (none)")

    # 7. 关闭应用
    print(f"\n[7] Closing app...")
    app.quit()
    print("    Done\n")

if __name__ == "__main__":
    debug_render_blocks()
