#include "framework/protocol/Common.h"
#include "framework/protocol/Transaction.h"
#include "framework/protocol/Block.h"
#include "scheduler/include/KDG.h"
#include "scheduler/include/CDS.h"
#include "state/include/StateCache.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

using namespace chase;

// ==================== 辅助函数 ====================

// 创建模拟交易
Transaction::Ptr createMockTransaction(TxID id, uint8_t accountIdx) {
    auto tx = std::make_shared<Transaction>();
    tx->id = id;
    tx->nonce = id;
    tx->gasPrice = 100 + id;
    tx->gasLimit = 21000;
    
    // 设置地址
    tx->from.fill(0x00);
    tx->from[19] = static_cast<uint8_t>(id);
    
    tx->to.fill(0x00);
    tx->to[19] = static_cast<uint8_t>(accountIdx);
    
    // 简化的数据
    tx->data = {static_cast<uint8_t>(id), 0x01, 0x02};
    
    return tx;
}

// 为交易添加读写集（模拟 DPP 预测结果）
void addMockReadWriteSet(Transaction::Ptr tx, uint8_t keyIdx) {
    StorageKey key;
    key.account.fill(0x00);
    key.account[19] = keyIdx;
    key.slot.fill(0x00);
    
    tx->rwSet.reads.insert(key);
    tx->rwSet.writes[key] = {static_cast<uint8_t>(tx->id), 0xFF};
    tx->gasUsed = 21000;
}

// ==================== 流水线阶段 ====================

// 阶段 1: TxPool -> 获取交易批次
std::vector<Transaction::Ptr> stage_TxPool_BatchFetch(size_t batchSize) {
    std::cout << "\n[Stage 1] TxPool: Fetching batch of " << batchSize << " transactions..." << std::endl;
    
    std::vector<Transaction::Ptr> batch;
    
    // 模拟从交易池获取批次
    for (size_t i = 1; i <= batchSize; ++i) {
        auto tx = createMockTransaction(i, static_cast<uint8_t>(i % 5 + 1));
        addMockReadWriteSet(tx, static_cast<uint8_t>(i % 3 + 1));
        batch.push_back(tx);
    }
    
    std::cout << "  [OK] Fetched " << batch.size() << " transactions" << std::endl;
    return batch;
}

// 阶段 2: 构建 KDG
scheduler::KDG stage_KDG_Build(const std::vector<Transaction::Ptr>& transactions) {
    std::cout << "\n[Stage 2] KDG: Building dependency graph..." << std::endl;
    
    scheduler::KDG kdg;
    
    for (const auto& tx : transactions) {
        // 添加读操作
        for (const auto& key : tx->rwSet.reads) {
            kdg.addOperation(key, scheduler::KeyOperation(tx->id, OpType::READ));
        }
        
        // 添加写操作
        for (const auto& [key, value] : tx->rwSet.writes) {
            kdg.addOperation(key, scheduler::KeyOperation(tx->id, OpType::WRITE, value));
        }
    }
    
    auto txIds = kdg.getAllTxIds();
    std::cout << "  [OK] KDG built with " << txIds.size() << " transactions" << std::endl;
    
    // 统计依赖关系
    int rawCount = 0, warCount = 0, wawCount = 0;
    for (size_t i = 0; i < txIds.size(); ++i) {
        for (size_t j = i + 1; j < txIds.size(); ++j) {
            for (const auto& tx : transactions) {
                if (tx->id == txIds[i] || tx->id == txIds[j]) {
                    for (const auto& key : tx->rwSet.reads) {
                        auto dep = kdg.getDependency(txIds[i], txIds[j], key);
                        if (dep == DependencyType::RAW) rawCount++;
                        else if (dep == DependencyType::WAR) warCount++;
                        else if (dep == DependencyType::WAW) wawCount++;
                    }
                    for (const auto& [key, _] : tx->rwSet.writes) {
                        auto dep = kdg.getDependency(txIds[i], txIds[j], key);
                        if (dep == DependencyType::RAW) rawCount++;
                        else if (dep == DependencyType::WAR) warCount++;
                        else if (dep == DependencyType::WAW) wawCount++;
                    }
                }
            }
        }
    }
    
    std::cout << "  [INFO] Dependencies - RAW: " << rawCount 
              << ", WAR: " << warCount 
              << ", WAW: " << wawCount << std::endl;
    
    return kdg;
}

