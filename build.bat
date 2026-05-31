@echo off
REM CHASE Blockchain Node Build Script for Windows

echo =========================================
echo   CHASE Blockchain Node Build Script
echo =========================================

REM Check if CMake is installed
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake is not installed or not in PATH
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure
echo.
echo Configuring project...
cmake .. -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

REM Build
echo.
echo Building project...
cmake --build . --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo Error: Build failed
    exit /b 1
)

echo.
echo =========================================
echo   Build completed successfully!
echo =========================================
echo.
echo Executable: build\Release\CHASE_node.exe
echo.
echo To run the node:
echo   build\Release\CHASE_node.exe
echo.

cd ..
