#pragma once

#include "Common.h"
#include <string>
#include <memory>

namespace chase {

// 交易结构
class Transaction {
public:
    using Ptr = std::shared_ptr<Transaction>;
    
    TxID id;                    // 共识层分配的单调递增ID
    Hash hash;                  // Keccak256(rlp(tx))
    Address from;               // 发送方地址
    Address to;                 // 接收方地址（合约地址或空）
    uint64_t nonce;             // 随机数
    uint64_t gasPrice;          // Gas价格
    uint64_t gasLimit;          // Gas限制
    bytes data;                 // 合约调用数据
    Signature signature;        // 签名
    
    // 调度阶段填充（DPP产出）
    ReadWriteSet rwSet;         // 预测的读写集
    uint64_t gasUsed;           // 模拟执行的Gas消耗
    
    // 执行阶段填充
    ReadWriteSet actualRWSet;   // 实际执行的读写集
    
    Transaction() : id(0), nonce(0), gasPrice(0), gasLimit(0), gasUsed(0) {}
    
    // 计算交易哈希
    void computeHash();
    
    // 验证签名
    bool verifySignature() const;
    
    // 序列化/反序列化
    bytes serialize() const;
    static Transaction deserialize(const bytes& data);
};

} // namespace chase