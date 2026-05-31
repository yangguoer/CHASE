#!/bin/bash
# CHASE 区块链节点构建脚本

set -e

echo "========================================="
echo "  CHASE Blockchain Node Build Script"
echo "========================================="

# 检查CMake版本
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
echo "CMake version: $CMAKE_VERSION"

# 创建构建目录
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
    echo "Created build directory"
fi

cd "$BUILD_DIR"

# 解析命令行参数
BUILD_TYPE="Release"
BUILD_TESTS=OFF
BUILD_BENCHMARKS=OFF

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --with-tests)
            BUILD_TESTS=ON
            shift
            ;;
        --with-benchmarks)
            BUILD_BENCHMARKS=ON
            shift
            ;;
        --clean)
            echo "Cleaning build directory..."
            cd ..
            rm -rf "$BUILD_DIR"
            mkdir -p "$BUILD_DIR"
            cd "$BUILD_DIR"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: ./build.sh [--debug] [--with-tests] [--with-benchmarks] [--clean]"
            exit 1
            ;;
    esac
done

echo "Build type: $BUILD_TYPE"
echo "Build tests: $BUILD_TESTS"
echo "Build benchmarks: $BUILD_BENCHMARKS"

# 配置
echo ""
echo "Configuring project..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DBUILD_TESTS=$BUILD_TESTS \
    -DBUILD_BENCHMARKS=$BUILD_BENCHMARKS

# 编译
echo ""
echo "Building project..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    MAKE_JOBS=$(sysctl -n hw.ncpu)
else
    # Linux
    MAKE_JOBS=$(nproc)
fi

make -j$MAKE_JOBS

echo ""
echo "========================================="
echo "  Build completed successfully!"
echo "========================================="
echo ""
echo "Executable: build/CHASE_node"
echo ""
echo "To run the node:"
echo "  ./build/CHASE_node"
echo ""
