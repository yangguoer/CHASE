#pragma once

#include "framework/protocol/Common.h"
#include <vector>
#include <string>
#include <utility>

namespace chase {
namespace crypto {

// SHA-256 哈希类
class Sha256 {
public:
    static Hash hash(const bytes& data);
    static Hash hash(const uint8_t* data, size_t len);
};

// Keccak-256 哈希类（以太坊兼容）
class Keccak256 {
public:
    static Hash hash(const bytes& data);
    static Hash hash(const uint8_t* data, size_t len);
};

// Secp256k1 签名类
class Secp256k1 {
public:
    // 验证签名
    static bool verify(const Hash& message, 
                      const Signature& signature,
                      const PublicKey& publicKey);
    
    // 从签名恢复公钥
    static PublicKey recoverPublicKey(const Hash& message,
                                     const Signature& signature);
    
    // 生成密钥对
    static std::pair<bytes, PublicKey> generateKeyPair();
};

} // namespace crypto
} // namespace chase
