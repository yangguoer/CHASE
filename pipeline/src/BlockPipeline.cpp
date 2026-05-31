#include "../include/BlockPipeline.h"
#include <chrono>

namespace chase {
namespace pipeline {

BlockPipeline::BlockPipeline(int numShards)
    : numShards_(numShards),
      running_(false),
      w2_(0),
      w3_(0) {}

BlockPipeline::~BlockPipeline() {
    stop();
}

void BlockPipeline::start() {
    running_ = true;
    
    orderingThread_ = std::thread(&BlockPipeline::orderingWorker, this);
    schedulingThread_ = std::thread(&BlockPipeline::schedulingWorker, this);
    executionThread_ = std::thread(&BlockPipeline::executionWorker, this);
    commitThread_ = std::thread(&BlockPipeline::commitWorker, this);
}

void BlockPipeline::stop() {
    running_ = false;
    queueCV_.notify_all();
    
    if (orderingThread_.joinable()) orderingThread_.join();
    if (schedulingThread_.joinable()) schedulingThread_.join();
    if (executionThread_.joinable()) executionThread_.join();
    if (commitThread_.joinable()) commitThread_.join();
}

void BlockPipeline::setOrderingHandler(std::function<void(Block::Ptr)> handler) {
    orderingHandler_ = std::move(handler);
}

void BlockPipeline::setSchedulingHandler(std::function<scheduler::SchedulePlan(Block::Ptr)> handler) {
    schedulingHandler_ = std::move(handler);
}

void BlockPipeline::setExecutionHandler(std::function<StateSnapshot(Block::Ptr, const scheduler::SchedulePlan&)> handler) {
    executionHandler_ = std::move(handler);
}

void BlockPipeline::setCommitHandler(std::function<void(Block::Ptr, const StateSnapshot&)> handler) {
    commitHandler_ = std::move(handler);
}

void BlockPipeline::pushBlock(Block::Ptr block) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    orderingQueue_.push(std::move(block));
    queueCV_.notify_one();
}

PipelineStatus BlockPipeline::status() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    PipelineStatus status;
    status.orderingQueueSize = orderingQueue_.size();
    status.schedulingQueueSize = schedulingQueue_.size();
    status.executionQueueSize = 0;  // TODO
    status.commitQueueSize = commitQueue_.size();
    status.w2 = w2_;
    status.w3 = w3_;
    
    return status;
}

void BlockPipeline::orderingWorker() {
    while (running_) {
        Block::Ptr block;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] {
                return !running_ || !orderingQueue_.empty();
            });
            
            if (!running_ && orderingQueue_.empty()) break;
            
            block = orderingQueue_.front();
            orderingQueue_.pop();
        }
        
        // 调用排序处理器（共识层）
        if (orderingHandler_) {
            orderingHandler_(block);
        }
        
        // 检查P1窗口
        while (windowFull(p1Window_, schedulingQueue_.size()) && running_) {
            updateWindow(p1Window_, w2_);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        // 推入调度队列
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            schedulingQueue_.push({block, scheduler::SchedulePlan()});
        }
        queueCV_.notify_one();
    }
}

void BlockPipeline::schedulingWorker() {
    while (running_) {
        std::pair<Block::Ptr, scheduler::SchedulePlan> item;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] {
                return !running_ || !schedulingQueue_.empty();
            });
            
            if (!running_ && schedulingQueue_.empty()) break;
            
            item = schedulingQueue_.front();
            schedulingQueue_.pop();
        }
        
        auto& [block, plan] = item;
        
        // 调用调度处理器（CDS）
        if (schedulingHandler_) {
            plan = schedulingHandler_(block);
        }
        
        // 调用执行处理器
        StateSnapshot snapshot;
        if (executionHandler_) {
            snapshot = executionHandler_(block, plan);
        }
        
        // 更新执行积压
        w2_ = 0;  // TODO: 计算实际Gas积压
        
        // 检查P2窗口
        while (windowFull(p2Window_, w3_) && running_) {
            updateWindow(p2Window_, w3_);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        // 推入提交队列
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            commitQueue_.push({block, snapshot});
        }
        queueCV_.notify_one();
    }
}

void BlockPipeline::executionWorker() {
    // 简化：执行与调度和并
    // 实际实现中，执行可能在独立线程池中并行运行
}

void BlockPipeline::commitWorker() {
    while (running_) {
        std::pair<Block::Ptr, StateSnapshot> item;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] {
                return !running_ || !commitQueue_.empty();
            });
            
            if (!running_ && commitQueue_.empty()) break;
            
            item = commitQueue_.front();
            commitQueue_.pop();
        }
        
        auto& [block, snapshot] = item;
        
        // 调用提交处理器
        if (commitHandler_) {
            commitHandler_(block, snapshot);
        }
        
        // 更新提交积压
        w3_ = 0;  // TODO: 计算实际字节积压
    }
}

void BlockPipeline::updateWindow(PipelineWindow& window, uint64_t backpressure) {
    if (backpressure == 0) {
        // 指数增长
        window.size = std::min(window.size * 2, window.maxSize);
    } else if (backpressure < window.threshold) {
        // 线性增长
        window.size = std::min(window.size + 1, window.maxSize);
    } else {
        // 乘性减少
        window.size = std::max(window.size / 2, static_cast<size_t>(1));
    }
}

bool BlockPipeline::windowFull(const PipelineWindow& window, 
                               const std::queue<Block::Ptr>& queue) const {
    return queue.size() >= window.size;
}

bool BlockPipeline::windowFull(const PipelineWindow& window, 
                               size_t queueSize) const {
    return queueSize >= window.size;
}

} // namespace pipeline
} // namespace chase
