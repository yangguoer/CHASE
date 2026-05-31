# CHASE Blockchain System

CHASE (Chain-oriented, High-performance, And Scalable Execution) 是一个面向许可区块链的高性能确定性执行系统。

## 核心特性

- **BLP (Block-Level Pipelining)**: 区块级流水线，解耦共识/调度/执行/提交四个阶段，实现跨区块并行
- **CDS (Chain-oriented Deterministic Scheduling)**: 链导向确定性调度，以"依赖链"作为最小调度单元
- **DPP (Deterministic Prediction Protocol)**: 确定性预测协议，通过轻量级栈追踪与感知机分支预测提取读写集
- **Tusk BFT Consensus**: 基于DAG结构的拜占庭容错共识
- **Two-Layer State Cache**: L1未提交状态 + L2已提交状态，支持跨区块状态访问

## 架构概览

```
┌─────────────────────────────────────────────────────────┐
│                      Client RPC Layer                    │
└────────────────────────┬────────────────────────────────┘
                         │ 交易提交
┌────────────────────────▼────────────────────────────────┐
│                   Transaction Pool                       │
└────────────────────────┬────────────────────────────────┘
                         │ 打包
┌────────────────────────▼────────────────────────────────┐
│         Stage 1: Ordering (Tusk BFT Consensus)          │
└────────────────────────┬────────────────────────────────┘
                         │ BLP 流水线并行推进
┌────────────────────────▼────────────────────────────────┐
│         Stage 2: Scheduling (CDS)                       │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────┐ │
│  │  DPP模拟执行  │→ │  KDG并行构建 │→ │  依赖链提取    │ │
│  └──────────────┘  └──────────────┘  └───────┬───────┘ │
│                                       ┌───────▼───────┐ │
│                                       │ 调度计划 Π     │ │
│                                       └───────────────┘ │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────┐
│         Stage 3: Execution                              │
│  CFZ(无锁并行) → Barrier → CZ(冲突区执行+重执行检查)      │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────┐
│         Stage 4: Commit                                 │
│  验证结果 → 持久化DeltaPage → 输出最终区块               │
└─────────────────────────────────────────────────────────┘
```

## 目录结构

```
CHASE/
├── framework/              # 核心接口与数据结构
│   └── protocol/          # 协议定义（Block, Transaction等）
├── crypto/                 # 密码学模块（SHA256, Keccak256, secp256k1）
├── storage/                # 持久化存储（RocksDB封装）
├── network/                # P2P网络模块
├── txpool/                 # 交易池模块
├── consensus/              # 共识模块（Tusk BFT）
├── scheduler/              # 调度模块（CDS核心）
│   ├── KDG.h/cpp          # 键级依赖图
│   ├── DPP.h/cpp          # 确定性预测协议
│   └── CDS.h/cpp          # 链导向调度器
├── executor/               # 执行模块（EVM封装）
├── state/                  # 状态管理（两层缓存）
│   └── StateCache.h/cpp   # L1+L2缓存实现
├── pipeline/               # BLP流水线控制
│   └── BlockPipeline.h/cpp # 流水线主控制器
├── proto/                  # Protocol Buffers定义
└── main.cpp                # 节点启动入口
```

## 构建要求

- C++20 编译器（GCC 10+ / Clang 12+ / MSVC 2019+）
- CMake 3.18+
- OpenSSL 1.1.1+
- Boost 1.74+
- Google glog
- yaml-cpp
- Protocol Buffers 3.15+

### 可选依赖（生产环境）

- RocksDB 7.0+（持久化存储）
- evmone 0.10+（EVM执行）
- libsecp256k1 0.3+（签名验证）

## 构建步骤

```bash
# 创建构建目录
mkdir -p build && cd build

# 配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
make -j$(nproc)

# 运行
./CHASE_node
```

## 快速开始

```cpp
// 示例：创建并启动节点
auto node = std::make_unique<BlockchainNode>();
node->initialize();
node->start();

// 节点将持续运行，按Ctrl+C停止
```

## 核心模块说明

### 1. CDS 调度器

CDS (Chain-oriented Deterministic Scheduling) 是 CHASE 的核心创新：

```cpp
// 生成调度计划
auto cds = std::make_shared<CDS>(numShards);
auto plan = cds->generatePlan(block, kdg);

// 调度计划包含：
// - CFZ chains: 无冲突区依赖链（可无锁并行）
// - CZ chains: 冲突区依赖链（需验证执行）
// - Shard assignment: 分片分配方案
```

### 2. KDG 并行构建

KDG (Key-level Dependency Graph) 以存储键为顶点构建依赖图：

```cpp
KDG kdg;
for (auto& tx : transactions) {
    for (auto& key : tx.rwSet.reads) {
        kdg.addOperation(key, KeyOperation(tx.id, OpType::READ));
    }
    for (auto& [key, val] : tx.rwSet.writes) {
        kdg.addOperation(key, KeyOperation(tx.id, OpType::WRITE, val));
    }
}
```

### 3. DPP 预测协议

DPP (Deterministic Prediction Protocol) 通过感知机预测交易读写集：

```cpp
DPP dpp;
auto result = dpp.predict(tx, mvState, predictor);

// 高置信度 → FastPath（仅追踪SLOAD/SSTORE）
// 低置信度 → SimulateFull（完整EVM模拟）
```

### 4. BLP 流水线

BLP (Block-Level Pipelining) 实现四阶段并行：

```
时间线：
T0: Block_h   (Ordering)
T1: Block_h   (Scheduling)  | Block_h+1 (Ordering)
T2: Block_h   (Execution)   | Block_h+1 (Scheduling) | Block_h+2 (Ordering)
T3: Block_h   (Commit)      | Block_h+1 (Execution)  | Block_h+2 (Scheduling) ...
```

AIMD窗口控制自动调节流水线深度以避免积压。

### 5. 两层状态缓存

- **L1 (Uncommitted)**: 内存索引表，存储未提交的DeltaPage，容量256MB
- **L2 (Committed)**: LRU缓存，存储已提交状态，容量可配置

后续区块可直接读取前序区块的L1状态，无需等待提交。

## 性能指标

目标性能（单机32线程）：

- SmallBank (Zipfian=0): ≥ 100 kTPS
- SmallBank (Zipfian=0.5): ≥ 80 kTPS
- SmallBank (Zipfian=1.0): ≥ 60 kTPS
- 峰值TPS: 170.6 kTPS

## TODO 清单

### 核心功能
- [ ] 集成完整的 Narwhal/Tusk 共识实现
- [ ] 集成 evmone EVM 执行引擎
- [ ] 集成 libsecp256k1 签名验证
- [ ] 集成 RocksDB 持久化存储
- [ ] 实现完整的 DPP FastPath（EVM字节码分析）
- [ ] 实现 SV-branch 栈追踪
- [ ] 实现 Merkle Patricia Trie 状态根计算

### 优化
- [ ] KDG 并行构建的负载均衡
- [ ] CDS 分片分配的局部优化迭代
- [ ] BLP AIMD 窗口的自适应阈值调整
- [ ] L1/L2 缓存的异步刷写优化

### 测试
- [ ] 单元测试（KDG, CDS, DPP, StateCache）
- [ ] 集成测试（端到端多节点）
- [ ] 性能基准测试（SmallBank, CPUHeavy）
- [ ] 拜占庭容错测试

## 参考论文

CHASE: A Permissioned Blockchain System with Pipelined Execution and Chain-Oriented Deterministic Scheduling

## 许可证

MIT License
