#pragma once

#include "framework/protocol/Common.h"
#include <string>
#include <vector>
#include <memory>

namespace chase {
namespace storage {

// 存储接口
class StorageInterface {
public:
    virtual ~StorageInterface() = default;
    
    // 读取
    virtual bytes get(const std::string& key) = 0;
    
    // 写入
    virtual void put(const std::string& key, const bytes& value) = 0;
    
    // 批量写入
    virtual void writeBatch(const std::vector<std::pair<std::string, bytes>>& kvs) = 0;
    
    // 删除
    virtual void del(const std::string& key) = 0;
};

} // namespace storage
} // namespace chase
