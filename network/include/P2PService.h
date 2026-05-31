#pragma once

#include "framework/protocol/Common.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace chase {
namespace network {

// 消息类型枚举
enum class MessageType {
    TRANSACTION,
    BLOCK,
    CONSENSUS,
    SYNC
};

// P2P 网络服务
class P2PService {
public:
    using MessageHandler = std::function<void(const NodeID&, MessageType, const bytes&)>;
    
    P2PService(const NodeID& nodeId, uint16_t port);
    ~P2PService();
    
    // 启动服务
    void start();
    
    // 停止服务
    void stop();
    
    // 广播消息
    void broadcast(MessageType type, const bytes& payload);
    
    // 发送给特定节点
    void sendTo(const NodeID& nodeId, MessageType type, const bytes& payload);
    
    // 注册消息处理器
    void registerHandler(MessageType type, MessageHandler handler);
    
    // 连接节点
    void connect(const std::string& endpoint);
    
    // 获取已知节点
    std::vector<NodeID> getKnownNodes() const;
    
private:
    NodeID nodeId_;
    uint16_t port_;
    bool running_ = false;
};

} // namespace network
} // namespace chase
