# CHASE 区块链系统需求文档
> 面向 AI 编程的完整系统实现规范
> 版本：v1.0 | 基准论文：CHASE: A Permissioned Blockchain System with Pipelined Execution and Chain-Oriented Deterministic Scheduling

---

## 目录

- [CHASE 区块链系统需求文档](#chase-区块链系统需求文档)
  - [目录](#目录)
  - [1. 项目概述](#1-项目概述)
    - [1.1 背景](#11-背景)
    - [1.2 技术选型](#12-技术选型)
    - [1.3 系统模型](#13-系统模型)
  - [2. 系统架构总览](#2-系统架构总览)
    - [2.1 执行架构（EV变体）](#21-执行架构ev变体)
  - [3. 目录结构规范](#3-目录结构规范)
  - [4. 模块详细需求](#4-模块详细需求)
    - [4.1 网络与P2P模块](#41-网络与p2p模块)
    - [4.2 共识模块（Tusk BFT）](#42-共识模块tusk-bft)
    - [4.3 交易池模块](#43-交易池模块)
    - [4.4 KDG构建与DPP模块](#44-kdg构建与dpp模块)
      - [KDG（Key-level Dependency Graph，键级依赖图）](#kdgkey-level-dependency-graph键级依赖图)
      - [DPP（Deterministic Prediction Protocol，确定性预测协议）](#dppdeterministic-prediction-protocol确定性预测协议)
    - [4.5 CDS调度模块](#45-cds调度模块)
      - [4.5.1 调度计划结构](#451-调度计划结构)
      - [4.5.2 CDS主流程](#452-cds主流程)
    - [4.6 BLP流水线控制模块](#46-blp流水线控制模块)
    - [4.7 执行模块（EVM）](#47-执行模块evm)
    - [4.8 状态管理与两层缓存模块](#48-状态管理与两层缓存模块)
    - [4.9 提交模块](#49-提交模块)
    - [4.10 区块管理模块](#410-区块管理模块)
    - [4.11 密码学模块](#411-密码学模块)
    - [4.12 存储模块（RocksDB）](#412-存储模块rocksdb)
    - [4.13 客户端与RPC模块](#413-客户端与rpc模块)
  - [5. 核心数据结构](#5-核心数据结构)
    - [5.1 交易（Transaction）](#51-交易transaction)
    - [5.2 存储键（StorageKey）](#52-存储键storagekey)
    - [5.3 调度簇（SchedulingCluster）](#53-调度簇schedulingcluster)
  - [6. 核心算法规范](#6-核心算法规范)
    - [6.1 DPP：确定性预测协议](#61-dpp确定性预测协议)
    - [6.2 KDG并行构建](#62-kdg并行构建)
    - [6.3 CDS依赖链提取](#63-cds依赖链提取)
    - [6.4 CDS分片与调度生成](#64-cds分片与调度生成)
    - [6.5 CDS并发执行](#65-cds并发执行)
    - [6.6 BLP流水线控制](#66-blp流水线控制)
  - [7. 模块间接口规范](#7-模块间接口规范)
    - [7.1 数据流接口](#71-数据流接口)
    - [7.2 关键接口契约](#72-关键接口契约)
  - [8. 与参考仓库的对应关系](#8-与参考仓库的对应关系)
    - [8.1 optme仓库（Dong-Hyeon-Yu/optme）](#81-optme仓库dong-hyeon-yuoptme)
    - [8.2 FISCO-BCOS仓库](#82-fisco-bcos仓库)
    - [8.3 narwhal仓库（facebookresearch/narwhal）](#83-narwhal仓库facebookresearchnarwhal)
  - [9. 构建与依赖配置](#9-构建与依赖配置)
    - [9.1 CMakeLists.txt 主配置](#91-cmakeliststxt-主配置)
    - [9.2 关键依赖版本](#92-关键依赖版本)
    - [9.3 构建脚本示例](#93-构建脚本示例)
  - [10. 测试规范](#10-测试规范)
    - [10.1 单元测试](#101-单元测试)
    - [10.2 集成测试](#102-集成测试)
    - [10.3 性能基准测试](#103-性能基准测试)
  - [11. 待完成项 Checklist](#11-待完成项-checklist)
    - [核心模块](#核心模块)
    - [DPP（最重要：KDG的基础）](#dpp最重要kdg的基础)
    - [KDG并行构建](#kdg并行构建)
    - [CDS调度](#cds调度)
    - [CDS执行](#cds执行)
    - [BLP流水线](#blp流水线)
    - [两层缓存](#两层缓存)
    - [基础设施](#基础设施)
    - [测试与基准](#测试与基准)
  - [附录：关键设计决策与注意事项](#附录关键设计决策与注意事项)
    - [A1. 关于DPP的确定性保证](#a1-关于dpp的确定性保证)
    - [A2. 关于CZ重执行的频率控制](#a2-关于cz重执行的频率控制)
    - [A3. 关于BLP的内存管理](#a3-关于blp的内存管理)
    - [A4. 关于与optme的最大差异](#a4-关于与optme的最大差异)
    - [A5. 关于C++与Rust的互操作](#a5-关于c与rust的互操作)

---

## 1. 项目概述

### 1.1 背景

CHASE（Chain-oriented, High-performance, And Scalable Execution）是一个面向许可区块链的高性能确定性执行系统。其核心创新点为：

- **BLP（Block-Level Pipelining）**：区块级流水线，解耦共识/调度/执行/提交四个阶段，实现跨区块并行，消除串行生命周期造成的资源闲置。
- **CDS（Chain-oriented Deterministic Scheduling）**：链导向确定性调度，以"依赖链"作为最小调度单元，通过KDG提取并发结构，实现无中止的确定性并行执行。
- **DPP（Deterministic Prediction Protocol）**：确定性预测协议，通过轻量级栈追踪与感知机分支预测，以远低于全量EVM模拟的开销完成读写集提取，用于并行构建KDG。

### 1.2 技术选型

| 组件 | 技术选型 | 参考来源 |
|------|----------|---------|
| 编程语言 | C++17/20 | FISCO-BCOS |
| 共识算法 | Tusk BFT (DAG-based) | facebookresearch/narwhal |
| EVM实现 | EVMone | ethereum/evmone |
| 密码学 | libsecp256k1 | bitcoin-core/secp256k1 |
| 持久化存储 | RocksDB | facebook/rocksdb |
| 网络层 | asio / libp2p风格 | FISCO-BCOS/gateway |
| 构建系统 | CMake 3.18+ | FISCO-BCOS |
| 序列化 | Protocol Buffers | narwhal |
| 状态树 | Merkle Patricia Trie | FISCO-BCOS |

### 1.3 系统模型

- 许可网络，N = 3f+1 个节点，容忍 f 个拜占庭节点
- EV（Execute-Verify）架构变体：共识确定交易集合全序，执行层按确定性规则重排块内交易顺序
- 正确性目标：**确定性序列化**——所有诚实副本对相同输入产生完全一致的最终状态

---

## 2. 系统架构总览

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
│   Narwhal Mempool  +  Tusk DAG-based BFT                │
│   Output: ordered Block Bh^p (proposed block)           │
└────────────────────────┬────────────────────────────────┘
                         │ BLP 流水线并行推进
┌────────────────────────▼────────────────────────────────┐
│         Stage 2: Scheduling (CDS)                       │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────┐ │
│  │  DPP模拟执行  │→ │  KDG并行构建 │→ │  依赖链提取    │ │
│  │  读写集提取   │  │  (键级依赖图) │  │  CFZ + CZ     │ │
│  └──────────────┘  └──────────────┘  └───────┬───────┘ │
│                                               │分片映射  │
│                                       ┌───────▼───────┐ │
│                                       │ 调度计划 Π     │ │
│                                       └───────────────┘ │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────┐
│         Stage 3: Execution                              │
│  Shard 1  |  Shard 2  |  ...  |  Shard k               │
│  CFZ(无锁并行) → Barrier → CZ(冲突区执行+重执行检查)      │
│  状态读取: L1 (未提交) → L2 (已提交) → RocksDB           │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────┐
│         Stage 4: Commit                                 │
│  验证Bh^e结果 → 持久化DeltaPage → 输出Bh^c              │
└─────────────────────────────────────────────────────────┘

BLP: 以上四个阶段对不同高度的区块并行推进
     Bh: Ordering  |  Bh-1: Scheduling  |  Bh-2: Execution  |  Bh-3: Commit
```

### 2.1 执行架构（EV变体）

采用 Execute-Verify 架构：
- 调度器（Scheduler）作为主节点，负责模拟执行、KDG构建、调度计划生成
- 执行器（Executor × k）作为从节点，根据调度计划并发执行
- Verify阶段仅检查副本间状态一致性（对比状态哈希），无需重执行

---

## 3. 目录结构规范

参考FISCO-BCOS的模块化编程思想，目录组织如下：

```
CHASE/
├── CMakeLists.txt
├── cmake/
│   ├── Options.cmake
│   ├── Dependencies.cmake          # 管理evmone, secp256k1, rocksdb等
│   └── Hunter.cmake
├── framework/                 # 核心接口与数据结构（参考FISCO-BCOS风格）
│   ├── interfaces/
│   │   ├── consensus/
│   │   │   └── ConsensusInterface.h
│   │   ├── executor/
│   │   │   └── ExecutorInterface.h
│   │   ├── scheduler/
│   │   │   └── SchedulerInterface.h
│   │   └── storage/
│   │       └── StorageInterface.h
│   └── protocol/
│       ├── Block.h / Block.cpp
│       ├── Transaction.h / Transaction.cpp
│       └── TransactionReceipt.h
├── tusk/                      # Tusk BFT共识模块
│   ├── include/tusk/
│   │   ├── TuskEngine.h
│   │   ├── Narwhal.h               # DAG Mempool
│   │   ├── BullShark.h             # or Tusk DAG-BFT
│   │   └── TuskTypes.h
│   └── src/
│       ├── TuskEngine.cpp
│       ├── Narwhal.cpp
│       └── BullShark.cpp
├── txpool/                    # 交易池模块
│   ├── include/txpool/
│   │   └── TxPool.h
│   └── src/
│       └── TxPool.cpp
├── scheduler/                 # 调度模块（CDS核心）
│   ├── include/scheduler/
│   │   ├── CDS.h                   # Chain-oriented Deterministic Scheduling
│   │   ├── KDG.h                   # Key-level Dependency Graph
│   │   ├── DPP.h                   # Deterministic Prediction Protocol
│   │   ├── DependencyChain.h       # 依赖链数据结构
│   │   ├── SchedulePlan.h          # 调度计划 Π
│   │   └── ShardMapper.h           # 分片映射
│   └── src/
│       ├── CDS.cpp
│       ├── KDG.cpp
│       ├── DPP.cpp
│       ├── DependencyChain.cpp
│       └── ShardMapper.cpp
├── executor/                  # 执行模块
│   ├── include/executor/
│   │   ├── Executor.h              # 单个执行分片
│   │   ├── ExecutorManager.h       # 多执行分片管理
│   │   └── EVMWrapper.h            # EVMone封装
│   └── src/
│       ├── Executor.cpp
│       ├── ExecutorManager.cpp
│       └── EVMWrapper.cpp
├── pipeline/                  # BLP流水线控制模块
│   ├── include/pipeline/
│   │   ├── BlockPipeline.h         # 流水线主控制器
│   │   ├── PipelineWindow.h        # AIMD滑动窗口
│   │   └── StageQueue.h            # 阶段间队列
│   └── src/
│       ├── BlockPipeline.cpp
│       ├── PipelineWindow.cpp
│       └── StageQueue.cpp
├── state/                     # 状态管理（两层缓存）
│   ├── include/state/
│   │   ├── StateCache.h            # L1+L2缓存主接口
│   │   ├── L1UncommittedCache.h    # 未提交状态（DeltaPage）
│   │   ├── L2CommittedCache.h      # 已提交状态（LRU缓存）
│   │   ├── DeltaPage.h             # 状态变更页
│   │   └── MemIndexTable.h         # 内存索引表
│   └── src/
│       ├── StateCache.cpp
│       ├── L1UncommittedCache.cpp
│       ├── L2CommittedCache.cpp
│       └── DeltaPage.cpp
├── storage/                   # 持久化存储（RocksDB封装）
│   ├── include/storage/
│   │   ├── RocksDBStorage.h
│   │   └── MerklePatriciaTrie.h
│   └── src/
│       ├── RocksDBStorage.cpp
│       └── MerklePatriciaTrie.cpp
├── crypto/                    # 密码学模块（libsecp256k1封装）
│   ├── include/crypto/
│   │   ├── Secp256k1.h
│   │   └── Hash.h
│   └── src/
│       ├── Secp256k1.cpp
│       └── Hash.cpp
├── network/                   # 网络模块
│   ├── include/network/
│   │   ├── P2PService.h
│   │   └── MessageCodec.h
│   └── src/
│       ├── P2PService.cpp
│       └── MessageCodec.cpp
├── rpc/                       # 客户端RPC
│   ├── include/rpc/
│   │   └── JsonRpcService.h
│   └── src/
│       └── JsonRpcService.cpp
├── benchmark/                 # 性能测试（SmallBank, CPUHeavy）
│   ├── SmallBankBenchmark.cpp
│   └── CPUHeavyBenchmark.cpp
├── node/                      # 节点启动入口
│   └── main.cpp
├── tests/                          # 单元测试
│   ├── test_kdg.cpp
│   ├── test_cds.cpp
│   ├── test_dpp.cpp
│   ├── test_blp.cpp
│   └── test_state.cpp
└── proto/                          # Protocol Buffers定义
    ├── block.proto
    ├── transaction.proto
    └── consensus.proto
```

---

## 4. 模块详细需求

### 4.1 网络与P2P模块

**模块路径**：`network/`

**功能**：
- 基于全网状P2P拓扑，支持N=3f+1节点间的消息广播与点对点通信
- 提供消息类型路由：共识消息、交易广播、区块同步
- 支持身份认证（基于公钥基础设施PKI）

**接口**：
```cpp
class P2PService {
public:
    // 广播消息到所有已知节点
    void broadcast(MessageType type, const bytes& payload);
    // 向特定节点发送消息
    void sendTo(const NodeID& nodeId, MessageType type, const bytes& payload);
    // 注册消息处理器
    void registerHandler(MessageType type, MessageHandler handler);
    // 节点发现与连接管理
    void connect(const NodeEndpoint& endpoint);
};
```

**实现参考**：FISCO-BCOS `gateway` 模块的P2P连接管理逻辑。narwhal中的`network`模块用于消息传递。

---

### 4.2 共识模块（Tusk BFT）

**模块路径**：`tusk/`

**功能**：实现基于DAG结构的Tusk BFT共识，负责确立区块间的全序。共识层**仅确立交易集合的全序**，不锁定区块内部交易执行顺序。

**关键概念**：
- **Narwhal**：DAG-based Mempool，负责交易的可靠广播与DAG结构维护。每个节点在每轮（round）向其他节点广播包含若干交易的"证书"（Certificate）。
- **Tusk**（或BullShark）：基于Narwhal DAG的BFT共识，通过在DAG中选取"锚点"（Anchor）来决定提交顺序。

**核心流程**：
1. 客户端提交交易 → TxPool
2. 节点从TxPool打包交易，创建"Batch"，广播给其他节点
3. 收集2f+1个确认后，形成"Certificate"，更新本地DAG
4. Tusk协议周期性地在DAG中选取锚点，确定一批Certificate（即一个Block）的全序
5. 输出：proposed block `Bh^p`，包含有序的交易集合

**接口**：
```cpp
class TuskEngine {
public:
    // 启动共识引擎
    void start();
    // 提交要打包的交易（由TxPool调用）
    void submitTransaction(Transaction::Ptr tx);
    // 注册新区块回调（供BLP流水线使用）
    void onNewBlock(std::function<void(Block::Ptr)> callback);
    // 当前节点ID
    NodeID nodeId() const;
};
```

**实现参考**：
- `facebookresearch/narwhal` 仓库的 `consensus/` 和 `worker/` 目录
- `Dong-Hyeon-Yu/optme` 中 `narwhal/` 子目录（该仓库直接集成了narwhal作为共识层）
- 关键参数：`batch_size`（每批交易数），`round_duration`（每轮时长），`committee_size`（委员会大小）

**注意**：narwhal原始实现为Rust。若CHASE为C++项目，需要：
- 方案A：通过FFI或gRPC将Rust narwhal作为独立进程运行，C++通过RPC与其通信
- 方案B：参考narwhal的协议逻辑，用C++重新实现简化版Tusk
- **推荐方案A**：保持与narwhal实现的协议兼容性，通过protobuf消息接口解耦

---

### 4.3 交易池模块

**模块路径**：`txpool/`

**功能**：
- 接收客户端提交的交易，验证签名（调用密码学模块）
- 维护待处理交易队列（优先级队列，按Gas Price排序）
- 向共识层提供交易批次（batch）
- 去重：过滤已提交或已在队列中的交易

**接口**：
```cpp
class TxPool {
public:
    // 提交新交易（返回交易哈希或错误）
    Error submit(Transaction::Ptr tx);
    // 供共识层调用，获取打包批次
    std::vector<Transaction::Ptr> fetchBatch(size_t maxCount, uint64_t maxGas);
    // 共识确认后，从pool中移除已提交交易
    void remove(const std::vector<TxHash>& txHashes);
    // 查询交易状态
    TxStatus query(const TxHash& hash);
};
```

---

### 4.4 KDG构建与DPP模块

**模块路径**：`scheduler/`，文件 `KDG.h/cpp`，`DPP.h/cpp`

这是CDS的基础，也是性能优化的关键所在。

#### KDG（Key-level Dependency Graph，键级依赖图）

**数据模型**：
- 以存储键（`AccountAddress + SlotIndex`）为顶点
- 每个键维护一个有序的**操作序列**（Operation Sequence），按交易ID升序排列
- 每个操作记录：`{TxID, OpType (READ/WRITE), Value}`
- 依赖关系通过键内操作序列的相对顺序**隐式推导**，无需显式构建跨键边
- 三类依赖：WAR（写后读）、WAW（写后写）、RAW（读后写，唯一硬约束）

**KDG核心结构**：
```cpp
struct KeyOperation {
    TxID    txId;
    OpType  opType;     // READ or WRITE
    bytes   value;      // 写操作时有效
};

class KDG {
public:
    // key -> 操作序列（按txId升序）
    std::unordered_map<StorageKey, std::vector<KeyOperation>> keySequences;
    
    // 向指定key添加操作
    void addOperation(const StorageKey& key, const KeyOperation& op);
    
    // 推导依赖关系（用于依赖链提取）
    DependencyType getDependency(TxID u, TxID v, const StorageKey& key) const;
    
    // 获取某笔交易的所有直接前驱
    std::vector<TxID> getPredecessors(TxID txId) const;
};
```

**并行构建策略**（参考optme中 `crates/sslab-execution/` 的并行KDG构建）：
1. 对每笔交易，通过DPP获得读写集 `{(key, opType)}`
2. 将键按哈希分片，每个工作线程负责一批键的操作序列更新
3. 线程间无交叉：键的分配确保同一键只由同一线程处理，实现无锁并行
4. 动态负载均衡：按键的访问频度（热点键）动态分配，避免线程负载倾斜
5. 各线程完成后合并结果（merge各线程的局部KDG）

**实现参考**：
- optme仓库 `crates/sslab-execution/optme/src/` 中的 `nezha_graph.rs` 或类似文件（并行KDG构建逻辑）
- 注意optme使用Rust，需翻译为C++的等价实现

#### DPP（Deterministic Prediction Protocol，确定性预测协议）

**目标**：以远低于全量EVM模拟（Full EVM Simulation）的开销，获取每笔交易的读写集，同时保持多副本间的确定性一致。

**架构设计**（双层预测模型）：

**层1：感知机分支预测器（Perceptron Predictor）**

对SV-branches（State-Variable-dependent branches，依赖状态变量的条件跳转）进行预测：

```cpp
struct BranchPredictor {
    // 权重向量，零初始化，区块粒度原子提交
    std::vector<int32_t> weights;   // w_i: 历史跳转方向
    std::vector<int32_t> pweights;  // w_j': 块内前序预测结果
    int32_t bias;                   // w_0
    
    // 预测函数: f = w0 + sum(wi * hi) + sum(wj' * pj)
    // hi: 该分支的历史跳转方向（L次历史）
    // pj: 块内前序K个预测结果
    float predict(const BranchHistory& history, 
                  const InBlockPredictions& prevPredictions) const;
    
    // 在线更新（感知机更新规则）
    void update(bool actualOutcome, float prediction);
};
```

关键约束：所有节点权重向量**统一零初始化**，按规范顺序在线更新，于**区块粒度原子提交**，保证各副本模型状态一致。

**层2：多版本状态层（Multi-version State Layer）**

- 维护确定性的多版本状态缓存，按模拟的交易执行顺序组织
- 对交易Ti，只允许读取前序交易{T1,...,Ti-1}产生的临时状态版本
- 用途：(a) 感知机置信度不足时作为精确回退；(b) 分支历史不足时直接预测

```cpp
class MultiVersionState {
public:
    // 获取交易Ti执行时，key的可见状态（来自T1...Ti-1）
    bytes getState(TxID txId, const StorageKey& key) const;
    
    // 记录交易Ti对key的写入（仅临时，用于后续交易预测）
    void setTemp(TxID txId, const StorageKey& key, const bytes& value);
};
```

**动态执行路径选择**：

```
给定预测值 f，共识预设阈值 θ（自适应调整）：

if |f| >= θ:
    FastPath(T)     // 高置信度：仅追踪SLOAD/SSTORE，跳过常规指令循环
else:
    SimulateFull(T) // 低置信度：基于多版本状态层执行精确模拟
```

**FastPath实现要点**：
- 开启检查点机制（Checkpoint），跳过非存储操作的指令循环
- 仅追踪存储原语：`SLOAD`（记录READ）和`SSTORE`（记录WRITE）
- 检测到未声明的状态访问或分支偏离时，立即触发一致性回退（Fallback to SimulateFull）

**SV-branch识别（栈追踪）**：
- 监控`SLOAD`指令读取的状态变量，对其进行"污点标记"
- 追踪标记值在栈操作（PUSH, POP, DUP, SWAP等）中的传播
- 当执行`JUMPI`时，若条件操作数带有标记，则该分支被标记为SV-branch

**最坏情况保证**：
- 在极端非线性场景（预测准确率趋近0），额外开销 ≤ `Cost_fast + Cost_full + δ_flush`
- 系统仅退化为确定性精确模拟，不引入额外重试或级联开销

---

### 4.5 CDS调度模块

**模块路径**：`scheduler/`，文件 `CDS.h/cpp`，`DependencyChain.h/cpp`，`ShardMapper.h/cpp`

这是CHASE最核心的创新模块。

#### 4.5.1 调度计划结构

```cpp
struct SchedulePlan {
    // 无冲突区（Conflict-Free Zone）依赖链集合
    std::vector<DependencyChain> cfzChains;     // L_free
    
    // 冲突区（Conflict Zone）依赖链集合
    std::vector<DependencyChain> czChains;       // L_conflict
    
    // 分片分配：shard_id -> [chain_ids]
    std::vector<std::vector<ChainID>> shardAssignment;
    
    // 块高度
    BlockHeight blockHeight;
};

struct DependencyChain {
    ChainID     id;
    ChainType   type;           // CFZ or CZ
    std::vector<TxID> txOrder;  // 链内交易执行顺序（严格有序）
    uint64_t    load;           // Gas估算负载
    
    // 仅CZ链使用：逻辑绑定的CFZ链集合（用于共置约束）
    std::vector<ChainID> boundCFZChains;    // D_bind(Lc)
    
    // 链的写集摘要（用于快速冲突判断）
    std::unordered_set<StorageKey> writeSetSummary;
};
```

#### 4.5.2 CDS主流程

```cpp
class CDS {
public:
    // 输入：proposed block（已有交易全序），输出：调度计划Π
    SchedulePlan generatePlan(const Block::Ptr& block, 
                               const KDG& kdg,
                               int numShards);

private:
    // Step 1: 交易排序（基于KDG拓扑）
    std::vector<TxID> topologicalSort(const KDG& kdg);
    
    // Step 2: 冲突处理（WAW消解，标记abort交易）
    void resolveConflicts(const KDG& kdg, 
                          std::unordered_set<TxID>& abortedTxs);
    
    // Step 3: 依赖链提取
    std::pair<std::vector<DependencyChain>, std::vector<DependencyChain>>
    extractChains(const KDG& kdg, 
                  const std::vector<TxID>& sortedTxs,
                  const std::unordered_set<TxID>& abortedTxs);
    
    // Step 4: 分片映射（装箱问题 + 负载均衡）
    std::vector<std::vector<ChainID>> 
    assignShards(const std::vector<DependencyChain>& allChains, int k);
};
```

与optme的对应关系：
- optme的Epoch 1（无冲突轮次）→ CHASE的CFZ（无冲突区）
- optme的Epoch 2/3/4+（后续冲突轮次）→ CHASE的CZ（冲突区，统一合并）
- optme的序列（sequence）→ CHASE依赖链中存在依赖的交易被"串"成链
- optme的Inter-epoch Reordering算法 → CHASE冲突依赖链提取算法（见6.3节）

---

### 4.6 BLP流水线控制模块

**模块路径**：`pipeline/`

**功能**：将区块处理的四个阶段解耦为可并行推进的流水线，通过AIMD（加性增乘性减）机制自适应控制流水线窗口大小。

**三个流水线阶段**（合并调度与执行为一个重资源阶段）：
- P1：排序（Ordering）
- P2：调度+执行（Scheduling + Execution，CPU密集型）
- P3：提交（Commit，I/O密集型）

**积压权重定义**：
- W2（执行积压）：P2队列中所有区块的Gas总量之和
- W3（提交积压）：P3队列中待落盘的状态增量总字节数

**AIMD窗口控制**：
```
ζi(t+1) =
  min(ζi(t) * 2, ζmax)     if W_{i+1} = 0        （指数增长）
  ζi(t) + 1                if 0 < W_{i+1} < λ     （线性增长）
  max(⌊ζi(t)/2⌋, 1)        if W_{i+1} >= λ        （乘性减少）
```

**两级内存管理**集成在BLP中（见4.8节）：
- L1（未提交状态）：BLP流水线中，允许区块Bh+1读取Bh的未提交状态（L1命中即返回）
- L2（已提交状态）：L1未命中时查找已持久化状态

**接口**：
```cpp
class BlockPipeline {
public:
    // 启动流水线
    void start(int numShards);
    
    // 注册各阶段处理函数
    void setOrderingStage(std::function<void(Block::Ptr)> handler);
    void setSchedulingStage(std::function<SchedulePlan(Block::Ptr)> handler);
    void setExecutionStage(std::function<StateSnapshot(Block::Ptr, SchedulePlan)> handler);
    void setCommitStage(std::function<void(Block::Ptr, StateSnapshot)> handler);
    
    // 推入新proposed区块（由共识层调用）
    void pushBlock(Block::Ptr block);
    
    // 查询流水线状态
    PipelineStatus status() const;
};
```

---

### 4.7 执行模块（EVM）

**模块路径**：`executor/`

**功能**：
- 封装EVMone，执行Solidity智能合约
- 记录每笔交易的实际读写键（执行期）
- 支持CFZ（无锁并行）和CZ（分片内局部同步）两种执行模式

**EVMone封装要求**：
```cpp
class EVMWrapper {
public:
    // 执行单笔交易（完整执行，用于最终状态更新）
    ExecResult execute(const Transaction& tx, 
                       StateAccessor& state);
    
    // FastPath模拟执行（仅追踪SLOAD/SSTORE，用于DPP）
    SimResult simulateFast(const Transaction& tx,
                           const BranchPredictions& predictions);
    
    // 全量模拟执行（用于DPP低置信度情况）
    SimResult simulateFull(const Transaction& tx,
                           const MultiVersionState& mvState);
    
    // 获取最后一次执行的读写集
    ReadWriteSet getLastRWSet() const;
};
```

**Executor（单个执行分片）**：
```cpp
class Executor {
public:
    ShardID shardId;
    
    // 按调度计划执行分配给本分片的依赖链
    void execute(const SchedulePlan& plan,
                 StateCache& stateCache);
    
    // CFZ执行：无锁并行，直接写入本地缓冲区
    void executeCFZ(const std::vector<DependencyChain>& chains,
                    StateCache& stateCache);
    
    // CZ执行：记录实际读写键，检查乐观假设
    void executeCZ(const std::vector<DependencyChain>& chains,
                   StateCache& stateCache,
                   std::vector<TxID>& invalidTxs);
};
```

---

### 4.8 状态管理与两层缓存模块

**模块路径**：`state/`

**核心数据结构**：
```cpp
struct DeltaPage {
    BlockHeight height;                              // 区块高度
    std::array<KVEntry, 128> entries;                // 最多128条记录
    int count;
    bool frozen;                                     // 是否已冻结（只读）
};

class MemIndexTable {
    // 内存索引：key -> DeltaPage*
    std::unordered_map<StorageKey, DeltaPage*> index;
    size_t totalBytes;
    static constexpr size_t CAPACITY_THRESHOLD = 256 * 1024 * 1024; // 256MB
    
    // 容量达到阈值时冻结，异步刷写到L2
    void maybeFreeze();
};
```

**写入流程**：
1. 执行阶段写入临时缓冲区（Buffer h+1）
2. 整个区块执行完成后，缓冲区编码为DeltaPage
3. DeltaPage发送到L1的MemIndexTable，设为只读
4. MemIndexTable达到256MB阈值时冻结，后台异步刷写到L2缓存及RocksDB
5. 刷写完成后从L1删除，回收内存

**读取流程**（查找顺序）：
1. 先查L1 MemIndexTable（内存拷贝）
2. L1未命中 → 查L2 LRU缓存（已提交状态）
3. L2未命中 → 查RocksDB持久化存储

**关键约束**：
- L1只存储写状态（忽略读状态），遵循写时复制（CoW）机制
- 每个DeltaPage限制128条记录（接近文件系统4KB块大小）
- 后续区块Bh+1可直接读取前序区块Bh的L1状态，**无需等待Bh提交**

---

### 4.9 提交模块

**模块路径**：集成在 `executor/` 或单独 `commit/`

**功能**：
- 验证Bh^e（执行结果）：对比所有诚实副本的状态哈希
- 持久化状态变更：触发L1→L2→RocksDB的刷盘流程
- 更新Merkle Patricia Trie，生成新的状态根
- 输出最终提交块Bh^c，广播给其他节点

**提交算法**（参考论文Algorithm 3，并发执行中的提交逻辑）：
```
CFZ交易：按链内顺序并发提交（无需运行时验证）
CZ交易：
  1. 记录实际读写键 rw_keys
  2. 检查 tx.prev_rw_keys == tx.rw_keys
     - 相等：valid_txs，按链内顺序批量提交
     - 不等：invalid_txs
  3. 对invalid_txs，检查 W_shard.disjoint(tx.rw_keys)
     - 不相交：可并行提交，移入valid_txs
     - 相交：必须重新执行
  4. commit_parallel(valid_txs)
  5. execute_and_commit(invalid_txs)  // 重执行
```

---

### 4.10 区块管理模块

**模块路径**：`framework/protocol/`

**区块生命周期状态**：
```cpp
enum class BlockState {
    PROPOSED,   // Bh^p: 共识提出，含交易列表
    ORDERED,    // 交易全序确定
    SCHEDULED,  // 调度计划Π生成完毕
    EXECUTED,   // Bh^e: 执行完成，含状态变更
    COMMITTED,  // Bh^c: 提交完成，持久化
};

struct Block {
    BlockHeight         height;
    Hash                parentHash;
    std::vector<TxHash> txHashes;
    BlockState          state;
    
    // 不同阶段附加的数据
    SchedulePlan        plan;           // SCHEDULED后有效
    StateSnapshot       stateSnapshot; // EXECUTED后有效
    Hash                stateRoot;     // COMMITTED后有效
};
```

---

### 4.11 密码学模块

**模块路径**：`crypto/`

基于libsecp256k1封装，提供：
- **签名验证**：`bool verifySignature(const TxHash& hash, const Signature& sig, const PublicKey& pubkey)`
- **哈希计算**：Keccak-256（与以太坊兼容），SHA-256
- **地址推导**：从公钥推导账户地址

**实现参考**：FISCO-BCOS `crypto` 模块，其对libsecp256k1的封装已经过生产验证。

---

### 4.12 存储模块（RocksDB）

**模块路径**：`storage/`

**Column Families设计**：
```
CF_STATE:   account_address + slot -> value  (账户状态)
CF_BLOCKS:  block_height -> serialized_block
CF_TXINDEX: tx_hash -> block_height + tx_index
CF_META:    "latest_committed_height" -> height
```

**写入策略**：
- 使用RocksDB的WriteBatch进行批量写入（减少写放大）
- BlockCache：512MB（L2已提交状态的磁盘缓存层）
- WriteBufferSize：64MB（memtable大小，配合BLP的提交频率）

**实现参考**：
- FISCO-BCOS `storage/storage/RocksDBStorage.h`
- optme `crates/sslab-execution/` 中对RocksDB的使用

---

### 4.13 客户端与RPC模块

**模块路径**：`rpc/`

支持JSON-RPC接口（兼容以太坊风格）：
- `eth_sendRawTransaction`：提交签名交易
- `eth_getTransactionReceipt`：查询交易回执
- `eth_getBlockByNumber`：查询区块信息
- `eth_call`：只读调用（不上链）
- `chase_getNodeStatus`：查询节点状态（流水线状态、TPS等）

---

## 5. 核心数据结构

### 5.1 交易（Transaction）

```cpp
struct Transaction {
    using Ptr = std::shared_ptr<Transaction>;
    
    TxID        id;             // 共识层分配的单调递增ID（全局全序）
    Hash        hash;           // Keccak256(rlp(tx))
    Address     from;
    Address     to;
    uint64_t    nonce;
    uint64_t    gasPrice;
    uint64_t    gasLimit;
    bytes       data;           // 合约调用数据
    Signature   signature;
    
    // 调度阶段填充（DPP产出）
    ReadWriteSet    rwSet;      // 预测的读写集
    uint64_t        gasUsed;    // 模拟执行的Gas消耗（用于负载估算）
    
    // 执行阶段填充
    ReadWriteSet    actualRWSet; // 实际执行的读写集（用于CZ验证）
};

struct ReadWriteSet {
    std::unordered_set<StorageKey> reads;
    std::unordered_map<StorageKey, bytes> writes;
};
```

### 5.2 存储键（StorageKey）

```cpp
struct StorageKey {
    Address     account;
    bytes32     slot;
    
    bool operator==(const StorageKey& o) const;
    size_t hash() const;        // 用于unordered_map
};
```

### 5.3 调度簇（SchedulingCluster）

```cpp
struct SchedulingCluster {
    ClusterID   id;
    std::vector<ChainID> chains;    // 包含的依赖链（CFZ + CZ）
    uint64_t    load;               // 负载 = sum(Load(Li))
    
    // 传递闭包合并结果，簇内链共置，簇间绝对解耦
};
```

---

## 6. 核心算法规范

### 6.1 DPP：确定性预测协议

```
输入：交易T，多版本状态缓存MVS，感知机分支预测器BranchPredictor
输出：T的读写集 RW(T)

DPP(T, MVS, BranchPredictor):
  1. 识别T中的所有SV-branches（通过栈追踪）
  2. 对每个SV-branch b:
     a. 查询BranchPredictor预测值 f = predict(history(b), prev_predictions)
     b. 调整阈值θ（基于历史预测置信度分布的自适应调整）
  3. 选择执行路径：
     if |f| >= θ for ALL SV-branches:
         RW(T) = FastPath(T, BranchPredictor)
     else:
         RW(T) = SimulateFull(T, MVS)
  4. 将T的写结果写入MVS（供后续交易使用）
  5. 更新BranchPredictor（在线感知机更新）
  6. 返回 RW(T)

FastPath(T, BranchPredictor):
  - 初始化检查点，跳过非存储指令循环
  - 遇到SLOAD：记录 reads.insert(key)，返回BranchPredictor预测值
  - 遇到SSTORE：记录 writes[key] = val
  - 遇到JUMPI：
    - 如果条件操作数被标记（SV-branch），使用预测方向跳转
    - 如果实际执行发现状态访问偏离预测：触发Fallback
  - 返回累积的读写集

SimulateFull(T, MVS):
  - 使用EVMone完整执行T
  - 对SLOAD使用MVS中Ti-1的可见状态
  - 精确记录所有SLOAD/SSTORE
  - 返回精确读写集
```

### 6.2 KDG并行构建

```
输入：区块Bh中的n笔交易（已有全序），各交易的读写集RW(Ti)
输出：KDG（键级依赖图）

ParallelBuildKDG(transactions, rwSets):
  1. 确定分片数M（工作线程数）
  2. 对所有出现的键计算哈希，按哈希值将键分配到M个桶
     （热点键特殊处理：独立分配给负载最低的线程）
  3. 并行执行（M个线程）:
     Thread i 处理桶 i 中的所有键:
       for each key k in bucket_i:
           for each tx Tj that accesses k (按TxID升序):
               keySequences[k].append({Tj.id, Tj.rwSet[k].type, val})
           按"读前写后"规则重排序列（read < write 对于相同TxID）
           隐式推导依赖边（无需显式存储）
  4. 合并所有线程的局部结果到全局KDG
  5. 识别RAW依赖：对键k的序列，若 write(Ti) -> read(Tj) 且 Ti有序在Tj之前
                    （即Ti需先于Tj执行），标记(Ti,Tj)为RAW依赖
     对WAW：Ti和Tj都写同一键，Ti.id < Tj.id，标记WAW依赖，Tj被标为abort候选
  6. 返回KDG

注意：RAW依赖是唯一可能破坏可串行化的硬性约束。
      WAR和WAW为良性依赖，可通过确定性排序消解。
```

### 6.3 CDS依赖链提取

**Step 1：交易排序（基于KDG拓扑）**
```
1. 对KDG中的键节点进行拓扑排序，确定各键的全局处理顺序
2. 同一键内部，为访问操作分配顺序索引：
   - 读操作始终先于写操作（相同键）
   - 存在RAW依赖的写操作约束在对应读操作之后
3. 为每笔交易导出唯一序列索引 seqIdx(T)
```

**Step 2：WAW冲突消解（决定哪些交易被abort）**
```
对同一键的多个写操作，按TxID（单调递增逻辑时间戳）排序：
  - TxID最小的写操作 = 执行赢家（胜出）
  - 其余写操作的交易 → 标记为 abort（隔离至CZ）
```

**Step 3a：无冲突依赖链提取（构建CFZ中的链集合 L_free）**

参考optme中序列（sequence）的组织方式，以及交易依赖关系将序列中的交易"串"成链：

```
输入：未被标记为abort的交易集合，按seqIdx排序
L_free = {}

for Tj in sorted(non-aborted txs, by seqIdx):
    pred(Tj) = {Ti | Ti直接依赖于Tj（KDG边 Ti→Tj）}
    D(Tj) = {Lf ∈ L_free | max_tx(Lf) ∈ pred(Tj)}
    
    case |D(Tj)| == 0:
        创建新链 Lf = [Tj]，加入L_free
    
    case |D(Tj)| == 1:
        将Tj追加到唯一前驱链 Lf 的末尾
    
    case |D(Tj)| >= 2:
        执行链合并：将所有前驱链的尾部统一指向Tj
        （Tj成为这些链的共同后继，链在Tj处合并）

特殊情况：若多笔交易共享同一链尾作为唯一前驱且彼此无依赖，
         允许它们作为该链尾的并行后继同时调度（并行分叉）
```

**Step 3b：冲突依赖链提取（构建CZ中的链集合 L_conflict）**

参考optme的Inter-epoch Reordering算法：

```
输入：被标记为abort的交易集合，按TxID升序处理
L_conflict = {}

for Tj in sorted(aborted txs, by TxID ascending):
    found = false
    for Lc in L_conflict (按链头TxID升序遍历):
        if RW(Tj) ∩ W_chain(Lc) ≠ ∅:   // 写集摘要快速判断
            Lc.append(Tj)
            W_chain(Lc) = W_chain(Lc) ∪ W(Tj)  // 更新写集摘要
            found = true
            break
    if not found:
        创建新链 Lc = [Tj]，W_chain(Lc) = W(Tj)
        加入L_conflict

// 建立冲突依赖链与CFZ链的逻辑绑定
for Lc in L_conflict:
    D_bind(Lc) = {Lf ∈ L_free | RW(Lc) ∩ W(Lf) ≠ ∅}
    Lc.boundCFZChains = D_bind(Lc)
```

### 6.4 CDS分片与调度生成

**Step 1：动态依赖传递闭包分析（构建调度簇）**
```
1. 将每条Lc及其D_bind(Lc)聚合为初始簇
2. 对存在间接数据耦合的簇递归合并（传递闭包）：
   if W(Cluster_i) ∩ RW(Cluster_j) ≠ ∅: merge(Cluster_i, Cluster_j)
3. 结果：互不相交的全局调度簇集合 C = {Cluster_1, ..., Cluster_m}
   Cluster负载 = sum(Load(Li)) for Li in Cluster
```

**Step 2：负载均衡装箱分配（LPT贪心 + 局部优化）**
```
if m < k:
    初始化k个分片，空分片置为∅

if m >= k:
    // LPT初始分配
    sort(Clusters, by load DESC)
    for i in [0, k): S_i = {Cluster_i}  // 前k个簇各占一个分片
    for remaining Cluster_j:
        S_min = argmin(load(Si))
        S_min.add(Cluster_j)

// 局部优化（迭代，最多K_max次）
repeat:
    Sp = argmax(load), Sq = argmin(load)
    for Ca in Sp:
        for Cb in Sq ∪ {∅}:  // ∅表示迁移操作
            D' = std_dev after swap(Ca, Cb)
            if D - D' > ε: 
                execute swap, update loads
                break

// 确定性保证：固定排序、字典序决策、迭代次数上限K_max
```

**Step 3：时间映射（链级异步推进）**
```
各分片内，工作线程以调度簇为最小单元异步拉取执行任务：
- 簇间无运行时协调（完成一个簇立即推进下一个）
- 无冲突链执行：无锁，写集仅写入本地缓冲区
- 冲突链执行：前驱写入已共置于同一分片，仅需分片内局部同步

执行顺序约束：CFZ先于CZ（命题保证，通过屏障Barrier实现）
```

### 6.5 CDS并发执行

完整实现论文中的Algorithm 3：

```cpp
void executeParallel(const SchedulePlan& plan, StateCache& state) {
    // 1. CFZ：并发提交，无需运行时验证
    parallel_for(plan.cfzChains, [&](const DependencyChain& chain) {
        for (TxID tx : chain.txOrder) {
            executeAndCommit(tx, state);  // 无锁，写入本地缓冲
        }
    });
    
    // Barrier：等待所有CFZ完成
    barrier.wait();
    
    // 2. CZ：执行并记录实际读写键
    parallel_for(plan.czChains, [&](const DependencyChain& chain) {
        std::vector<TxID> validTxs, invalidTxs;
        
        for (TxID tx : chain.txOrder) {
            executeAndRecord(tx, state);  // 记录实际rw_keys
            
            if (tx.prev_rw_keys == tx.actual_rw_keys) {
                validTxs.push_back(tx);
            } else {
                invalidTxs.push_back(tx);
            }
        }
        
        // 快速判定：检查无效交易是否与其他链写集不相交
        if (!invalidTxs.empty()) {
            auto W_shard = unionWriteSets(plan.czChains, excluding=chain);
            for (TxID tx : invalidTxs) {
                if (W_shard.disjoint(tx.actual_rw_keys)) {
                    validTxs.push_back(tx);
                    invalidTxs.remove(tx);
                }
            }
        }
        
        commitParallel(validTxs, state);
        executeAndCommit(invalidTxs, state);  // 重执行
    });
}
```

### 6.6 BLP流水线控制

```cpp
// 流水线主循环（简化版）
void BlockPipeline::run() {
    while (running) {
        // 从共识层接收新proposed block
        Block::Ptr block = consensusQueue.pop();
        
        // AIMD窗口检查：是否允许进入P2阶段
        while (windowFull(P2)) {
            updateWindow(P2, W2);  // 根据积压权重调整窗口
            std::this_thread::sleep_for(1ms);
        }
        
        // 异步提交到P2（调度+执行）
        threadPool.submit([block, this]() {
            auto plan = scheduler.generatePlan(block);
            auto snapshot = executor.execute(block, plan);
            
            // P3检查
            while (windowFull(P3)) {
                updateWindow(P3, W3);
                std::this_thread::sleep_for(1ms);
            }
            
            commitQueue.push({block, snapshot});
        });
    }
}

void BlockPipeline::updateWindow(int stage, uint64_t backpressure) {
    auto& w = windows[stage];
    if (backpressure == 0)
        w.size = std::min(w.size * 2, w.maxSize);
    else if (backpressure < w.threshold)
        w.size = std::min(w.size + 1, w.maxSize);
    else
        w.size = std::max(w.size / 2, 1ul);
}
```

---

## 7. 模块间接口规范

### 7.1 数据流接口

```
TuskEngine --[Block::Ptr]--> BlockPipeline
BlockPipeline --[Block::Ptr]--> CDS
CDS --[SchedulePlan]--> ExecutorManager
ExecutorManager --[StateSnapshot]--> CommitManager
CommitManager --[CommittedBlock]--> StateCache(L1→L2→RocksDB)
```

### 7.2 关键接口契约

**共识→流水线**：
- 输出：`Block::Ptr`，其中 `block->txHashes` 按全序排列，`block->state == PROPOSED`
- 保证：所有诚实副本对相同高度的区块有完全相同的txHashes顺序

**流水线→调度（CDS）**：
- 输入：`Block::Ptr`（PROPOSED状态）
- 输出：`SchedulePlan`，包含CFZ链集合、CZ链集合、分片分配
- 保证：调度计划对所有诚实节点完全确定性（给定相同输入必然相同输出）

**调度→执行**：
- 输入：`SchedulePlan`
- 执行器从StateCache读取状态，写入本地缓冲区
- 输出：各分片本地缓冲区的状态变更，最终合并为StateSnapshot

**执行→提交**：
- 输入：`Block::Ptr`（EXECUTED状态），`StateSnapshot`
- 输出：DeltaPage写入L1，状态根更新，`Block::Ptr`（COMMITTED状态）

---

## 8. 与参考仓库的对应关系

### 8.1 optme仓库（Dong-Hyeon-Yu/optme）

optme基于Sui/Narwhal代码库，核心执行逻辑在 `crates/sslab-execution/` 中。

| optme组件 | 对应路径 | CHASE对应模块 | 主要改动 |
|-----------|----------|--------------|---------|
| 并行KDG构建 | `crates/sslab-execution/optme/src/nezha_graph.rs` (推测) | `scheduler/KDG.cpp` | 增加DPP协议（FastPath + 感知机预测），C++重写 |
| Epoch 1 无冲突序列 | optme调度核心 | `scheduler/CDS.cpp` CFZ部分 | 将序列内存在依赖的交易显式串成依赖链（链合并逻辑） |
| Epoch 2+ 冲突轮次 | optme Inter-epoch Reordering | `scheduler/CDS.cpp` CZ部分 | 所有后续epoch统一合并为单一冲突区，按写集摘要归链 |
| Narwhal共识 | `narwhal/` 子目录 | `tusk/` | 通过gRPC/FFI调用，或C++重实现 |
| RocksDB状态存储 | `crates/sslab-execution/` | `storage/` | 增加两层缓存（L1/L2）设计 |

### 8.2 FISCO-BCOS仓库

| FISCO-BCOS模块 | 路径 | CHASE对应 | 使用方式 |
|----------------|------|-----------|---------|
| pbft | `pbft/` | **不使用**（替换为Tusk） | 仅参考流水线设计思想 |
| crypto | `crypto/` | `crypto/` | 直接复用libsecp256k1封装 |
| storage | `storage/` | `storage/` | 直接复用RocksDB封装 |
| framework协议 | `framework/` | `framework/` | 参考Block/Tx数据结构设计 |
| gateway | `gateway/` | `network/` | 参考P2P连接管理 |

### 8.3 narwhal仓库（facebookresearch/narwhal）

| narwhal组件 | 路径 | CHASE使用方式 |
|-------------|------|--------------|
| Worker（交易打包）| `worker/` | 对应交易池+打包逻辑 |
| Primary（DAG构建）| `primary/` | Narwhal mempool核心 |
| Consensus（Tusk）| `consensus/` | BFT共识主逻辑 |
| Network | `network/` | P2P消息传递 |

---

## 9. 构建与依赖配置

### 9.1 CMakeLists.txt 主配置

```cmake
cmake_minimum_required(VERSION 3.18)
project(CHASE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
add_compile_options(-Wall -Wextra -pthread)

# 外部依赖
find_package(RocksDB REQUIRED)
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# 子模块
add_subdirectory(framework)
add_subdirectory(crypto)
add_subdirectory(storage)
add_subdirectory(network)
add_subdirectory(txpool)
add_subdirectory(tusk)
add_subdirectory(scheduler)
add_subdirectory(executor)
add_subdirectory(state)
add_subdirectory(pipeline)
add_subdirectory(rpc)
add_subdirectory(node)
add_subdirectory(benchmark)

# 测试
enable_testing()
add_subdirectory(tests)
```

### 9.2 关键依赖版本

```
EVMone:        >= 0.10.0   (https://github.com/ethereum/evmone)
libsecp256k1:  >= 0.3.0    (https://github.com/bitcoin-core/secp256k1)
RocksDB:       >= 7.0.0    (https://github.com/facebook/rocksdb)
Protobuf:      >= 3.15     
OpenSSL:       >= 1.1.1
Boost:         >= 1.74     (asio, filesystem)
GTest:         >= 1.11     (测试框架)
spdlog:        >= 1.9      (日志)
```

### 9.3 构建脚本示例

```bash
#!/bin/bash
# build.sh

mkdir -p build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DROCKSDB_ROOT=/usr/local \
    -DEVMONE_ROOT=/usr/local \
    -DSECP256K1_ROOT=/usr/local \
    -DBUILD_TESTS=ON \
    -DBUILD_BENCHMARKS=ON

make -j$(nproc)
```

---

## 10. 测试规范

### 10.1 单元测试

**test_dpp.cpp**：
- 测试FastPath与SimulateFull的读写集一致性
- 测试感知机权重的确定性更新（同输入必须产生同输出）
- 测试SV-branch识别正确性（测试合约含状态相关分支）
- 压力测试：10万笔交易的读写集提取，对比全量EVM模拟的结果

**test_kdg.cpp**：
- 测试并行KDG构建正确性（与串行构建结果完全一致）
- 测试三类依赖关系（WAR/WAW/RAW）识别正确性
- 负载均衡测试：热点键场景下的线程负载分布

**test_cds.cpp**：
- 测试无冲突链提取的正确性（无RAW依赖的交易不应在同一链内有序约束）
- 测试冲突链提取（abort交易正确归入CZ链，写集摘要正确更新）
- 测试链合并操作（多前驱交易正确触发链合并）
- 测试分片分配的确定性（相同输入多次调用结果完全一致）
- 测试负载均衡效果（分片负载标准差）

**test_blp.cpp**：
- 测试AIMD窗口控制（低负载指数增长，高负载乘性减少）
- 测试跨区块并行度（验证Bh+1的调度在Bh执行时已启动）
- 测试反压机制（模拟P3积压，验证P1速率受控降低）

**test_state.cpp**：
- 测试L1状态可见性（Bh+1可读取Bh的未提交L1状态）
- 测试DeltaPage冻结与异步刷写
- 测试并发读写的正确性

### 10.2 集成测试

- **端到端测试**：10节点集群，SmallBank工作负载（账户数10万，Zipfian系数0.5/1.0），验证所有节点最终状态一致
- **拜占庭容错测试**：1个拜占庭节点，验证系统正确性不受影响
- **故障恢复测试**：节点崩溃后重新加入，验证状态同步正确

### 10.3 性能基准测试

参考CHASE论文第6节的实验设置：

```bash
# SmallBank基准（I/O密集型）
./benchmark/SmallBankBenchmark \
    --accounts=100000 \
    --zipfian=0.5 \
    --block-size=5300 \
    --threads=32 \
    --duration=60

# CPUHeavy基准（计算密集型）
./benchmark/CPUHeavyBenchmark \
    --threads=4,8,16,32 \
    --duration=60
```

预期指标（单机32线程）：
- SmallBank（Zipfian=0）：≥ 100 kTPS
- SmallBank（Zipfian=0.5）：≥ 80 kTPS
- SmallBank（Zipfian=1.0）：≥ 60 kTPS（高争用场景）
- 峰值TPS：目标 170.6 kTPS（论文结果）

---

## 11. 待完成项 Checklist

以下是基于论文和参考仓库梳理的完整实现清单，用于检验现有CHASE仓库的完成度：

### 核心模块
- [ ] **tusk**：Tusk BFT共识集成（Narwhal mempool + BullShark/Tusk DAG共识）
- [ ] **txpool**：交易池（签名验证、去重、批次提供）

### DPP（最重要：KDG的基础）
- [ ] **SV-branch识别**：基于栈追踪的SLOAD污点标记与JUMPI检测
- [ ] **感知机预测器**：零初始化权重，区块粒度原子提交，多副本确定性保证
- [ ] **多版本状态层**：按共识顺序组织，TxID隔离读取
- [ ] **FastPath**：检查点机制，仅追踪SLOAD/SSTORE
- [ ] **SimulateFull**：基于多版本状态层的精确模拟
- [ ] **自适应阈值θ**：基于历史置信度分布动态调整
- [ ] **一致性回退**：FastPath检测状态访问偏离时正确回退

### KDG并行构建
- [ ] **键哈希分片**：按键哈希值分配到M个线程桶
- [ ] **热点键处理**：高频键独立分配避免负载倾斜
- [ ] **依赖类型推导**：WAR/WAW/RAW三类依赖正确识别
- [ ] **RAW依赖标记**：abort交易正确标记

### CDS调度
- [ ] **交易拓扑排序**：基于KDG的序列索引分配
- [ ] **WAW冲突消解**：按TxID选取写赢家，其余标为abort
- [ ] **无冲突依赖链提取**：链创建、追加、合并三种规则完整实现
- [ ] **并行后继支持**：共享链尾的无依赖交易允许并行调度
- [ ] **冲突依赖链提取**：写集摘要快速判断，Inter-epoch Reordering
- [ ] **逻辑绑定关系**：CZ链与CFZ链的D_bind计算
- [ ] **传递闭包聚簇**：间接数据耦合的簇合并
- [ ] **LPT装箱分配**：初始分配 + 簇迁移/交换局部优化
- [ ] **确定性保证**：固定排序、字典序决策、K_max上限

### CDS执行
- [ ] **CFZ无锁并行执行**：链内串行，链间并行，写入本地缓冲
- [ ] **CFZ→CZ屏障**：正确同步
- [ ] **CZ乐观执行**：记录实际rw_keys
- [ ] **CZ有效性检查**：prev_rw_keys == actual_rw_keys
- [ ] **W_shard快速判定**：无效交易与其他链写集不相交时转为并行提交
- [ ] **重执行**：剩余无效交易的重执行逻辑

### BLP流水线
- [ ] **四阶段解耦**：Ordering/Scheduling/Execution/Commit独立推进
- [ ] **AIMD窗口控制**：W2（Gas积压）和W3（字节积压）的两个窗口
- [ ] **自适应反压**：下游积压触发上游速率调节
- [ ] **跨区块状态访问**：Bh+1调度时可访问Bh的L1未提交状态

### 两层缓存
- [ ] **DeltaPage**：128条记录上限，4KB对齐
- [ ] **MemIndexTable**：256MB容量阈值，冻结与异步刷写
- [ ] **L1读取**：内存拷贝，未提交状态可见性
- [ ] **L2 LRU缓存**：已提交状态的磁盘缓存
- [ ] **写时复制（CoW）**：L1只存写状态

### 基础设施
- [ ] **EVMone集成**：完整EVM执行 + FastPath模式
- [ ] **libsecp256k1集成**：签名验证
- [ ] **RocksDB集成**：多Column Family，WriteBatch，BlockCache
- [ ] **Merkle Patricia Trie**：状态根计算
- [ ] **P2P网络**：全网状拓扑，消息路由
- [ ] **JSON-RPC**：客户端接口

### 测试与基准
- [ ] **SmallBank工作负载**：Zipfian系数可调
- [ ] **CPUHeavy工作负载**：快速排序智能合约
- [ ] **节点可扩展性测试**：4/10/100节点配置
- [ ] **分片可扩展性测试**：1~6分片配置

---

## 附录：关键设计决策与注意事项

### A1. 关于DPP的确定性保证

**最关键的正确性约束**：感知机权重更新必须在所有副本上产生完全相同的结果。实现时注意：
- 权重必须以相同的精度（推荐int32_t或定点数，避免浮点数差异）存储
- 更新顺序必须与共识确定的交易顺序完全一致
- 区块粒度的原子提交：区块内的权重更新累积，在区块提交后统一应用

### A2. 关于CZ重执行的频率控制

论文中CZ交易的重执行是"最坏情况"路径。在实际实现中：
- 通过DPP的高准确率（正常情况下>95%），大多数CZ交易无需重执行
- 可以通过监控重执行率来调整DPP的阈值θ

### A3. 关于BLP的内存管理

在流水线并行的情况下，同一时刻可能有多个区块（Bh, Bh-1, Bh-2, Bh-3）处于不同阶段。L1缓存需要支持多区块的未提交状态共存，并按区块高度正确隔离。

### A4. 关于与optme的最大差异

optme中每个Epoch独立运行，Epoch 1的序列是独立的"读写不相交"序列（等同于CHASE的链但没有显式的链内依赖连接）。CHASE的关键改进是：
- 将optme序列中存在数据依赖的交易**显式串联为依赖链**，形成线性时序结构
- 冲突交易不是分散到多个后续Epoch，而是统一组织到单一CZ，消除Epoch数量的不确定性
- 这使得调度粒度从"交易级"提升到"链级"，减少了调度开销和跨Epoch同步

### A5. 关于C++与Rust的互操作

若选择保留narwhal的Rust实现作为共识层，建议：
1. 将narwhal编译为独立的可执行进程
2. 通过Unix Domain Socket或gRPC与C++ CHASE主程序通信
3. 接口消息格式使用Protobuf序列化

具体接口：
```protobuf
// chase_consensus.proto
service ConsensusService {
    rpc SubmitTransaction(TxRequest) returns (TxResponse);
    rpc SubscribeBlocks(SubscribeRequest) returns (stream BlockNotification);
}
```
