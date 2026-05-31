#include "../include/StorageInterface.h"
// #include <rocksdb/db.h>  // TODO: 集成RocksDB

namespace chase {
namespace storage {

// RocksDB存储实现（简化版，占位符）
class RocksDBStorage : public StorageInterface {
public:
    RocksDBStorage(const std::string& dbPath) {
        // TODO: 打开RocksDB数据库
        // rocksdb::DB::Open(options, dbPath, &db_);
    }
    
    ~RocksDBStorage() override {
        // TODO: 关闭数据库
        // delete db_;
    }
    
    bytes get(const std::string& key) override {
        // TODO: 从RocksDB读取
        return bytes();
    }
    
    void put(const std::string& key, const bytes& value) override {
        // TODO: 写入RocksDB
    }
    
    void writeBatch(const std::vector<std::pair<std::string, bytes>>& kvs) override {
        // TODO: 批量写入
    }
    
    void del(const std::string& key) override {
        // TODO: 删除
    }
    
private:
    // rocksdb::DB* db_ = nullptr;
};

} // namespace storage
} // namespace chase
