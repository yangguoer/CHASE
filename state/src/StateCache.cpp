#include "../include/StateCache.h"
#include <algorithm>

namespace chase {
namespace state {

// MemIndexTable 实现
void MemIndexTable::addPage(std::shared_ptr<DeltaPage> page) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (int i = 0; i < page->count; ++i) {
        if (page->entries[i].valid) {
            index[page->entries[i].key] = page.get();
            totalBytes += page->entries[i].value.size() + sizeof(StorageKey);
        }
    }
    
    pages_.push_back(page);
}

const bytes* MemIndexTable::lookup(const StorageKey& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = index.find(key);
    if (it != index.end()) {
        return it->second->find(key);
    }
    return nullptr;
}

void MemIndexTable::maybeFreeze() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (totalBytes >= CAPACITY_THRESHOLD) {
        // 冻结所有未冻结的页面
        for (auto& page : pages_) {
            if (!page->frozen) {
                page->freeze();
                frozenPages_.push_back(page);
            }
        }
        
        // TODO: 触发异步刷写到L2和RocksDB
    }
}

std::vector<std::shared_ptr<DeltaPage>> MemIndexTable::getFrozenPages() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto pages = frozenPages_;
    frozenPages_.clear();
    return pages;
}

// L1UncommittedCache 实现
L1UncommittedCache::L1UncommittedCache() {
    currentBuffer_.height = 0;
}

bytes L1UncommittedCache::read(const StorageKey& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 先查当前缓冲区
    auto it = currentBuffer_.writes.find(key);
    if (it != currentBuffer_.writes.end()) {
        return it->second;
    }
    
    // 再查MemIndexTable
    auto result = indexTable_.lookup(key);
    if (result) {
        return *result;
    }
    
    return bytes();  // 未找到
}

void L1UncommittedCache::write(const StorageKey& key, const bytes& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentBuffer_.writes[key] = value;
}

std::shared_ptr<DeltaPage> L1UncommittedCache::finalizeBlock(BlockHeight height) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto page = std::make_shared<DeltaPage>();
    page->height = height;
    
    for (const auto& [key, value] : currentBuffer_.writes) {
        if (!page->addEntry(key, value)) {
            // 页面已满，创建新页面（简化处理）
            break;
        }
    }
    
    page->freeze();
    indexTable_.addPage(page);
    
    // 清空缓冲区
    currentBuffer_.writes.clear();
    currentBuffer_.height = height + 1;
    
    return page;
}

// L2CommittedCache 实现
L2CommittedCache::L2CommittedCache(size_t capacity) 
    : capacity_(capacity) {}

bytes L2CommittedCache::read(const StorageKey& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        // LRU缓存命中，但不修改顺序（const方法）
        return it->second;
    }
    
    return bytes();  // 未找到
}

void L2CommittedCache::write(const StorageKey& key, const bytes& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    evictIfNeeded();
    
    cache_[key] = value;
    accessOrder_.push_back(key);
}

void L2CommittedCache::loadFromDeltaPage(const DeltaPage& page) {
    for (int i = 0; i < page.count; ++i) {
        if (page.entries[i].valid) {
            write(page.entries[i].key, page.entries[i].value);
        }
    }
}

void L2CommittedCache::evictIfNeeded() {
    if (cache_.size() >= capacity_ && !accessOrder_.empty()) {
        // 移除最久未使用的条目
        auto lruKey = accessOrder_.front();
        accessOrder_.erase(accessOrder_.begin());
        cache_.erase(lruKey);
    }
}

// StateCache 实现
StateCache::StateCache() 
    : l1Cache_(std::make_shared<L1UncommittedCache>()),
      l2Cache_(std::make_shared<L2CommittedCache>()) {}

bytes StateCache::read(const StorageKey& key) {
    // L1查找
    auto l1Result = l1Cache_->read(key);
    if (!l1Result.empty()) {
        return l1Result;
    }
    
    // L2查找
    auto l2Result = l2Cache_->read(key);
    if (!l2Result.empty()) {
        return l2Result;
    }
    
    // TODO: RocksDB查找
    // return storage_->read(key);
    
    return bytes();  // 未找到
}

void StateCache::write(const StorageKey& key, const bytes& value) {
    l1Cache_->write(key, value);
}

void StateCache::finalizeBlock(BlockHeight height) {
    auto deltaPage = l1Cache_->finalizeBlock(height);
    
    // 加载到L2缓存
    l2Cache_->loadFromDeltaPage(*deltaPage);
    
    // TODO: 异步刷写到RocksDB
}

void StateCache::asyncFlush() {
    // TODO: 实现异步刷写逻辑
    auto frozenPages = l1Cache_->getIndexTable().getFrozenPages();
    
    for (const auto& page : frozenPages) {
        l2Cache_->loadFromDeltaPage(*page);
        
        // TODO: 写入RocksDB
        // storage_->writeBatch(*page);
    }
}

} // namespace state
} // namespace chase
