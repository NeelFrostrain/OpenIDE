@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   MyIDE - Clean Build ^& Run Script
echo ===================================================

:: 1. Locate Visual Studio vcvars64.bat
set "VCVARS=D:\Applications\VS\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
        set "VCVARS=%%i\VC\Auxiliary\Build\vcvars64.bat"
    )
)

if not exist "%VCVARS%" (
    echo [ERROR] Could not locate vcvars64.bat for MSVC environment.
    pause
    exit /b 1
)

echo [1/5] Initializing MSVC x64 Environment...
call "%VCVARS%" >nul 2>&1

:: 2. Configure Tool Paths
set "CMAKE_EXE=C:\Users\neelo\AppData\Local\Python\pythoncore-3.14-64\Scripts\cmake.exe"
set "NINJA_EXE=C:\Users\neelo\AppData\Local\Python\pythoncore-3.14-64\Scripts\ninja.exe"
set "QT_PATH=C:\Qt\6.8.0\msvc2022_64"

where cmake >nul 2>&1 && set "CMAKE_EXE=cmake"
where ninja >nul 2>&1 && set "NINJA_EXE=ninja"

:: 3. Configure with CMake & Ninja (--fresh forces fresh generation)
echo [2/5] Configuring CMake project (fresh build)...
"%CMAKE_EXE%" -B build --fresh -G Ninja -DCMAKE_PREFIX_PATH="%QT_PATH%" -DCMAKE_MAKE_PROGRAM="%NINJA_EXE%"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b %ERRORLEVEL%
)

:: 4. Clean previous targets
echo [3/5] Cleaning previous targets...
"%CMAKE_EXE%" --build build --target clean >nul 2>&1

:: 5. Compile MyIDE
echo [4/5] Compiling MyIDE...
"%CMAKE_EXE%" --build build
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %ERRORLEVEL%
)

:: 6. Launch MyIDE
echo [5/5] Launching MyIDE.exe...
set "PATH=%QT_PATH%\bin;%PATH%"
start "" "build\MyIDE.exe"

echo ===================================================
echo   MyIDE Build Complete and Running!
echo ===================================================
