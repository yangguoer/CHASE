#include "Transaction.h"
#include <cstring>

namespace chase {

void Transaction::computeHash() {
    // 简化的哈希计算，实际应使用Keccak256
    hash.fill(0);
    for (size_t i = 0; i < sizeof(TxID) && i < 32; ++i) {
        hash[i] = static_cast<uint8_t>((id >> (i * 8)) & 0xFF);
    }
}

bool Transaction::verifySignature() const {
    // TODO: 集成libsecp256k1进行真正的签名验证
    return true;
}

bytes Transaction::serialize() const {
    bytes data;
    // TODO: 实现RLP序列化
    return data;
}

Transaction Transaction::deserialize(const bytes& data) {
    Transaction tx;
    // TODO: 实现RLP反序列化
    return tx;
}

} // namespace chase
