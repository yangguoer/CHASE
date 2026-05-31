# CHASE 测试指南

## 快速开始

### Windows 用户

```powershell
# 方法1：使用提供的脚本（推荐）
.\run_tests.bat

# 方法2：手动编译
cd tests
g++ -std=c++20 -I.. test_core.cpp ^
    ..\framework\protocol\Transaction.cpp ^
    ..\framework\protocol\Block.cpp ^
    ..\scheduler\src\KDG.cpp ^
    ..\scheduler\src\DPP.cpp ^
    ..\scheduler\src\CDS.cpp ^
    ..\state\src\StateCache.cpp ^
    -o test_core.exe
.\test_core.exe
```

### Linux/macOS 用户

```bash
# 方法1：使用提供的脚本（推荐）
chmod +x run_tests.sh
./run_tests.sh

# 方法2：手动编译
cd tests
g++ -std=c++20 -I.. test_core.cpp \
    ../framework/protocol/Transaction.cpp \
    ../framework/protocol/Block.cpp \
    ../scheduler/src/KDG.cpp \
    ../scheduler/src/DPP.cpp \
    ../scheduler/src/CDS.cpp \
    ../state/src/StateCache.cpp \
    -o test_core \
    -pthread -lssl -lcrypto
./test_core
```

## 前提条件

### Windows
1. 安装 [MSYS2](https://www.msys2.org/) 或 [MinGW-w64](https://www.mingw-w64.org/)
2. 确保 `g++` 在 PATH 中
3. 验证版本：`g++ --version`（需要支持 C++20）

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev
```

### macOS
```bash
brew install gcc openssl
```

## 常见问题

### Q1: "fatal error: framework/protocol/Common.h: No such file or directory"

**原因**: 头文件路径不正确

**解决**: 
- 确保从项目根目录编译
- 使用 `-I..` 指定包含路径
- 或使用提供的脚本自动处理路径

### Q2: "undefined reference to ..."

**原因**: 缺少源文件或库

**解决**:
- 确保编译所有必要的 `.cpp` 文件
- Windows: 添加 `-lpthread`
- Linux/macOS: 添加 `-pthread -lssl -lcrypto`

### Q3: "g++: error: unrecognized command line option '-std=c++20'"

**原因**: GCC 版本过低

**解决**:
- Ubuntu: `sudo apt-get install g++-10` 然后使用 `g++-10`
- macOS: `brew install gcc` 使用 `g++-11` 或更高
- Windows: 更新 MSYS2/MinGW

### Q4: OpenSSL 相关错误

**原因**: 缺少 OpenSSL 开发库

**解决**:
- Ubuntu: `sudo apt-get install libssl-dev`
- macOS: `brew install openssl`
- Windows (MSYS2): `pacman -S mingw-w64-x86_64-openssl`

## 测试内容

当前测试覆盖：

1. ✅ **Transaction 测试**
   - 交易创建
   - 哈希计算

2. ✅ **KDG 测试**
   - 操作添加
   - 依赖关系推导（RAW/WAR/WAW）
   - 交易ID收集

3. ✅ **CDS 测试**
   - 调度计划生成
   - CFZ/CZ 链提取
   - 分片分配

4. ✅ **StateCache 测试**
   - L1/L2 缓存读写
   - 区块 finalize

## 扩展测试

要添加新的测试，编辑 `tests/test_core.cpp`：

```cpp
void testYourFeature() {
    std::cout << "Testing Your Feature..." << std::endl;
    
    // 你的测试代码
    
    std::cout << "  Test passed!" << std::endl << std::endl;
}

int main() {
    // ...
    testYourFeature();  // 添加到这里
    // ...
}
```

## 调试技巧

### 启用详细输出

```bash
# Linux/macOS
g++ -std=c++20 -g -O0 -I.. ...  # -g 调试信息, -O0 禁用优化
gdb ./test_core
```

### Windows (GDB)

```powershell
gdb test_core.exe
(gdb) run
(gdb) bt          # 崩溃时查看堆栈
(gdb) break main  # 设置断点
```

## 性能测试

对于性能基准测试，使用 Release 模式编译：

```bash
# Linux/macOS
g++ -std=c++20 -O3 -DNDEBUG -I.. ...

# Windows
g++ -std=c++20 -O3 -DNDEBUG -I.. ...
```

## 下一步

完成基础测试后：

1. 阅读 [CHASE_Requirements.md](../CHASE_Requirements.md) 第10节了解完整测试规范
2. 实现更多单元测试（DPP, BLP等）
3. 编写集成测试（多模块交互）
4. 运行性能基准（SmallBank, CPUHeavy）

---

**Happy Testing! 🧪**
