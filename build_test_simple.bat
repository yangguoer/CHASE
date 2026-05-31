@echo off
REM CHASE 简化测试编译脚本 (Windows)

echo ========================================
echo   CHASE Simple Test Build Script
echo ========================================
echo.

cd ..

REM 检查编译器
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: g++ not found. Please install MinGW or MSYS2.
    echo Download from: https://www.mingw-w64.org/
    exit /b 1
)

echo Checking g++ version...
g++ --version | findstr "g++"
echo.

echo Compiling test_core.cpp with minimal dependencies...
echo.

REM 只编译最基本的测试，不依赖外部库
g++ -std=c++17 -I. ^
    tests/test_core.cpp ^
    -o tests/test_core_simple.exe ^
    2>&1

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo Compilation failed!
    echo ========================================
    echo.
    echo Trying to identify missing dependencies...
    echo.
    
    REM 尝试找出缺少的文件
    if exist "tests\test_core.cpp" (
        echo Checking Transaction.h...
        type framework\protocol\Transaction.h | findstr "#include" 
        echo.
    )
    
    exit /b 1
)

echo.
echo ========================================
echo   Compilation successful!
echo ========================================
echo.
echo Running tests...
echo.

tests\test_core_simple.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   All tests passed!
    echo ========================================
) else (
    echo.
    echo ========================================
    echo   Tests failed with code %ERRORLEVEL%
    echo ========================================
)