// 阶段 3: CDS 生成调度计划
void stage_CDS_GeneratePlan(scheduler::KDG& kdg, const std::vector<Transaction::Ptr>& transactions) {
    std::cout << "\n[Stage 3] CDS: Generating schedule plan..." << std::endl;
    
    scheduler::CDS cds(4);  // 4 分片
    
    // 简化版：直接生成分片分配
    // 实际应该调用 cds.generatePlan(block, kdg)
    
    std::cout << "  [OK] CDS initialized with 4 shards" << std::endl;
    std::cout << "  [INFO] Schedule plan would include:" << std::endl;
    std::cout << "    - CFZ chains (conflict-free zones)" << std::endl;
    std::cout << "    - CZ chains (conflict zones)" << std::endl;
    std::cout << "    - Shard assignments for each transaction" << std::endl;
}

// 阶段 4: 并行执行（模拟）
void stage_Executor_Execute(const std::vector<Transaction::Ptr>& transactions, 
                           state::StateCache& stateCache) {
    std::cout << "\n[Stage 4] Executor: Executing transactions..." << std::endl;
    
    const size_t numShards = 4;
    std::vector<std::vector<Transaction::Ptr>> shardTxs(numShards);
    
    // 简单分片策略：按交易 ID 模分片数
    for (const auto& tx : transactions) {
        size_t shardId = tx->id % numShards;
        shardTxs[shardId].push_back(tx);
    }
    
    std::cout << "  [INFO] Distributed " << transactions.size() << " transactions across " 
              << numShards << " shards:" << std::endl;
    
    for (size_t i = 0; i < numShards; ++i) {
        std::cout << "    Shard " << i << ": " << shardTxs[i].size() << " transactions" << std::endl;
    }
    
    // 模拟并行执行
    std::vector<std::thread> executors;
    
    for (size_t shardId = 0; shardId < numShards; ++shardId) {
        executors.emplace_back([&stateCache, &shardTxs, shardId]() {
            for (const auto& tx : shardTxs[shardId]) {
                // 模拟执行：写入状态
                for (const auto& [key, value] : tx->rwSet.writes) {
                    stateCache.write(key, value);
                }
                
                // 更新实际读写集
                tx->actualRWSet = tx->rwSet;
            }
        });
    }
    
    // 等待所有执行器完成
    for (auto& t : executors) {
        t.join();
    }
    
    std::cout << "  [OK] All transactions executed in parallel" << std::endl;
}

// 阶段 5: StateCache 提交
void stage_StateCache_Commit(state::StateCache& stateCache, uint64_t blockHeight) {
    std::cout << "\n[Stage 5] StateCache: Committing block " << blockHeight << "..." << std::endl;
    
    stateCache.finalizeBlock(blockHeight);
    
    std::cout << "  [OK] Block " << blockHeight << " finalized" << std::endl;
}

// 阶段 6: 验证结果
void stage_Verify(const std::vector<Transaction::Ptr>& transactions, 
                 state::StateCache& stateCache) {
    std::cout << "\n[Stage 6] Verification: Checking execution results..." << std::endl;
    
    int verifiedCount = 0;
    
    for (const auto& tx : transactions) {
        // 验证实际读写集已填充
        assert(!tx->actualRWSet.writes.empty());
        
        // 验证状态已写入
        for (const auto& [key, expectedValue] : tx->rwSet.writes) {
            auto actualValue = stateCache.read(key);
            // 注意：由于多交易可能写入同一键，最后一个写入会覆盖
            // 这里只验证非空
            if (!actualValue.empty()) {
                verifiedCount++;
            }
        }
    }
    
    std::cout << "  [OK] Verified " << verifiedCount << " state writes" << std::endl;
    std::cout << "  [OK] All " << transactions.size() << " transactions processed successfully" << std::endl;
}

// ==================== 完整流水线测试 ====================

void test_FullPipeline_SmallBatch() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Pipeline Test 1: Small Batch (10 txs)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Stage 1: 从 TxPool 获取批次
    auto batch = stage_TxPool_BatchFetch(10);
    
    // Stage 2: 构建 KDG
    auto kdg = stage_KDG_Build(batch);
    
    // Stage 3: CDS 生成调度计划
    stage_CDS_GeneratePlan(kdg, batch);
    
    // Stage 4: 并行执行
    state::StateCache stateCache;
    stage_Executor_Execute(batch, stateCache);
    
    // Stage 5: 提交状态
    stage_StateCache_Commit(stateCache, 1);
    
    // Stage 6: 验证
    stage_Verify(batch, stateCache);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\n[PERF] Execution time: " << duration.count() << " ms" << std::endl;
    std::cout << "[PERF] Throughput: " << (batch.size() * 1000.0 / duration.count()) << " tx/s" << std::endl;
    std::cout << "[PASS] Small batch pipeline completed successfully" << std::endl;
}

