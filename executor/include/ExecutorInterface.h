#pragma once

#include "framework/protocol/Common.h"
#include "framework/protocol/Transaction.h"
#include <memory>

namespace chase {
namespace executor {

// 状态访问器接口
class StateAccessor {
public:
    virtual ~StateAccessor() = default;
    virtual bytes getState(const StorageKey& key) = 0;
    virtual void setState(const StorageKey& key, const bytes& value) = 0;
};

// EVM 包装器接口
class EVMWrapper {
public:
    virtual ~EVMWrapper() = default;
    
    // 执行交易
    virtual ExecResult execute(const Transaction& tx, StateAccessor& state) = 0;
    
    // FastPath 模拟（仅追踪 SLOAD/SSTORE）
    virtual SimResult simulateFast(const Transaction& tx, StateAccessor& state) = 0;
    
    // 全量模拟执行
    virtual SimResult simulateFull(const Transaction& tx, StateAccessor& state) = 0;
};

// 单个执行分片
class Executor {
public:
    using Ptr = std::shared_ptr<Executor>;
    
    virtual ~Executor() = default;
    virtual ShardID shardId() const = 0;
    virtual void execute(const Transaction::Ptr& tx, StateAccessor& state) = 0;
};

// 执行管理器
class ExecutorManager {
public:
    virtual ~ExecutorManager() = default;
    virtual void addExecutor(Executor::Ptr executor) = 0;
    virtual Executor::Ptr getExecutor(ShardID shardId) = 0;
};

} // namespace executor
} // namespace chase
