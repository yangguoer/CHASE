@echo off
REM CHASE Test Build Script

echo ========================================
echo   CHASE Test Build Script
echo ========================================
echo.

where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: g++ not found. Please install MinGW or MSYS2.
    exit /b 1
)

echo Compiling from project root directory...
echo.

g++ -std=c++17 -I. tests/test_core.cpp scheduler/src/KDG.cpp scheduler/src/DPP.cpp scheduler/src/CDS.cpp state/src/StateCache.cpp -o tests/test_core.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Compilation failed!
    echo Please check the error messages above.
    exit /b 1
)

echo.
echo ========================================
echo   Compilation successful!
echo ========================================
echo.
echo Running tests...
echo.

tests\test_core.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   All tests passed!
    echo ========================================
) else (
    echo.
    echo Tests failed with code %ERRORLEVEL%
)
