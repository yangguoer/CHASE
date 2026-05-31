#include "framework/protocol/Common.h"
#include "framework/protocol/Block.h"
#include "framework/protocol/Transaction.h"
#include "consensus/include/TuskEngine.h"
#include "txpool/include/TxPool.h"
#include "scheduler/include/CDS.h"
#include "scheduler/include/KDG.h"
#include "pipeline/include/BlockPipeline.h"
#include "state/include/StateCache.h"
#include "network/include/P2PService.h"
#include "glog/logging.h"
#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>

// 全局运行标志
std::atomic<bool> g_running{true};

void signalHandler(int signum) {
    LOG(INFO) << "Received signal " << signum << ", shutting down...";
    g_running = false;
}

int main() {
    // Initialize logging
    google::InitGoogleLogging("CHASE_node");
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;  // INFO
    
    std::cout << "========================================" << std::endl;
    std::cout << "  CHASE Blockchain Node Starting..." << std::endl;
    std::cout << "  Version: 1.0.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 注册信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // 1. 初始化网络服务
        LOG(INFO) << "Initializing P2P network...";
        auto p2pService = std::make_shared<chase::network::P2PService>("node_1", 30303);
        p2pService->start();
        
        // 连接其他节点（示例）
        // p2pService->connect("node_2@192.168.1.2:30303");
        
        // 2. 初始化交易池
        LOG(INFO) << "Initializing transaction pool...";
        auto txPool = std::make_shared<chase::txpool::TxPool>();
        
        // 3. 初始化状态缓存（两层缓存）
        LOG(INFO) << "Initializing state cache (L1 + L2)...";
        auto stateCache = std::make_shared<chase::state::StateCache>();
        
        // 4. 初始化CDS调度器
        LOG(INFO) << "Initializing CDS scheduler...";
        auto cdsScheduler = std::make_shared<chase::scheduler::CDS>(4);  // 4分片
        
        // 5. 初始化共识引擎（Tusk BFT）
        LOG(INFO) << "Initializing Tusk consensus engine...";
        auto consensus = std::make_shared<chase::consensus::TuskEngine>("node_1", 4);
        
        // 6. 初始化BLP流水线
        LOG(INFO) << "Initializing BLP pipeline...";
        auto pipeline = std::make_shared<chase::pipeline::BlockPipeline>(4);
        
        // 注册流水线各阶段处理函数
        
        // Ordering阶段：从共识接收区块
        pipeline->setOrderingHandler([txPool, consensus](chase::Block::Ptr block) {
            LOG(INFO) << "Ordering block at height " << block->header.height;
            // 从交易池获取交易并打包到区块
            auto txs = txPool->fetchBatch(1000, 10000000);
            for (auto& tx : txs) {
                block->addTransaction(tx);
            }
            block->state = chase::BlockState::ORDERED;
        });
        
        // Scheduling阶段：CDS生成调度计划
        pipeline->setSchedulingHandler([cdsScheduler](chase::Block::Ptr block) {
            LOG(INFO) << "Scheduling block at height " << block->header.height;
            
            // 构建KDG
            chase::scheduler::KDG kdg;
            for (const auto& tx : block->transactions) {
                // 通过DPP获取读写集
                for (const auto& key : tx->rwSet.reads) {
                    kdg.addOperation(key, chase::scheduler::KeyOperation(tx->id, chase::OpType::READ));
                }
                for (const auto& [key, value] : tx->rwSet.writes) {
                    kdg.addOperation(key, chase::scheduler::KeyOperation(tx->id, chase::OpType::WRITE, value));
                }
            }
            
            // 生成调度计划
            auto plan = cdsScheduler->generatePlan(block, kdg);
            block->state = chase::BlockState::SCHEDULED;
            
            LOG(INFO) << "Generated schedule plan: " 
                     << plan.cfzChains.size() << " CFZ chains, "
                     << plan.czChains.size() << " CZ chains";
            
            return plan;
        });
        
        // Execution阶段：执行交易
        pipeline->setExecutionHandler([stateCache](chase::Block::Ptr block, 
                                                   const chase::scheduler::SchedulePlan& plan) {
            LOG(INFO) << "Executing block at height " << block->header.height;
            
            // TODO: 实际执行逻辑
            // 1. CFZ无锁并行执行
            // 2. Barrier同步
            // 3. CZ执行并验证
            
            chase::StateSnapshot snapshot;
            block->state = chase::BlockState::EXECUTED;
            
            return snapshot;
        });
        
        // Commit阶段：提交区块
        pipeline->setCommitHandler([stateCache, txPool](chase::Block::Ptr block, 
                                                        const chase::StateSnapshot& snapshot) {
            LOG(INFO) << "Committing block at height " << block->header.height;
            
            //  finalize状态
            stateCache->finalizeBlock(block->header.height);
            
            // 从交易池移除已执行的交易
            std::vector<chase::TxHash> executedTxs;
            for (const auto& tx : block->transactions) {
                executedTxs.push_back(tx->hash);
            }
            txPool->remove(executedTxs);
            
            block->state = chase::BlockState::COMMITTED;
            
            LOG(INFO) << "Block " << block->header.height << " committed successfully";
        });
        
        // 7. 启动共识引擎
        LOG(INFO) << "Starting consensus engine...";
        consensus->onNewBlock([pipeline](chase::Block::Ptr block) {
            pipeline->pushBlock(block);
        });
        consensus->start();
        
        // 8. 启动BLP流水线
        LOG(INFO) << "Starting BLP pipeline...";
        pipeline->start();
        
        std::cout << "========================================" << std::endl;
        std::cout << "  CHASE Node started successfully!" << std::endl;
        std::cout << "  Press Ctrl+C to stop the node" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 主循环：保持节点运行并打印状态
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            auto status = pipeline->status();
            LOG(INFO) << "Pipeline Status:"
                     << " Ordering=" << status.orderingQueueSize
                     << " Scheduling=" << status.schedulingQueueSize
                     << " Commit=" << status.commitQueueSize
                     << " W2(Gas)=" << status.w2
                     << " W3(Bytes)=" << status.w3;
            
            LOG(INFO) << "TxPool pending: " << txPool->pendingCount();
        }
        
        // 优雅关闭
        LOG(INFO) << "Shutting down node...";
        pipeline->stop();
        consensus->stop();
        p2pService->stop();
        
        google::ShutdownGoogleLogging();
        
        std::cout << "CHASE Node stopped." << std::endl;
        
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error starting CHASE Node: " << e.what();
        std::cerr << "Error: " << e.what() << std::endl;
        google::ShutdownGoogleLogging();
        return 1;
    }
    
    return 0;
}
