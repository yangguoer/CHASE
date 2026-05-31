#include "../include/KDG.h"
#include <algorithm>
#include <set>

namespace chase {
namespace scheduler {

void KDG::addOperation(const StorageKey& key, const KeyOperation& op) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& seq = keySequences[key];
    
    // 按txId升序插入，保持有序
    auto it = std::lower_bound(seq.begin(), seq.end(), op.txId,
        [](const KeyOperation& op, TxID txId) {
            return op.txId < txId;
        });
    
    // 同一交易的读操作优先于写操作
    if (it != seq.end() && it->txId == op.txId) {
        if (op.opType == OpType::READ && it->opType == OpType::WRITE) {
            // 在读之前插入
            seq.insert(it, op);
        } else {
            // 在写之后插入
            seq.insert(it + 1, op);
        }
    } else {
        seq.insert(it, op);
    }
}

void KDG::mergeFrom(const ThreadLocalKDG& local) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [key, ops] : local.localSequences) {
        auto& globalSeq = keySequences[key];
        globalSeq.insert(globalSeq.end(), ops.begin(), ops.end());
        
        // 重新排序
        std::sort(globalSeq.begin(), globalSeq.end(),
            [](const KeyOperation& a, const KeyOperation& b) {
                if (a.txId != b.txId) return a.txId < b.txId;
                // 读优先于写
                return a.opType == OpType::READ && b.opType == OpType::WRITE;
            });
    }
}

DependencyType KDG::getDependency(TxID u, TxID v, const StorageKey& key) const {
    auto it = keySequences.find(key);
    if (it == keySequences.end()) {
        return DependencyType::RAW;  // 无依赖
    }
    
    const auto& seq = it->second;
    bool foundU = false, foundV = false;
    OpType uType, vType;
    
    for (const auto& op : seq) {
        if (op.txId == u) {
            foundU = true;
            uType = op.opType;
        }
        if (op.txId == v) {
            foundV = true;
            vType = op.opType;
        }
        if (foundU && foundV) break;
    }
    
    if (!foundU || !foundV) {
        return DependencyType::RAW;
    }
    
    // 判断依赖类型
    if (uType == OpType::WRITE && vType == OpType::READ) {
        return DependencyType::RAW;  // Write-After-Read (硬约束)
    } else if (uType == OpType::READ && vType == OpType::WRITE) {
        return DependencyType::WAR;  // Read-After-Write (良性)
    } else if (uType == OpType::WRITE && vType == OpType::WRITE) {
        return DependencyType::WAW;  // Write-After-Write (良性)
    }
    
    return DependencyType::RAW;
}

std::vector<TxID> KDG::getPredecessors(TxID txId) const {
    std::set<TxID> predecessors;
    
    for (const auto& [key, seq] : keySequences) {
        // 找到当前交易在该键上的操作位置
        size_t currentPos = 0;
        bool foundCurrent = false;
        
        for (size_t i = 0; i < seq.size(); ++i) {
            if (seq[i].txId == txId) {
                currentPos = i;
                foundCurrent = true;
                break;
            }
        }
        
        if (!foundCurrent) continue;
        
        // 查找当前操作之前的所有依赖
        for (size_t i = 0; i < currentPos; ++i) {
            const auto& prevOp = seq[i];
            const auto& currOp = seq[currentPos];
            
            // RAW: 之前WRITE，当前READ
            if (prevOp.opType == OpType::WRITE && currOp.opType == OpType::READ) {
                predecessors.insert(prevOp.txId);
            }
            // WAR: 之前READ，当前WRITE
            else if (prevOp.opType == OpType::READ && currOp.opType == OpType::WRITE) {
                predecessors.insert(prevOp.txId);
            }
            // WAW: 之前WRITE，当前WRITE
            else if (prevOp.opType == OpType::WRITE && currOp.opType == OpType::WRITE) {
                predecessors.insert(prevOp.txId);
            }
        }
    }
    
    return std::vector<TxID>(predecessors.begin(), predecessors.end());
}

std::vector<TxID> KDG::getAllTxIds() const {
    std::set<TxID> txIds;
    for (const auto& [key, seq] : keySequences) {
        for (const auto& op : seq) {
            txIds.insert(op.txId);
        }
    }
    return std::vector<TxID>(txIds.begin(), txIds.end());
}

void KDG::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    keySequences.clear();
}

} // namespace scheduler
} // namespace chase