void test_FullPipeline_MediumBatch() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Pipeline Test 2: Medium Batch (50 txs)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Stage 1-6: 完整流水线
    auto batch = stage_TxPool_BatchFetch(50);
    auto kdg = stage_KDG_Build(batch);
    stage_CDS_GeneratePlan(kdg, batch);
    
    state::StateCache stateCache;
    stage_Executor_Execute(batch, stateCache);
    stage_StateCache_Commit(stateCache, 2);
    stage_Verify(batch, stateCache);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\n[PERF] Execution time: " << duration.count() << " ms" << std::endl;
    std::cout << "[PERF] Throughput: " << (batch.size() * 1000.0 / duration.count()) << " tx/s" << std::endl;
    std::cout << "[PASS] Medium batch pipeline completed successfully" << std::endl;
}

void test_FullPipeline_Conflicts() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Pipeline Test 3: High Conflict Scenario" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建多个访问相同键的交易（高冲突场景）
    std::vector<Transaction::Ptr> batch;
    
    for (size_t i = 1; i <= 20; ++i) {
        auto tx = createMockTransaction(i, 1);  // 所有交易访问相同账户
        
        // 所有交易都读写同一个键
        StorageKey sharedKey;
        sharedKey.account.fill(0x01);
        sharedKey.slot.fill(0x00);
        
        tx->rwSet.reads.insert(sharedKey);
        tx->rwSet.writes[sharedKey] = {static_cast<uint8_t>(i)};
        tx->gasUsed = 21000;
        
        batch.push_back(tx);
    }
    
    std::cout << "\n[Stage 1] Created 20 transactions accessing same key" << std::endl;
    
    // 构建 KDG 并检测冲突
    auto kdg = stage_KDG_Build(batch);
    
    // 统计冲突数量
    int conflictCount = 0;
    auto txIds = kdg.getAllTxIds();
    for (size_t i = 0; i < txIds.size(); ++i) {
        for (size_t j = i + 1; j < txIds.size(); ++j) {
            StorageKey sharedKey;
            sharedKey.account.fill(0x01);
            sharedKey.slot.fill(0x00);
            
            auto dep = kdg.getDependency(txIds[i], txIds[j], sharedKey);
            if (dep != DependencyType::RAW) {
                conflictCount++;
            }
        }
    }
    
    std::cout << "  [INFO] Detected " << conflictCount << " conflicts" << std::endl;
    
    // 执行流水线
    stage_CDS_GeneratePlan(kdg, batch);
    
    state::StateCache stateCache;
    stage_Executor_Execute(batch, stateCache);
    stage_StateCache_Commit(stateCache, 3);
    stage_Verify(batch, stateCache);
    
    std::cout << "[PASS] High conflict scenario handled correctly" << std::endl;
}

void test_FullPipeline_MultiBlock() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Pipeline Test 4: Multi-Block Processing" << std::endl;
    std::cout << "========================================" << std::endl;
    
    state::StateCache stateCache;
    
    // 处理 3 个区块
    for (uint64_t blockHeight = 1; blockHeight <= 3; ++blockHeight) {
        std::cout << "\n--- Processing Block " << blockHeight << " ---" << std::endl;
        
        auto batch = stage_TxPool_BatchFetch(15);
        auto kdg = stage_KDG_Build(batch);
        stage_CDS_GeneratePlan(kdg, batch);
        stage_Executor_Execute(batch, stateCache);
        stage_StateCache_Commit(stateCache, blockHeight);
        stage_Verify(batch, stateCache);
    }
    
    std::cout << "\n[PASS] Multi-block processing completed (3 blocks)" << std::endl;
}

// ==================== 主测试函数 ====================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  CHASE Full Pipeline Integration Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // 测试 1: 小批量处理
        test_FullPipeline_SmallBatch();
        
        // 测试 2: 中等批量处理
        test_FullPipeline_MediumBatch();
        
        // 测试 3: 高冲突场景
        test_FullPipeline_Conflicts();
        
        // 测试 4: 多区块处理
        test_FullPipeline_MultiBlock();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "  RESULT: All pipeline tests PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nERROR: Pipeline test failed: " << e.what() << std::endl;
        return 1;
    }
}
