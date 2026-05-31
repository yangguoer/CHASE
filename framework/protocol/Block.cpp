#include "Block.h"

namespace chase {

Hash Block::computeHash() const {
    Hash hash;
    hash.fill(0);
    // 简化的哈希计算
    for (size_t i = 0; i < sizeof(BlockHeight) && i < 32; ++i) {
        hash[i] = static_cast<uint8_t>((header.height >> (i * 8)) & 0xFF);
    }
    return hash;
}

void Block::addTransaction(Transaction::Ptr tx) {
    transactions.push_back(tx);
}

bytes Block::serialize() const {
    bytes data;
    // TODO: 实现序列化
    return data;
}

Block Block::deserialize(const bytes& data) {
    Block block;
    // TODO: 实现反序列化
    return block;
}

} // namespace chase
