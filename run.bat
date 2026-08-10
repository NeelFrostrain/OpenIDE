@echo off
setlocal

set "QT_PATH=C:\Qt\6.8.0\msvc2022_64"
set "PATH=%QT_PATH%\bin;%PATH%"

if not exist "build\MyIDE.exe" (
    echo [INFO] MyIDE.exe not found. Running clean build first...
    call build_and_run.bat
    exit /b %ERRORLEVEL%
)

echo Launching MyIDE...
start "" "build\MyIDE.exe"
