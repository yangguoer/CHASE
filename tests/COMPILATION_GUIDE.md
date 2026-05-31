# CHASE 测试编译指南

## 问题说明

之前编译失败的原因是头文件路径问题。所有头文件都使用**项目根目录**作为基准路径，因此必须从项目根目录编译。

## ✅ 正确的编译方法

### Windows

```powershell
# 方法1：使用提供的脚本（推荐）
cd f:\C++\CHASE
.\run_tests.bat

# 方法2：手动编译（从项目根目录）
cd f:\C++\CHASE
g++ -std=c++17 -I. ^
    tests/test_core.cpp ^
    framework/protocol/Transaction.cpp ^
    framework/protocol/Block.cpp ^
    scheduler/src/KDG.cpp ^
    scheduler/src/DPP.cpp ^
    scheduler/src/CDS.cpp ^
    state/src/StateCache.cpp ^
    crypto/src/Crypto.cpp ^
    -o tests/test_core.exe ^
    -lpthread

tests\test_core.exe
```

### Linux/macOS

```bash
# 方法1：使用提供的脚本（推荐）
cd /path/to/CHASE
chmod +x run_tests.sh
./run_tests.sh

# 方法2：手动编译（从项目根目录）
cd /path/to/CHASE
g++ -std=c++17 -I. \
    tests/test_core.cpp \
    framework/protocol/Transaction.cpp \
    framework/protocol/Block.cpp \
    scheduler/src/KDG.cpp \
    scheduler/src/DPP.cpp \
    scheduler/src/CDS.cpp \
    state/src/StateCache.cpp \
    crypto/src/Crypto.cpp \
    -o tests/test_core \
    -pthread -lssl -lcrypto

./tests/test_core
```

## ❌ 错误的编译方法

```bash
# 错误：从 tests 目录编译，会导致头文件路径错误
cd tests
g++ test_core.cpp -o test_core  # ✗ 找不到头文件
```

## 头文件路径规则

所有头文件都使用**项目根目录**为基准：

```cpp
// ✅ 正确
#include "framework/protocol/Common.h"
#include "scheduler/include/KDG.h"
#include "state/include/StateCache.h"

// ❌ 错误（旧代码）
#include "../framework/protocol/Common.h"
#include "../../scheduler/include/KDG.h"
```

## 编译参数说明

- `-std=c++17`: 使用 C++17 标准（兼容性更好）
- `-I.`: 将当前目录（项目根目录）添加到包含路径
- `-lpthread`: 链接 pthread 库（多线程支持）
- `-lssl -lcrypto`: 链接 OpenSSL 库（Linux/macOS 需要）

## 常见问题

### Q1: "fatal error: framework/protocol/Common.h: No such file or directory"

**原因**: 没有从项目根目录编译，或者缺少 `-I.` 参数

**解决**: 
```bash
cd f:\C++\CHASE  # 确保在项目根目录
g++ -std=c++17 -I. ...  # 添加 -I. 参数
```

### Q2: "undefined reference to ..."

**原因**: 缺少源文件或库

**解决**: 确保编译所有必要的 `.cpp` 文件：
- `framework/protocol/Transaction.cpp`
- `framework/protocol/Block.cpp`
- `scheduler/src/KDG.cpp`
- `scheduler/src/DPP.cpp`
- `scheduler/src/CDS.cpp`
- `state/src/StateCache.cpp`
- `crypto/src/Crypto.cpp`

### Q3: "cannot find -lpthread"

**Windows**: pthread 通常不需要单独链接，移除 `-lpthread`  
**Linux**: 确保安装了 `libpthread`（通常已预装）

## 简化测试

如果完整测试编译困难，可以先运行简化版测试：

```bash
cd f:\C++\CHASE
g++ -std=c++17 tests/test_simple.cpp -o tests/test_simple.exe
tests\test_simple.exe
```

这个测试不依赖任何外部文件，可以快速验证编译器是否正常工作。

## 下一步

成功编译并运行测试后：

1. 查看测试结果，确认核心模块工作正常
2. 阅读 [CHASE_Requirements.md](../CHASE_Requirements.md) 深入理解算法
3. 尝试修改测试，添加自己的测试用例
4. 使用 CMake 构建完整项目（需要安装依赖库）

---

**Happy Testing! 🧪**
