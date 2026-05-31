#include "../include/CDS.h"
#include <algorithm>
#include <queue>
#include <cmath>

namespace chase {
namespace scheduler {

CDS::CDS(int numShards) : numShards_(numShards) {}

SchedulePlan CDS::generatePlan(const Block::Ptr& block, 
                                const KDG& kdg) {
    SchedulePlan plan;
    plan.blockHeight = block->header.height;
    
    // Step 1: 交易拓扑排序
    auto sortedTxs = topologicalSort(kdg);
    
    // Step 2: 冲突处理（WAW消解）
    std::unordered_set<TxID> abortedTxs;
    resolveConflicts(kdg, abortedTxs);
    
    // Step 3: 依赖链提取
    auto [cfzChains, czChains] = extractChains(kdg, sortedTxs, abortedTxs);
    plan.cfzChains = std::move(cfzChains);
    plan.czChains = std::move(czChains);
    
    // Step 4: 分片映射
    std::vector<DependencyChain> allChains;
    allChains.insert(allChains.end(), plan.cfzChains.begin(), plan.cfzChains.end());
    allChains.insert(allChains.end(), plan.czChains.begin(), plan.czChains.end());
    
    plan.shardAssignment = assignShards(allChains);
    
    return plan;
}

std::vector<TxID> CDS::topologicalSort(const KDG& kdg) {
    // 基于KDG的拓扑排序
    auto allTxIds = kdg.getAllTxIds();
    
    // 简化实现：按TxID排序（实际应基于依赖关系进行拓扑排序）
    std::sort(allTxIds.begin(), allTxIds.end());
    
    return allTxIds;
}

void CDS::resolveConflicts(const KDG& kdg, 
                           std::unordered_set<TxID>& abortedTxs) {
    // WAW冲突消解：对同一键的多个写操作，TxID最小的为赢家
    for (const auto& [key, ops] : kdg.keySequences) {
        std::unordered_set<TxID> writers;
        
        for (const auto& op : ops) {
            if (op.opType == OpType::WRITE) {
                if (writers.count(op.txId) > 0) {
                    // 发现WAW冲突，标记为abort
                    abortedTxs.insert(op.txId);
                } else {
                    writers.insert(op.txId);
                }
            }
        }
    }
}

std::pair<std::vector<DependencyChain>, std::vector<DependencyChain>>
CDS::extractChains(const KDG& kdg, 
                   const std::vector<TxID>& sortedTxs,
                   const std::unordered_set<TxID>& abortedTxs) {
    auto cfzChains = extractCFZChains(kdg, sortedTxs);
    auto czChains = extractCZChains(kdg, 
                                     std::vector<TxID>(abortedTxs.begin(), abortedTxs.end()));
    
    return {cfzChains, czChains};
}

std::vector<DependencyChain> CDS::extractCFZChains(
    const KDG& kdg,
    const std::vector<TxID>& sortedTxs) {
    
    std::vector<DependencyChain> chains;
    ChainID nextChainId = 0;
    
    // 简化的链提取算法
    // 实际应根据依赖关系将交易串成链
    for (TxID txId : sortedTxs) {
        // 创建新链或追加到现有链
        if (chains.empty()) {
            DependencyChain chain;
            chain.id = nextChainId++;
            chain.type = ChainType::CFZ;
            chain.addTransaction(txId, 1000);  // 假设gas
            chains.push_back(chain);
        } else {
            // 简化：每笔交易单独成链
            DependencyChain chain;
            chain.id = nextChainId++;
            chain.type = ChainType::CFZ;
            chain.addTransaction(txId, 1000);
            chains.push_back(chain);
        }
    }
    
    return chains;
}

std::vector<DependencyChain> CDS::extractCZChains(
    const KDG& kdg,
    const std::vector<TxID>& abortedTxs) {
    
    std::vector<DependencyChain> czChains;
    ChainID nextChainId = 0;
    
    // Inter-epoch Reordering算法
    for (TxID txId : abortedTxs) {
        bool found = false;
        
        // 尝试加入现有CZ链
        for (auto& chain : czChains) {
            // 检查写集冲突
            // 简化：直接加入第一个链
            chain.addTransaction(txId, 1000);
            found = true;
            break;
        }
        
        if (!found) {
            // 创建新CZ链
            DependencyChain chain;
            chain.id = nextChainId++;
            chain.type = ChainType::CZ;
            chain.addTransaction(txId, 1000);
            czChains.push_back(chain);
        }
    }
    
    return czChains;
}

std::vector<std::vector<ChainID>> 
CDS::assignShards(const std::vector<DependencyChain>& allChains) {
    // LPT (Longest Processing Time) 装箱算法
    std::vector<std::vector<ChainID>> shards(numShards_);
    std::vector<uint64_t> shardLoads(numShards_, 0);
    
    // 按负载降序排序
    std::vector<const DependencyChain*> sortedChains;
    for (const auto& chain : allChains) {
        sortedChains.push_back(&chain);
    }
    std::sort(sortedChains.begin(), sortedChains.end(),
        [](const DependencyChain* a, const DependencyChain* b) {
            return a->load > b->load;
        });
    
    // LPT分配
    for (const auto* chain : sortedChains) {
        // 找到负载最小的分片
        auto minIt = std::min_element(shardLoads.begin(), shardLoads.end());
        int shardIdx = std::distance(shardLoads.begin(), minIt);
        
        shards[shardIdx].push_back(chain->id);
        shardLoads[shardIdx] += chain->load;
    }
    
    // 局部优化（简化实现）
    // TODO: 实现簇迁移/交换优化
    
    return shards;
}

std::unordered_set<StorageKey, StorageKey::Hasher>
CDS::computeWriteSetSummary(const std::vector<TxID>& txIds, const KDG& kdg) {
    std::unordered_set<StorageKey, StorageKey::Hasher> writeSet;
    
    for (const auto& [key, ops] : kdg.keySequences) {
        for (const auto& op : ops) {
            if (op.opType == OpType::WRITE && 
                std::find(txIds.begin(), txIds.end(), op.txId) != txIds.end()) {
                writeSet.insert(key);
            }
        }
    }
    
    return writeSet;
}

} // namespace scheduler
} // namespace chase
