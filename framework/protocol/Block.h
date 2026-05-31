#pragma once

#include "Common.h"
#include "Transaction.h"
#include <vector>
#include <memory>
#include <string>

// 前向声明，具体定义取决于项目其他部分
namespace chase {
    struct SchedulePlan;
    struct StateSnapshot;
    enum class BlockState; 
}

namespace chase {

// 交易回执
struct TransactionReceipt {
    TxID txId;
    bool success;
    uint64_t gasUsed;
    bytes output;
    std::string errorMsg;

    TransactionReceipt() : success(false), gasUsed(0) {}
};

// 区块头
struct BlockHeader {
    BlockHeight height;
    Hash parentHash;
    Hash txRoot;          // 交易Merkle根
    Hash stateRoot;       // 状态根
    Hash receiptsRoot;    // 收据根
    uint64_t timestamp;
    uint64_t gasUsed;
    uint64_t gasLimit;
    
    BlockHeader() : height(0), timestamp(0), gasUsed(0), gasLimit(0) {}
};

// 区块结构
class Block {
public:
    using Ptr = std::shared_ptr<Block>;
    
    BlockHeader header;
    std::vector<Transaction::Ptr> transactions;
    std::vector<TransactionReceipt> receipts;
    BlockState state;
    
    // 不同阶段附加的数据
    struct SchedulePlan* plan;        // SCHEDULED后有效
    struct StateSnapshot* stateSnapshot; // EXECUTED后有效
    
    Block() : state(BlockState::PROPOSED), plan(nullptr), stateSnapshot(nullptr) {}
    
    // 计算区块哈希
    Hash computeHash() const;
    
    // 添加交易
    void addTransaction(Transaction::Ptr tx);
    
    // 获取交易数量
    size_t transactionCount() const { return transactions.size(); }
    
    // 序列化/反序列化
    bytes serialize() const;
    static Block deserialize(const bytes& data);
};

} // namespace chase