#include "../include/TxPool.h"
#include <algorithm>

namespace chase {
namespace txpool {

TxPool::TxPool() {}

bool TxPool::submit(Transaction::Ptr tx) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查池大小
    if (pendingTxs_.size() >= MAX_POOL_SIZE) {
        return false;
    }
    
    // 去重
    if (txHashes_.count(tx->hash) > 0) {
        return false;
    }
    
    // 验证签名
    if (!tx->verifySignature()) {
        return false;
    }
    
    // 添加到池
    pendingTxs_.push(tx);
    txIndex_[tx->hash] = tx;
    txStatus_[tx->hash] = TxStatus::PENDING;
    txHashes_.insert(tx->hash);
    
    return true;
}

std::vector<Transaction::Ptr> TxPool::fetchBatch(size_t maxCount, uint64_t maxGas) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Transaction::Ptr> batch;
    uint64_t totalGas = 0;
    
    while (batch.size() < maxCount && !pendingTxs_.empty()) {
        auto tx = pendingTxs_.top();
        
        // 检查Gas限制
        if (totalGas + tx->gasLimit > maxGas) {
            break;
        }
        
        pendingTxs_.pop();
        batch.push_back(tx);
        totalGas += tx->gasLimit;
    }
    
    return batch;
}

void TxPool::remove(const std::vector<TxHash>& txHashes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& hash : txHashes) {
        txStatus_[hash] = TxStatus::EXECUTED;
        txIndex_.erase(hash);
        txHashes_.erase(hash);
    }
}

TxStatus TxPool::query(const TxHash& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = txStatus_.find(hash);
    if (it != txStatus_.end()) {
        return it->second;
    }
    
    return TxStatus::DROPPED;
}

size_t TxPool::pendingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pendingTxs_.size();
}

} // namespace txpool
} // namespace chase
