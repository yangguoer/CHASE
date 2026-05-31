#include "../include/P2PService.h"

namespace chase {
namespace network {

P2PService::P2PService(const NodeID& nodeId, int port)
    : nodeId_(nodeId),
      port_(port),
      running_(false) {}

P2PService::~P2PService() {
    stop();
}

void P2PService::start() {
    running_ = true;
    // TODO: 启动网络监听线程
}

void P2PService::stop() {
    running_ = false;
    // TODO: 停止网络线程
}

void P2PService::broadcast(MessageType type, const bytes& payload) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [nodeId, endpoint] : knownNodes_) {
        if (nodeId != nodeId_) {
            sendTo(nodeId, type, payload);
        }
    }
}

void P2PService::sendTo(const NodeID& nodeId, MessageType type, const bytes& payload) {
    // TODO: 实际发送消息到指定节点
    // 使用asio或libp2p实现
}

void P2PService::registerHandler(MessageType type, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_[type] = std::move(handler);
}

void P2PService::connect(const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 简化：使用endpoint作为nodeId
    NodeID nodeId = endpoint;
    knownNodes_[nodeId] = endpoint;
    
    // TODO: 实际建立网络连接
}

std::vector<NodeID> P2PService::getKnownNodes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<NodeID> nodes;
    for (const auto& [nodeId, endpoint] : knownNodes_) {
        nodes.push_back(nodeId);
    }
    return nodes;
}

} // namespace network
} // namespace chase
