#pragma once

#include "framework/protocol/Common.h"
#include "framework/protocol/Transaction.h"
#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>

namespace chase {
namespace txpool {

// 交易池
class TxPool {
public:
    using Ptr = std::shared_ptr<TxPool>;
    
    TxPool();
    ~TxPool();
    
    // 提交交易
    bool submit(Transaction::Ptr tx);
    
    // 批量获取交易（按 Gas Price 排序）
    std::vector<Transaction::Ptr> fetchBatch(size_t maxCount, uint64_t maxGas);
    
    // 移除已执行的交易
    void remove(const std::vector<TxID>& txIds);
    
    // 查询交易状态
    Transaction::Ptr query(TxID txId) const;
    
    // 待处理交易数量
    size_t pendingCount() const;
    
private:
    struct TxCompare {
        bool operator()(const Transaction::Ptr& a, const Transaction::Ptr& b) const {
            return a->gasPrice < b->gasPrice;  // 最大堆
        }
    };
    
    std::priority_queue<Transaction::Ptr, std::vector<Transaction::Ptr>, TxCompare> pendingTxs_;
    std::unordered_map<TxID, Transaction::Ptr> txMap_;
    mutable std::mutex mutex_;
};

} // namespace txpool
} // namespace chase
