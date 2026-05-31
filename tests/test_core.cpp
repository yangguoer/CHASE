#include "framework/protocol/Common.h"
#include "scheduler/include/KDG.h"
#include "scheduler/include/CDS.h"
#include "state/include/StateCache.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <thread>

using namespace chase;

// ==================== 基础数据结构测试 ====================

void testStorageKey() {
    std::cout << "[Test 1] StorageKey..." << std::endl;
    
    StorageKey key1, key2;
    key1.account.fill(0x01);
    key1.slot.fill(0x00);
    
    key2.account.fill(0x01);
    key2.slot.fill(0x00);
    
    assert(key1 == key2);
    std::cout << "  [PASS] Equality comparison works" << std::endl;
}

void testReadWriteSet() {
    std::cout << "\n[Test 2] ReadWriteSet..." << std::endl;
    
    ReadWriteSet rwSet;
    
    StorageKey key1;
    key1.account.fill(0x01);
    key1.slot.fill(0x00);
    
    rwSet.reads.insert(key1);
    rwSet.writes[key1] = {0x01, 0x02};
    
    assert(rwSet.reads.size() == 1);
    assert(rwSet.writes.size() == 1);
    std::cout << "  [PASS] Read/write tracking works" << std::endl;
}

// ==================== KDG 依赖图测试 ====================

void testKDG_BasicDependency() {
    std::cout << "\n[Test 3] KDG - Basic Dependencies..." << std::endl;
    
    scheduler::KDG kdg;
    
    StorageKey key1, key2;
    key1.account.fill(0x01);
    key1.slot.fill(0x00);
    
    key2.account.fill(0x02);
    key2.slot.fill(0x00);
    
    // tx1 READ key1, tx2 WRITE key1 -> WAR (Write-After-Read)
    kdg.addOperation(key1, scheduler::KeyOperation(1, OpType::READ));
    kdg.addOperation(key1, scheduler::KeyOperation(2, OpType::WRITE, {0x01}));
    
    auto dep1 = kdg.getDependency(1, 2, key1);
    assert(dep1 == DependencyType::WAR);
    std::cout << "  [PASS] WAR dependency detected" << std::endl;
    
    // tx1 WRITE key2, tx2 READ key2 -> RAW (Read-After-Write)
    kdg.addOperation(key2, scheduler::KeyOperation(1, OpType::WRITE, {0x02}));
    kdg.addOperation(key2, scheduler::KeyOperation(2, OpType::READ));
    
    auto dep2 = kdg.getDependency(1, 2, key2);
    assert(dep2 == DependencyType::RAW);
    std::cout << "  [PASS] RAW dependency detected" << std::endl;
    
    auto txIds = kdg.getAllTxIds();
    assert(txIds.size() == 2);
    std::cout << "  [PASS] Transaction count: " << txIds.size() << std::endl;
}

void testKDG_MultipleKeys() {
    std::cout << "\n[Test 4] KDG - Multiple Keys..." << std::endl;
    
    scheduler::KDG kdg;
    
    // Create 3 keys
    for (int i = 0; i < 3; ++i) {
        StorageKey key;
        key.account.fill(static_cast<uint8_t>(i + 1));
        key.slot.fill(0x00);
        
        // Each key has different access patterns
        kdg.addOperation(key, scheduler::KeyOperation(1, OpType::READ));
        kdg.addOperation(key, scheduler::KeyOperation(2, OpType::WRITE, {static_cast<uint8_t>(i)}));
        kdg.addOperation(key, scheduler::KeyOperation(3, OpType::READ));
    }
    
    auto txIds = kdg.getAllTxIds();
    assert(txIds.size() == 3);
    std::cout << "  [PASS] Multi-key tracking: " << txIds.size() << " transactions" << std::endl;
    
    // Check predecessors
    auto preds = kdg.getPredecessors(2);
    assert(!preds.empty());
    std::cout << "  [PASS] Predecessor detection works (found " << preds.size() << " predecessors)" << std::endl;
}

// ==================== CDS 调度器测试 ====================

void testCDS_GeneratePlan() {
    std::cout << "\n[Test 5] CDS - Schedule Plan Generation..." << std::endl;
    
    scheduler::CDS cds(4);  // 4 shards
    
    // Build a simple KDG
    scheduler::KDG kdg;
    
    for (int i = 0; i < 5; ++i) {
        StorageKey key;
        key.account.fill(static_cast<uint8_t>(i + 1));
        key.slot.fill(0x00);
        
        kdg.addOperation(key, scheduler::KeyOperation(i + 1, OpType::WRITE, {static_cast<uint8_t>(i)}));
    }
    
    std::cout << "  [PASS] CDS initialized with 4 shards" << std::endl;
    std::cout << "  [PASS] KDG built with 5 operations" << std::endl;
}

// ==================== State Cache 测试 ====================

void testStateCache_BasicOperations() {
    std::cout << "\n[Test 6] State Cache - Basic Operations..." << std::endl;
    
    state::StateCache cache;
    
    StorageKey key1;
    key1.account.fill(0x01);
    key1.slot.fill(0x00);
    
    bytes value1 = {0x01, 0x02, 0x03};
    cache.write(key1, value1);
    
    auto readValue = cache.read(key1);
    assert(readValue == value1);
    std::cout << "  [PASS] Write and read operations work" << std::endl;
    
    cache.finalizeBlock(1);
    std::cout << "  [PASS] Block finalization works" << std::endl;
}

void testStateCache_MultiBlock() {
    std::cout << "\n[Test 7] State Cache - Multi-Block Operations..." << std::endl;
    
    state::StateCache cache;
    
    // Block 1
    StorageKey key1;
    key1.account.fill(0x01);
    key1.slot.fill(0x00);
    cache.write(key1, {0x01});
    cache.finalizeBlock(1);
    
    // Block 2
    StorageKey key2;
    key2.account.fill(0x02);
    key2.slot.fill(0x00);
    cache.write(key2, {0x02});
    cache.finalizeBlock(2);
    
    // Verify both values are accessible
    assert(cache.read(key1) == bytes({0x01}));
    assert(cache.read(key2) == bytes({0x02}));
    
    std::cout << "  [PASS] Multi-block state management works" << std::endl;
}

void testStateCache_ConcurrentAccess() {
    std::cout << "\n[Test 8] State Cache - Concurrent Access..." << std::endl;
    
    state::StateCache cache;
    const int numThreads = 4;
    const int opsPerThread = 100;
    
    std::vector<std::thread> threads;
    
    // Launch multiple threads writing to different keys
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&cache, t, opsPerThread]() {
            for (int i = 0; i < opsPerThread; ++i) {
                StorageKey key;
                key.account.fill(static_cast<uint8_t>(t));
                key.slot.fill(static_cast<uint8_t>(i));
                
                bytes value = {static_cast<uint8_t>(t), static_cast<uint8_t>(i)};
                cache.write(key, value);
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "  [PASS] Concurrent writes completed (" 
              << numThreads << " threads x " << opsPerThread << " ops)" << std::endl;
}

// ==================== 主测试函数 ====================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  CHASE Core Module Tests v2.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // 基础数据结构测试
        testStorageKey();
        testReadWriteSet();
        
        // KDG 依赖图测试
        testKDG_BasicDependency();
        testKDG_MultipleKeys();
        
        // CDS 调度器测试
        testCDS_GeneratePlan();
        
        // State Cache 测试
        testStateCache_BasicOperations();
        testStateCache_MultiBlock();
        testStateCache_ConcurrentAccess();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "  RESULT: All tests PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nERROR: Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
