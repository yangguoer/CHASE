# CHASE 项目文档索引

## 📚 文档导航

### 1. 入门文档

| 文档 | 用途 | 适合人群 |
|------|------|---------|
| [README.md](README.md) | 项目介绍、架构概览、快速开始 | 所有人 |
| [QUICKSTART.md](QUICKSTART.md) | 环境搭建、编译运行、常见问题 | 新手 |
| [PROJECT_COMPLETION_REPORT.md](PROJECT_COMPLETION_REPORT.md) | 完成度报告、下一步计划 | 开发者、管理者 |

### 2. 技术文档

| 文档 | 内容 | 深度 |
|------|------|------|
| [CHASE_Requirements.md](CHASE_Requirements.md) | 完整需求规范、算法伪代码、接口定义 | 🔥 核心参考 |
| [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) | 实现总结、模块说明、待办清单 | 开发者 |

### 3. 代码结构

```
CHASE/
├── 📖 文档
│   ├── README.md                      # 项目主页
│   ├── QUICKSTART.md                  # 快速开始
│   ├── IMPLEMENTATION_SUMMARY.md      # 实现总结
│   ├── PROJECT_COMPLETION_REPORT.md   # 完成报告
│   └── CHASE_Requirements.md          # 需求文档（权威）
│
├── 🔧 构建系统
│   ├── CMakeLists.txt                 # 主配置文件
│   ├── build.sh                       # Linux/macOS 构建脚本
│   └── build.bat                      # Windows 构建脚本
│
├── 🎯 核心模块
│   ├── framework/protocol/            # 协议层
│   │   ├── Common.h                   # 基础类型
│   │   ├── Transaction.h/cpp          # 交易
│   │   ├── Block.h/cpp                # 区块
│   │   └── StateSnapshot.h            # 状态快照
│   │
│   ├── scheduler/                     # 调度模块 (CDS核心) ⭐
│   │   ├── KDG.h/cpp                  # 键级依赖图
│   │   ├── DPP.h/cpp                  # 确定性预测协议
│   │   ├── SchedulePlan.h             # 调度计划
│   │   └── CDS.h/cpp                  # 链导向调度器
│   │
│   ├── state/                         # 状态管理 ⭐
│   │   └── StateCache.h/cpp           # 两层缓存 (L1+L2)
│   │
│   ├── pipeline/                      # 流水线控制 ⭐
│   │   └── BlockPipeline.h/cpp        # BLP AIMD窗口
│   │
│   ├── crypto/                        # 密码学
│   │   └── Crypto.h/cpp               # SHA256, Keccak256, Secp256k1
│   │
│   ├── consensus/                     # 共识
│   │   └── TuskEngine.h/cpp           # Tusk BFT框架
│   │
│   ├── txpool/                        # 交易池
│   │   └── TxPool.h/cpp               # 优先级队列
│   │
│   ├── network/                       # 网络
│   │   └── P2PService.h/cpp           # P2P服务框架
│   │
│   ├── executor/                      # 执行
│   │   └── ExecutorInterface.h        # EVM包装器接口
│   │
│   └── storage/                       # 存储
│       ├── StorageInterface.h         # 存储接口
│       └── RocksDBStorage.cpp         # RocksDB框架
│
├── 🚀 入口程序
│   └── main.cpp                       # 节点启动入口
│
├── 🧪 测试
│   └── tests/
│       └── test_core.cpp              # 核心模块测试
│
└── 📦 依赖配置
    └── proto/                         # Protocol Buffers定义
        ├── block.proto
        ├── transaction.proto
        └── ...
```

---

## 🎓 学习路径推荐

### 路径1: 理解架构（1-2天）

1. **阅读 README.md** - 了解项目概况
2. **查看架构图** - 理解四阶段流水线
3. **浏览目录结构** - 熟悉模块划分

### 路径2: 深入研究核心算法（1周）

1. **阅读 CHASE_Requirements.md 第4-6节**
   - 4.4 KDG与DPP
   - 4.5 CDS调度
   - 6. 核心算法规范

2. **阅读源码**
   - `scheduler/KDG.cpp` - 依赖图构建
   - `scheduler/DPP.cpp` - 预测协议
   - `scheduler/CDS.cpp` - 调度算法

3. **运行测试**
   - 编译并运行 `tests/test_core.cpp`
   - 调试理解数据流

### 路径3: 实践开发（2-4周）

1. **集成第一个第三方库**
   - 推荐：libsecp256k1（相对简单）
   - 修改 `crypto/Crypto.cpp`

2. **完善一个模块**
   - 推荐：RocksDB 存储
   - 实现 `storage/RocksDBStorage.cpp`

3. **添加单元测试**
   - 为 DPP 编写测试
   - 验证预测准确性

### 路径4: 性能优化（1-2月）

1. **Profiling**
   - 使用 perf/VTune 找出瓶颈
   - 分析热点函数

2. **并行优化**
   - KDG 构建线程负载均衡
   - CDS 分片分配优化

3. **基准测试**
   - 实现 SmallBank 工作负载
   - 测量 TPS

---

## 🔍 快速查找指南

### 我想了解...

