#pragma once

#include "framework/protocol/Common.h"
#include <vector>
#include <unordered_map>
#include <mutex>

namespace chase {
namespace scheduler {

// 键操作记录
struct KeyOperation {
    TxID txId;
    OpType opType;     // READ or WRITE
    bytes value;       // 写操作时有效
    
    KeyOperation() : txId(0), opType(OpType::READ) {}
    KeyOperation(TxID id, OpType type, const bytes& val = {})
        : txId(id), opType(type), value(val) {}
};

// KDG - 键级依赖图
class KDG {
public:
    // key -> 操作序列（按txId升序）
    std::unordered_map<StorageKey, std::vector<KeyOperation>, StorageKey::Hasher> keySequences;
    
    // 并行构建时的线程局部数据
    struct ThreadLocalKDG {
        std::unordered_map<StorageKey, std::vector<KeyOperation>, StorageKey::Hasher> localSequences;
    };
    
    // 向指定key添加操作
    void addOperation(const StorageKey& key, const KeyOperation& op);
    
    // 合并线程局部KDG到全局KDG
    void mergeFrom(const ThreadLocalKDG& local);
    
    // 推导依赖关系
    DependencyType getDependency(TxID u, TxID v, const StorageKey& key) const;
    
    // 获取某笔交易的所有直接前驱
    std::vector<TxID> getPredecessors(TxID txId) const;
    
    // 获取所有出现过的交易ID
    std::vector<TxID> getAllTxIds() const;
    
    // 清空KDG
    void clear();
    
private:
    mutable std::mutex mutex_;  // 保护并发访问
};

} // namespace scheduler
} // namespace chase
