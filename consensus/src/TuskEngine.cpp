#include "../include/TuskEngine.h"

namespace chase {
namespace consensus {

TuskEngine::TuskEngine(const NodeID& nodeId, int committeeSize)
    : nodeId_(nodeId),
      committeeSize_(committeeSize),
      running_(false) {}

TuskEngine::~TuskEngine() {
    stop();
}

void TuskEngine::start() {
    running_ = true;
    consensusThread_ = std::thread(&TuskEngine::consensusWorker, this);
}

void TuskEngine::stop() {
    running_ = false;
    if (consensusThread_.joinable()) {
        consensusThread_.join();
    }
}

void TuskEngine::submitTransaction(Transaction::Ptr tx) {
    // TODO: 提交到Narwhal mempool
}

void TuskEngine::onNewBlock(std::function<void(Block::Ptr)> callback) {
    newBlockCallback_ = std::move(callback);
}

void TuskEngine::consensusWorker() {
    // TODO: 实现Tusk BFT共识逻辑
    // 1. Narwhal: 打包交易，广播证书
    // 2. Tusk: 在DAG中选取锚点，确定区块全序
    
    while (running_) {
        // 简化：定期生成区块
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // 创建新区块
        auto block = std::make_shared<Block>();
        block->header.height = 0;  // TODO
        block->state = BlockState::PROPOSED;
        
        // 调用回调
        if (newBlockCallback_) {
            newBlockCallback_(block);
        }
    }
}

} // namespace consensus
} // namespace chase
