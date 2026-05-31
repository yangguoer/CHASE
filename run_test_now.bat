@echo off
cd /d f:\C++\CHASE

echo Compiling...
g++ -std=c++17 -I. tests/test_core.cpp scheduler/src/KDG.cpp scheduler/src/DPP.cpp scheduler/src/CDS.cpp state/src/StateCache.cpp -o tests/test_core.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    exit /b 1
)

echo.
echo Running tests...
echo.
tests\test_core.exe

echo.
echo Exit code: %ERRORLEVEL%
