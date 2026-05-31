#pragma once

#include "framework/protocol/Common.h"
#include <memory>

namespace chase {

// 状态快照（执行结果）
struct StateSnapshot {
    Hash stateRoot;
    uint64_t gasUsed;
    std::vector<TransactionReceipt> receipts;
    
    StateSnapshot() : gasUsed(0) {}
};

} // namespace chase
