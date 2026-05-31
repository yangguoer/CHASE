#pragma once

#include "framework/protocol/Common.h"
#include "framework/protocol/Block.h"
#include "KDG.h"
#include "DPP.h"
#include "SchedulePlan.h"
#include <memory>
#include <vector>

namespace chase {
namespace scheduler {

// CDS - Chain-oriented Deterministic Scheduling
class CDS {
public:
    CDS(int numShards = 4);
    
    // 输入：proposed block（已有交易全序），输出：调度计划Π
    SchedulePlan generatePlan(const Block::Ptr& block, 
                              const KDG& kdg);
    
    // 设置分片数量
    void setNumShards(int numShards) { numShards_ = numShards; }
    
private:
    int numShards_;
    DPP dpp_;
    
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
    
    // Step 3a: 无冲突依赖链提取（CFZ）
    std::vector<DependencyChain> extractCFZChains(
        const KDG& kdg,
        const std::vector<TxID>& sortedTxs);
    
    // Step 3b: 冲突依赖链提取（CZ）
    std::vector<DependencyChain> extractCZChains(
        const KDG& kdg,
        const std::vector<TxID>& abortedTxs);
    
    // Step 4: 分片映射（装箱问题 + 负载均衡）
    std::vector<std::vector<ChainID>> 
    assignShards(const std::vector<DependencyChain>& allChains);
    
    // 计算写集摘要
    std::unordered_set<StorageKey, StorageKey::Hasher>
    computeWriteSetSummary(const std::vector<TxID>& txIds, const KDG& kdg);
};

} // namespace scheduler
} // namespace chase
