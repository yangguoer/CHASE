#!/bin/bash
# CHASE 测试编译和运行脚本 (Linux/macOS)

echo "========================================"
echo "  CHASE Test Build and Run Script"
echo "========================================"
echo ""

cd ..

# 检查编译器
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ not found. Please install GCC."
    exit 1
fi

echo "Compiling test_core.cpp..."
echo ""

# 编译测试（包含所有必要的源文件）
g++ -std=c++20 -I. \
    tests/test_core.cpp \
    framework/protocol/Transaction.cpp \
    framework/protocol/Block.cpp \
    scheduler/src/KDG.cpp \
    scheduler/src/DPP.cpp \
    scheduler/src/CDS.cpp \
    state/src/StateCache.cpp \
    -o tests/test_core \
    -pthread \
    -lssl -lcrypto

if [ $? -ne 0 ]; then
    echo ""
    echo "Error: Compilation failed!"
    echo ""
    echo "Tips:"
    echo "  1. Make sure you have g++ with C++20 support (GCC 10+)"
    echo "  2. Install required dependencies:"
    echo "     Ubuntu: sudo apt-get install libssl-dev"
    echo "     macOS:  brew install openssl"
    echo "  3. Check error messages above"
    exit 1
fi

echo ""
echo "========================================"
echo "  Compilation successful!"
echo "========================================"
echo ""
echo "Running tests..."
echo ""

./tests/test_core

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "  All tests completed successfully!"
    echo "========================================"
else
    echo ""
    echo "========================================"
    echo "  Tests failed!"
    echo "========================================"
fi
