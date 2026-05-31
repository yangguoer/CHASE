# CHASE 快速开始指南

## 1. 环境准备

### Linux (Ubuntu/Debian)

```bash
# 安装编译工具
sudo apt-get update
sudo apt-get install -y build-essential cmake git

# 安装依赖库
sudo apt-get install -y libssl-dev libboost-all-dev libgoogle-glog-dev \
                        libyaml-cpp-dev protobuf-compiler libprotobuf-dev

# 可选：安装RocksDB（生产环境需要）
sudo apt-get install -y librocksdb-dev
```

### macOS

```bash
# 使用Homebrew安装
brew install cmake openssl boost glog yaml-cpp protobuf

# 设置环境变量
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)
```

### Windows

```powershell
# 使用vcpkg安装依赖
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

.\vcpkg install openssl boost glog yaml-cpp protobuf:x64-windows

# 集成到Visual Studio
.\vcpkg integrate install
```

## 2. 克隆项目

```bash
git clone <repository-url>
cd CHASE
```

## 3. 构建项目

### Linux/macOS

```bash
# 方法1：使用构建脚本
chmod +x build.sh
./build.sh

# 方法2：手动构建
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Windows

```powershell
# 方法1：使用批处理脚本
.\build.bat

# 方法2：使用CMake GUI
# 或使用命令行
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## 4. 运行测试

```bash
# 编译测试程序
cd build
g++ -std=c++20 -I../.. ../tests/test_core.cpp \
    -L. -lframework -lcrypto -lscheduler -lstate \
    -o test_core

# 运行测试
./test_core
```

## 5. 启动节点

```bash
# 运行节点
./build/CHASE_node

# 输出示例：
# ========================================
#   CHASE Blockchain Node Starting...
#   Version: 1.0.0
# ========================================
# [INFO] Initializing P2P network...
# [INFO] Initializing transaction pool...
# [INFO] Initializing state cache (L1 + L2)...
# [INFO] Initializing CDS scheduler...
# [INFO] Initializing Tusk consensus engine...
# [INFO] Initializing BLP pipeline...
# [INFO] Starting consensus engine...
# [INFO] Starting BLP pipeline...
# ========================================
#   CHASE Node started successfully!
#   Press Ctrl+C to stop the node
# ========================================
```

## 6. 验证运行状态

节点运行后，每10秒会输出一次状态：

```
[INFO] Pipeline Status: Ordering=0 Scheduling=0 Commit=0 W2(Gas)=0 W3(Bytes)=0
[INFO] TxPool pending: 0
```

## 7. 常见问题

### Q1: 找不到 OpenSSL

**Linux:**
```bash
sudo apt-get install libssl-dev
export OPENSSL_ROOT_DIR=/usr
```

**macOS:**
```bash
brew install openssl
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)
```

**Windows:**
确保vcpkg已正确安装openssl包。

### Q2: 找不到 Boost

**Linux:**
```bash
sudo apt-get install libboost-all-dev
```

**macOS:**
```bash
brew install boost
```

### Q3: CMake版本过低

需要CMake 3.18+：

```bash
# Ubuntu 20.04+
sudo apt-get install cmake

# 或从源码编译
wget https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0.tar.gz
tar -xzf cmake-3.25.0.tar.gz
cd cmake-3.25.0
./bootstrap && make && sudo make install
```

### Q4: 编译错误 "undefined reference"

确保所有子模块的CMakeLists.txt都正确配置：

```bash
# 清理重新构建
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make clean && make -j$(nproc)
```

### Q5: 运行时找不到共享库

**Linux:**
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build
```

**macOS:**
```bash
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:./build
```

## 8. 开发调试

### Debug模式

```bash
./build.sh --debug
# 或
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### 启用日志

```cpp
// 在main.cpp中调整日志级别
FLAGS_minloglevel = 0;  // INFO
FLAGS_minloglevel = 1;  // WARNING
FLAGS_minloglevel = 2;  // ERROR
```

### GDB调试

```bash
gdb ./build/CHASE_node
(gdb) run
(gdb) bt  # 崩溃时查看堆栈
```

## 9. 性能优化建议

### 编译优化

```bash
# Release模式已启用-O3优化
cmake .. -DCMAKE_BUILD_TYPE=Release

# 额外的优化标志
CXXFLAGS="-O3 -march=native -flto" cmake ..
```

### 并行编译

```bash
# 使用所有CPU核心
make -j$(nproc)

# 或使用ninja（更快）
sudo apt-get install ninja-build
cmake .. -G Ninja
ninja
```

## 10. 下一步

- 📖 阅读 [README.md](README.md) 了解架构细节
- 📖 阅读 [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) 了解实现状态
- 🔧 集成 evmone 实现真正的EVM执行
- 🔧 集成 RocksDB 实现持久化存储
- 🧪 编写更多单元测试
- 📊 运行性能基准测试

## 支持

如有问题，请：
1. 查看 [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) 中的TODO清单
2. 查阅 CHASE_Requirements.md 需求文档
3. 提交Issue或Pull Request

---

**Happy Coding! 🚀**
