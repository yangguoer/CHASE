@echo off
REM CHASE Pipeline Integration Test Script

echo ========================================
echo   CHASE Pipeline Integration Tests
echo ========================================
echo.

where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: g++ not found. Please install MinGW or MSYS2.
    exit /b 1
)

echo Compiling pipeline tests...
echo.

g++ -std=c++17 -I. tests/test_pipeline.cpp scheduler/src/KDG.cpp scheduler/src/DPP.cpp scheduler/src/CDS.cpp state/src/StateCache.cpp -o tests/test_pipeline.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Compilation failed!
    echo Please check the error messages above.
    exit /b 1
)

echo ========================================
echo   Compilation successful!
echo ========================================
echo.
echo Running pipeline integration tests...
echo.

tests\test_pipeline.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   All pipeline tests PASSED!
    echo ========================================
) else (
    echo.
    echo Pipeline tests failed with code %ERRORLEVEL%
)
