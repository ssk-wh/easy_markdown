# PowerShell script to test exe launch
Write-Host "Testing SimpleMarkdown.exe launch" -ForegroundColor Green
Write-Host "=" * 60

# Test build/app version
Write-Host "`n[1/2] Testing build/app/SimpleMarkdown.exe"
Write-Host "-" * 60
$proc = Start-Process -FilePath "D:\iflytek_projects\simple_markdown\build\app\SimpleMarkdown.exe" `
    -WorkingDirectory "D:\iflytek_projects\simple_markdown\build\app" `
    -PassThru -NoNewWindow

Start-Sleep -Seconds 2

if ($proc.HasExited) {
    Write-Host "[FAIL] Process exited immediately (exit code: $($proc.ExitCode))"
} else {
    Write-Host "[OK] Process is running!"
    $proc | Stop-Process -Force
}

# Test dist version
Write-Host "`n[2/2] Testing dist/SimpleMarkdown.exe"
Write-Host "-" * 60
$proc = Start-Process -FilePath "D:\iflytek_projects\simple_markdown\dist\SimpleMarkdown.exe" `
    -WorkingDirectory "D:\iflytek_projects\simple_markdown\dist" `
    -PassThru -NoNewWindow

Start-Sleep -Seconds 2

if ($proc.HasExited) {
    Write-Host "[FAIL] Process exited immediately (exit code: $($proc.ExitCode))"
} else {
    Write-Host "[OK] Process is running!"
    $proc | Stop-Process -Force
}

Write-Host "`n" + ("=" * 60)
