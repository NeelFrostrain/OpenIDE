# build_and_run.ps1 - Builds and launches MyIDE for debugging

$ErrorActionPreference = "Stop"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host " Building and Running MyIDE (Debug mode) " -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# Set environment PATH for CMake, Ninja, and Qt 6.8.0
$env:PATH = "C:\Users\neelo\AppData\Local\Python\pythoncore-3.14-64\Scripts;C:\Qt\6.8.0\msvc2022_64\bin;" + $env:PATH

# Force kill any previous running myide instances to prevent multiple windows
cmd.exe /c "taskkill /F /IM myide.exe >NUL 2>&1"
Start-Sleep -Milliseconds 500

# Run CMake configure & build inside MSVC environment
$vsDevCmd = "D:\Applications\VS\Common7\Tools\VsDevCmd.bat"
$cmd = "call `"$vsDevCmd`" -arch=x64 && cmake -B build -G `"Ninja`" -DCMAKE_PREFIX_PATH=`"C:\Qt\6.8.0\msvc2022_64`" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build build"

Write-Host "`n[1/3] Compiling C++ application with MSVC, CMake, & Ninja..." -ForegroundColor Yellow
cmd.exe /c $cmd

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n[ERROR] Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "`n[2/3] Deploying Qt runtime dependencies..." -ForegroundColor Yellow
windeployqt.exe --debug build\myide.exe | Out-Null

Write-Host "`n[3/3] Launching MyIDE..." -ForegroundColor Green
$process = Start-Process -FilePath "build\myide.exe" -WorkingDirectory "e:\Projects\nscode" -PassThru

Write-Host "`nMyIDE is running successfully (PID: $($process.Id))!" -ForegroundColor Green
