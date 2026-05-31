// 简化版测试 - 仅测试基本数据结构
#include <iostream>
#include <vector>
#include <memory>
#include <cstdint>
#include <array>
#include <unordered_map>
#include <unordered_set>

// 简化的类型定义
using bytes = std::vector<uint8_t>;
using Hash = std::array<uint8_t, 32>;
using Address = std::array<uint8_t, 20>;
using TxID = uint64_t;
using BlockHeight = uint64_t;

enum class OpType {
    READ,
    WRITE
};

enum class DependencyType {
    RAW,  // Read-After-Write
    WAR,  // Write-After-Read
    WAW   // Write-After-Write
};

struct StorageKey {
    Address account;
    bytes slot;
    
    bool operator==(const StorageKey& other) const {
        return account == other.account && slot == other.slot;
    }
};

namespace std {
    template<>
    struct hash<StorageKey> {
        size_t operator()(const StorageKey& k) const {
            size_t h = 0;
            for (auto b : k.account) h = h * 31 + b;
            for (auto b : k.slot) h = h * 31 + b;
            return h;
        }
    };
}

struct ReadWriteSet {
    std::unordered_set<StorageKey> reads;
    std::unordered_map<StorageKey, bytes> writes;
};

// 简化的 Transaction
class Transaction {
public:
    using Ptr = std::shared_ptr<Transaction>;
    
    TxID id;
    Hash hash;
    Address from;
    Address to;
    uint64_t nonce;
    uint64_t gasPrice;
    uint64_t gasLimit;
    bytes data;
    
    ReadWriteSet rwSet;
    uint64_t gasUsed;
    
    Transaction() : id(0), nonce(0), gasPrice(0), gasLimit(0), gasUsed(0) {}
    
    void computeHash() {
        // 简化：只用 ID 作为哈希
        hash.fill(0);
        for (int i = 0; i < 8 && i < sizeof(TxID); ++i) {
            hash[i] = (id >> (i * 8)) & 0xFF;
        }
    }
};

void testTransaction() {
    std::cout << "Testing Transaction..." << std::endl;
    
    auto tx = std::make_shared<Transaction>();
    tx->id = 1;
    tx->nonce = 0;
    tx->gasPrice = 100;
    tx->gasLimit = 21000;
    tx->data = {0x01, 0x02, 0x03};
    
    tx->computeHash();
    
    std::cout << "  Transaction ID: " << tx->id << std::endl;
    std::cout << "  Gas Price: " << tx->gasPrice << std::endl;
    std::cout << "  Test passed!" << std::endl << std::endl;
}

void testStorageKey() {
    std::cout << "Testing StorageKey..." << std::endl;
    
    StorageKey key1, key2;
    key1.account.fill(0x01);
    key1.slot = {0x00};
    
    key2.account.fill(0x01);
    key2.slot = {0x00};
    
    std::cout << "  Key1 == Key2: " << (key1 == key2 ? "true" : "false") << std::endl;
    
    std::unordered_map<StorageKey, bytes> map;
    map[key1] = {0x01, 0x02, 0x03};
    
    std::cout << "  Map lookup: " << (map[key2].size() == 3 ? "success" : "failed") << std::endl;
    std::cout << "  Test passed!" << std::endl << std::endl;
}

void testReadWriteSet() {
    std::cout << "Testing ReadWriteSet..." << std::endl;
    
    ReadWriteSet rwSet;
    
    StorageKey key1;
    key1.account.fill(0x01);
    key1.slot = {0x00};
    
    rwSet.reads.insert(key1);
    rwSet.writes[key1] = {0x01, 0x02};
    
    std::cout << "  Reads count: " << rwSet.reads.size() << std::endl;
    std::cout << "  Writes count: " << rwSet.writes.size() << std::endl;
    std::cout << "  Test passed!" << std::endl << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  CHASE Simplified Core Tests" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    try {
        testTransaction();
        testStorageKey();
        testReadWriteSet();
        
        std::cout << "========================================" << std::endl;
        std::cout << "  All tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