#### 💡 CDS 调度算法
→ 阅读 `CHASE_Requirements.md` 第 4.5 节  
→ 查看 `scheduler/CDS.h/cpp`  
→ 运行 `test_core.cpp` 中的 `testCDS()`

#### 💡 BLP 流水线
→ 阅读 `CHASE_Requirements.md` 第 4.6 节  
→ 查看 `pipeline/BlockPipeline.h/cpp`  
→ 关注 AIMD 窗口控制逻辑

#### 💡 两层状态缓存
→ 阅读 `CHASE_Requirements.md` 第 4.8 节  
→ 查看 `state/StateCache.h/cpp`  
→ 理解 L1 → L2 → RocksDB 查找链

#### 💡 DPP 预测协议
→ 阅读 `CHASE_Requirements.md` 第 4.4 节 (DPP部分)  
→ 查看 `scheduler/DPP.h/cpp`  
→ 理解 FastPath vs SimulateFull

#### 💡 KDG 依赖图
→ 阅读 `CHASE_Requirements.md` 第 4.4 节 (KDG部分)  
→ 查看 `scheduler/KDG.h/cpp`  
→ 理解 RAW/WAR/WAW 依赖类型

---

## 🛠️ 常用命令速查

### 构建

```bash
# Linux/macOS
./build.sh                    # Release模式
./build.sh --debug            # Debug模式
./build.sh --clean            # 清理重建

# Windows
.\build.bat
```

### 运行

```bash
# 启动节点
./build/CHASE_node

# 运行测试
./tests/test_core
```

### 调试

```bash
# GDB调试
gdb ./build/CHASE_node
(gdb) run
(gdb) bt          # 崩溃时查看堆栈
(gdb) break main  # 设置断点

# Valgrind内存检查
valgrind --leak-check=full ./build/CHASE_node
```

### 性能分析

```bash
# perf profiling (Linux)
perf record -g ./build/CHASE_node
perf report

# VTune (Intel)
vtune -collect hotspots ./build/CHASE_node
```

---

## 📊 关键指标

### 代码统计

- **总文件数**: ~50个
- **头文件**: ~25个
- **实现文件**: ~20个
- **测试文件**: 1个（可扩展）
- **文档文件**: 5个
- **总代码行数**: ~4000行

### 模块完成度

| 模块 | 完成度 | 优先级 |
|------|--------|--------|
| 协议层 | 100% | ✅ |
| KDG | 100% | ✅ |
| DPP | 85% | ⚠️ |
| CDS | 90% | ✅ |
| State Cache | 95% | ✅ |
| BLP Pipeline | 90% | ✅ |
| TxPool | 85% | ✅ |
| Consensus | 60% | 🔴 |
| Executor | 70% | 🔴 |
| Network | 50% | 🟡 |
| Storage | 40% | 🔴 |

---

## 🎯 下一步行动

### 立即执行

```bash
# 1. 克隆项目（如果还没有）
git clone <repository-url>
cd CHASE

# 2. 安装依赖（Ubuntu示例）
sudo apt-get install build-essential cmake libssl-dev \
                     libboost-all-dev libgoogle-glog-dev \
                     libyaml-cpp-dev protobuf-compiler

# 3. 构建项目
./build.sh

# 4. 运行测试
cd build
g++ -std=c++20 -I.. ../tests/test_core.cpp \
    -L. -lframework -lcrypto -lscheduler -lstate \
    -o test_core
./test_core

# 5. 启动节点
./CHASE_node
```

### 短期目标（本周）

- [ ] 成功编译项目
- [ ] 运行所有核心测试
- [ ] 阅读 CHASE_Requirements.md 前6章
- [ ] 理解 CDS 调度流程

### 中期目标（本月）

- [ ] 集成 libsecp256k1
- [ ] 完善签名验证
- [ ] 编写 DPP 单元测试
- [ ] 性能 profiling

### 长期目标（本季）

- [ ] 集成 EVMone
- [ ] 集成 RocksDB
- [ ] 实现完整 Narwhal 客户端
- [ ] SmallBank 基准测试

---

## 🆘 获取帮助

### 遇到问题？

1. **查阅文档**
   - [QUICKSTART.md](QUICKSTART.md) - 常见问题
   - [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) - 已知限制

2. **检查编译错误**
   ```bash
   # 查看详细输出
   make VERBOSE=1
   
   # 清理重建
   rm -rf build && ./build.sh
   ```

3. **调试运行时错误**
   ```bash
   # 启用详细日志
   export GLOG_v=3
   ./build/CHASE_node
   
   # 查看日志文件
   ls /tmp/CHASE_node.*
   ```

### 贡献代码

1. Fork 仓库
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 提交 Pull Request

---

## 📞 联系方式

- **项目主页**: [GitHub Repository]
- **问题反馈**: [Issues]
- **讨论区**: [Discussions]
- **邮件列表**: [Mailing List]

---

## 📜 许可证

MIT License - 详见 LICENSE 文件

---

**最后更新**: 2026-05-31  
**维护者**: CHASE Team  
**版本**: v1.0 Framework

---

**Happy Coding! 🚀**
