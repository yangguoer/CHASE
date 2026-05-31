#pragma once

#include "framework/protocol/Common.h"
#include "framework/protocol/Block.h"
#include "framework/protocol/Transaction.h"
#include <functional>
#include <memory>
#include <string>

namespace chase {
namespace consensus {

// Tusk BFT 共识引擎
class TuskEngine {
public:
    using Ptr = std::shared_ptr<TuskEngine>;
    
    TuskEngine(const NodeID& nodeId, int committeeSize = 4);
    ~TuskEngine();
    
    // 启动共识引擎
    void start();
    
    // 停止共识引擎
    void stop();
    
    // 提交要打包的交易（由TxPool调用）
    void submitTransaction(Transaction::Ptr tx);
    
    // 注册新区块回调（供BLP流水线使用）
    void onNewBlock(std::function<void(Block::Ptr)> callback);
    
    // 当前节点ID
    NodeID nodeId() const { return nodeId_; }
    
    // 委员会大小
    int committeeSize() const { return committeeSize_; }
    
private:
    NodeID nodeId_;
    int committeeSize_;
    bool running_;
    
    // 新区块回调
    std::function<void(Block::Ptr)> newBlockCallback_;
    
    // TODO: Narwhal mempool
    // TODO: Tusk DAG-BFT
    
    // 工作线程
    std::thread consensusThread_;
    
    void consensusWorker();
};

} // namespace consensus
} // namespace chase
