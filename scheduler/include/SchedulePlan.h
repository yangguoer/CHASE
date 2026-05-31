#pragma once

#include "framework/protocol/Common.h"
#include "KDG.h"
#include <vector>
#include <unordered_set>

namespace chase {
namespace scheduler {

// 依赖链
struct DependencyChain {
    ChainID id;
    ChainType type;           // CFZ or CZ
    std::vector<TxID> txOrder;  // 链内交易执行顺序（严格有序）
    uint64_t load;            // Gas估算负载
    
    // 仅CZ链使用：逻辑绑定的CFZ链集合
    std::vector<ChainID> boundCFZChains;    // D_bind(Lc)
    
    // 链的写集摘要（用于快速冲突判断）
    std::unordered_set<StorageKey, StorageKey::Hasher> writeSetSummary;
    
    DependencyChain() : id(0), type(ChainType::CFZ), load(0) {}
    
    // 添加交易到链
    void addTransaction(TxID txId, uint64_t gasUsed) {
        txOrder.push_back(txId);
        load += gasUsed;
    }
    
    // 获取最后一个交易ID
    TxID lastTxId() const {
        return txOrder.empty() ? 0 : txOrder.back();
    }
};

// 调度簇
struct SchedulingCluster {
    ClusterID id;
    std::vector<ChainID> chains;    // 包含的依赖链（CFZ + CZ）
    uint64_t load;                  // 负载 = sum(Load(Li))
    
    SchedulingCluster() : id(0), load(0) {}
    
    void addChain(ChainID chainId, uint64_t chainLoad) {
        chains.push_back(chainId);
        load += chainLoad;
    }
};

// 调度计划 Π
struct SchedulePlan {
    // 无冲突区（Conflict-Free Zone）依赖链集合
    std::vector<DependencyChain> cfzChains;     // L_free
    
    // 冲突区（Conflict Zone）依赖链集合
    std::vector<DependencyChain> czChains;       // L_conflict
    
    // 分片分配：shard_id -> [chain_ids]
    std::vector<std::vector<ChainID>> shardAssignment;
    
    // 调度簇
    std::vector<SchedulingCluster> clusters;
    
    // 块高度
    BlockHeight blockHeight;
    
    SchedulePlan() : blockHeight(0) {}
};

} // namespace scheduler
} // namespace chase
