#pragma once

#include "framework/protocol/Common.h"
#include "framework/protocol/Block.h"
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace chase {
namespace pipeline {

// 流水线阶段枚举
enum class PipelineStage {
    ORDERING,      // P1: 排序（共识）
    SCHEDULING,    // P2: 调度
    EXECUTION,     // P2: 执行
    COMMIT         // P3: 提交
};

// 流水线窗口
struct PipelineWindow {
    size_t size;          // 当前窗口大小
    size_t maxSize;       // 最大窗口大小
    uint64_t threshold;   // 积压阈值λ
    
    PipelineWindow(size_t max = 10, uint64_t thresh = 1000000)
        : size(1), maxSize(max), threshold(thresh) {}
};

// 流水线状态
struct PipelineStatus {
    size_t orderingQueueSize;
    size_t schedulingQueueSize;
    size_t executionQueueSize;
    size_t commitQueueSize;
    
    uint64_t w2;  // 执行积压（Gas总量）
    uint64_t w3;  // 提交积压（字节数）
};

// BLP流水线控制器
class BlockPipeline {
public:
    using Ptr = std::shared_ptr<BlockPipeline>;
    
    BlockPipeline(int numShards = 4);
    ~BlockPipeline();
    
    // 启动流水线
    void start();
    
    // 停止流水线
    void stop();
    
    // 注册各阶段处理函数
    void setOrderingHandler(std::function<void(Block::Ptr)> handler);
    void setSchedulingHandler(std::function<scheduler::SchedulePlan(Block::Ptr)> handler);
    void setExecutionHandler(std::function<StateSnapshot(Block::Ptr, const scheduler::SchedulePlan&)> handler);
    void setCommitHandler(std::function<void(Block::Ptr, const StateSnapshot&)> handler);
    
    // 推入新proposed区块（由共识层调用）
    void pushBlock(Block::Ptr block);
    
    // 查询流水线状态
    PipelineStatus status() const;
    
private:
    int numShards_;
    std::atomic<bool> running_;
    
    // 各阶段队列
    std::queue<Block::Ptr> orderingQueue_;
    std::queue<std::pair<Block::Ptr, scheduler::SchedulePlan>> schedulingQueue_;
    std::queue<std::pair<Block::Ptr, StateSnapshot>> commitQueue_;
    
    mutable std::mutex queueMutex_;
    std::condition_variable queueCV_;
    
    // AIMD窗口
    PipelineWindow p1Window_;  // Ordering -> Scheduling
    PipelineWindow p2Window_;  // Scheduling/Execution -> Commit
    
    // 积压权重
    uint64_t w2_;  // 执行积压
    uint64_t w3_;  // 提交积压
    
    // 处理函数
    std::function<void(Block::Ptr)> orderingHandler_;
    std::function<scheduler::SchedulePlan(Block::Ptr)> schedulingHandler_;
    std::function<StateSnapshot(Block::Ptr, const scheduler::SchedulePlan&)> executionHandler_;
    std::function<void(Block::Ptr, const StateSnapshot&)> commitHandler_;
    
    // 工作线程
    std::thread orderingThread_;
    std::thread schedulingThread_;
    std::thread executionThread_;
    std::thread commitThread_;
    
    // 线程函数
    void orderingWorker();
    void schedulingWorker();
    void executionWorker();
    void commitWorker();
    
    // AIMD窗口控制
    void updateWindow(PipelineWindow& window, uint64_t backpressure);
    
    // 检查窗口是否已满
    bool windowFull(const PipelineWindow& window, const std::queue<Block::Ptr>& queue) const;
    bool windowFull(const PipelineWindow& window, size_t queueSize) const;
};

} // namespace pipeline
} // namespace chase
