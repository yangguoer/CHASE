#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace chase {

// 前向声明
class Transaction;
class Block;

// 基础类型定义
using TxID = uint64_t;           // 交易ID（单调递增）
using BlockHeight = uint64_t;    // 区块高度
using ChainID = uint32_t;        // 依赖链ID
using ShardID = uint32_t;        // 分片ID
using ClusterID = uint32_t;      // 调度簇ID

// 哈希类型（32字节）
using Hash = std::array<uint8_t, 32>;
using bytes = std::vector<uint8_t>;
using bytes32 = std::array<uint8_t, 32>;

// 地址类型（20字节，以太坊兼容）
using Address = std::array<uint8_t, 20>;

// 签名类型（65字节：r(32) + s(32) + v(1)）
using Signature = std::array<uint8_t, 65>;

// 公钥类型（64字节：x(32) + y(32)）
using PublicKey = std::array<uint8_t, 64>;

// 节点ID
using NodeID = std::string;

// 枚举定义
enum class OpType {
    READ,
    WRITE
};

enum class DependencyType {
    RAW,  // Read-After-Write (硬约束)
    WAR,  // Write-After-Read (良性)
    WAW   // Write-After-Write (良性)
};

enum class ChainType {
    CFZ,  // Conflict-Free Zone
    CZ    // Conflict Zone
};

enum class BlockState {
    PROPOSED,   // Bh^p: 共识提出
    ORDERED,    // 交易全序确定
    SCHEDULED,  // 调度计划生成
    EXECUTED,   // Bh^e: 执行完成
    COMMITTED   // Bh^c: 提交完成
};

// 存储键结构
struct StorageKey {
    Address account;
    bytes32 slot;
    
    bool operator==(const StorageKey& other) const {
        return account == other.account && slot == other.slot;
    }
    
    struct Hasher {
        size_t operator()(const StorageKey& key) const {
            size_t h1 = 0;
            for (auto b : key.account) h1 = h1 * 31 + b;
            size_t h2 = 0;
            for (auto b : key.slot) h2 = h2 * 31 + b;
            return h1 ^ (h2 << 1);
        }
    };
};

// 读写集
struct ReadWriteSet {
    std::unordered_set<StorageKey, StorageKey::Hasher> reads;
    std::unordered_map<StorageKey, bytes, StorageKey::Hasher> writes;
    
    bool operator==(const ReadWriteSet& other) const {
        return reads == other.reads && writes == other.writes;
    }
};

// 执行结果
struct ExecResult {
    bool success;
    uint64_t gasUsed;
    bytes output;
    std::string errorMsg;
    ReadWriteSet rwSet;
};

// 模拟结果（用于DPP）
struct SimResult {
    bool success;
    uint64_t gasUsed;
    ReadWriteSet rwSet;
    bool usedFastPath;  // 是否使用了FastPath
};

} // namespace chase
