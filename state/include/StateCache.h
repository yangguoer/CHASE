#pragma once

#include "framework/protocol/Common.h"
#include <array>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace chase {
namespace state {

// KV条目
struct KVEntry {
    StorageKey key;
    bytes value;
    bool valid;
    
    KVEntry() : valid(false) {}
};

// DeltaPage - 状态变更页（最多128条记录）
class DeltaPage {
public:
    BlockHeight height;
    std::array<KVEntry, 128> entries;
    int count;
    bool frozen;  // 是否已冻结（只读）
    
    DeltaPage() : height(0), count(0), frozen(false) {
        entries.fill(KVEntry());
    }
    
    // 添加KV条目
    bool addEntry(const StorageKey& key, const bytes& value) {
        if (count >= 128 || frozen) return false;
        
        entries[count].key = key;
        entries[count].value = value;
        entries[count].valid = true;
        count++;
        return true;
    }
    
    // 查找key
    const bytes* find(const StorageKey& key) const {
        for (int i = 0; i < count; ++i) {
            if (entries[i].valid && entries[i].key == key) {
                return &entries[i].value;
            }
        }
        return nullptr;
    }
    
    // 冻结页面
    void freeze() {
        frozen = true;
    }
};

// 内存索引表
class MemIndexTable {
public:
    static constexpr size_t CAPACITY_THRESHOLD = 256 * 1024 * 1024; // 256MB
    
    // key -> DeltaPage*
    std::unordered_map<StorageKey, DeltaPage*, StorageKey::Hasher> index;
    size_t totalBytes;
    
    MemIndexTable() : totalBytes(0) {}
    
    // 添加DeltaPage
    void addPage(std::shared_ptr<DeltaPage> page);
    
    // 查找key
    const bytes* lookup(const StorageKey& key) const;
    
    // 容量达到阈值时冻结
    void maybeFreeze();
    
    // 获取所有冻结的页面（用于异步刷写）
    std::vector<std::shared_ptr<DeltaPage>> getFrozenPages();
    
private:
    std::vector<std::shared_ptr<DeltaPage>> pages_;
    std::vector<std::shared_ptr<DeltaPage>> frozenPages_;
    mutable std::mutex mutex_;
};

// L1未提交状态缓存
class L1UncommittedCache {
public:
    using Ptr = std::shared_ptr<L1UncommittedCache>;
    
    L1UncommittedCache();
    
    // 读取状态（先查L1）
    bytes read(const StorageKey& key) const;
    
    // 写入状态（写入临时缓冲区）
    void write(const StorageKey& key, const bytes& value);
    
    // 完成区块执行，将缓冲区编码为DeltaPage
    std::shared_ptr<DeltaPage> finalizeBlock(BlockHeight height);
    
    // 获取MemIndexTable
    MemIndexTable& getIndexTable() { return indexTable_; }
    
private:
    MemIndexTable indexTable_;
    
    // 当前区块的临时缓冲区
    struct Buffer {
        BlockHeight height;
        std::unordered_map<StorageKey, bytes, StorageKey::Hasher> writes;
    };
    
    Buffer currentBuffer_;
    mutable std::mutex mutex_;
};

// L2已提交状态缓存（LRU）
class L2CommittedCache {
public:
    using Ptr = std::shared_ptr<L2CommittedCache>;
    
    static constexpr size_t DEFAULT_CAPACITY = 10000;
    
    L2CommittedCache(size_t capacity = DEFAULT_CAPACITY);
    
    // 读取状态
    bytes read(const StorageKey& key) const;
    
    // 写入状态
    void write(const StorageKey& key, const bytes& value);
    
    // 从DeltaPage加载
    void loadFromDeltaPage(const DeltaPage& page);
    
private:
    size_t capacity_;
    std::unordered_map<StorageKey, bytes, StorageKey::Hasher> cache_;
    std::vector<StorageKey> accessOrder_;  // LRU顺序
    mutable std::mutex mutex_;
    
    void evictIfNeeded();
};

// 状态缓存主接口（L1 + L2）
class StateCache {
public:
    using Ptr = std::shared_ptr<StateCache>;
    
    StateCache();
    
    // 读取状态（L1 -> L2 -> RocksDB）
    bytes read(const StorageKey& key);
    
    // 写入状态（写入L1）
    void write(const StorageKey& key, const bytes& value);
    
    // 完成区块执行
    void finalizeBlock(BlockHeight height);
    
    // 异步刷写L1到L2和RocksDB
    void asyncFlush();
    
    // 获取L1和L2缓存
    L1UncommittedCache::Ptr getL1() { return l1Cache_; }
    L2CommittedCache::Ptr getL2() { return l2Cache_; }
    
private:
    L1UncommittedCache::Ptr l1Cache_;
    L2CommittedCache::Ptr l2Cache_;
    
    // TODO: RocksDB存储引用
    // RocksDBStorage* storage_;
};

} // namespace state
} // namespace chase
