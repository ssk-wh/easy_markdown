#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Diagnose if exe can launch"""
import subprocess
import os
import sys
import time

# Force UTF-8 output
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

def test_exe(exe_path):
    """Test if exe can launch"""
    print(f"Testing: {exe_path}")
    print(f"Exists: {os.path.exists(exe_path)}")
    print(f"Size: {os.path.getsize(exe_path)} bytes")
    print()

    # Set QT debug
    env = os.environ.copy()
    env['QT_DEBUG_PLUGINS'] = '1'
    env['QT_QPA_PLATFORM'] = 'windows'

    # Try to launch
    print("Launching process...")
    try:
        proc = subprocess.Popen(
            exe_path,
            cwd=os.path.dirname(exe_path),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=env
        )

        # Wait 3 seconds
        time.sleep(3)

        # Check process status
        poll_result = proc.poll()

        if poll_result is None:
            print("[OK] Process running, launch successful!")
            proc.terminate()
            proc.wait(timeout=5)
            return True
        else:
            print(f"[FAIL] Process exited with code: {poll_result}")

            # Get output
            stdout, stderr = proc.communicate()
            if stdout:
                print(f"\nstdout:\n{stdout.decode('utf-8', errors='ignore')}")
            if stderr:
                print(f"\nstderr:\n{stderr.decode('utf-8', errors='ignore')}")
            return False

    except Exception as e:
        print(f"[FAIL] Launch error: {e}")
        return False

if __name__ == '__main__':
    print("=" * 60)
    print("SimpleMarkdown exe Diagnostic")
    print("=" * 60)
    print()

    build_exe = r"D:\iflytek_projects\simple_markdown\build\app\SimpleMarkdown.exe"
    dist_exe = r"D:\iflytek_projects\simple_markdown\dist\SimpleMarkdown.exe"

    print("[ 1/2 ] Testing build/app exe")
    print("-" * 60)
    build_ok = test_exe(build_exe)
    print()

    print("[ 2/2 ] Testing dist exe")
    print("-" * 60)
    dist_ok = test_exe(dist_exe)
    print()

    print("=" * 60)
    print("Results:")
    print(f"  build/app: {'OK' if build_ok else 'FAIL'}")
    print(f"  dist:      {'OK' if dist_ok else 'FAIL'}")
    print("=" * 60)
