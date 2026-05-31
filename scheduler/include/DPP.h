#pragma once

#include "framework/protocol/Common.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace chase {
namespace scheduler {

// 分支历史记录
struct BranchHistory {
    std::vector<bool> directions;  // L次历史跳转方向
    static constexpr size_t HISTORY_LENGTH = 16;
    
    void record(bool direction) {
        directions.push_back(direction);
        if (directions.size() > HISTORY_LENGTH) {
            directions.erase(directions.begin());
        }
    }
};

// 块内预测结果
struct InBlockPredictions {
    std::vector<bool> predictions;  // 前序K个预测结果
    static constexpr size_t LOOKBACK = 8;
    
    void record(bool prediction) {
        predictions.push_back(prediction);
        if (predictions.size() > LOOKBACK) {
            predictions.erase(predictions.begin());
        }
    }
};

// 感知机分支预测器
class BranchPredictor {
public:
    std::vector<int32_t> weights;    // w_i: 历史跳转方向权重
    std::vector<int32_t> pweights;   // w_j': 块内前序预测权重
    int32_t bias;                    // w_0
    
    static constexpr size_t WEIGHT_COUNT = 16;
    static constexpr int32_t THRESHOLD_DEFAULT = 10;
    
    BranchPredictor();
    
    // 预测函数: f = w0 + sum(wi * hi) + sum(wj' * pj)
    float predict(const BranchHistory& history, 
                  const InBlockPredictions& prevPredictions) const;
    
    // 在线更新（感知机更新规则）
    void update(bool actualOutcome, float prediction);
    
    // 区块粒度原子提交
    void commitBlock();
    
    // 重置为下一区块
    void resetForNextBlock();
    
private:
    std::vector<int32_t> pendingWeights;   // 待提交的权重更新
    std::vector<int32_t> pendingPWeights;
    int32_t pendingBias;
};

// 多版本状态层
class MultiVersionState {
public:
    // 获取交易Ti执行时，key的可见状态（来自T1...Ti-1）
    bytes getState(TxID txId, const StorageKey& key) const;
    
    // 记录交易Ti对key的写入（仅临时，用于后续交易预测）
    void setTemp(TxID txId, const StorageKey& key, const bytes& value);
    
    // 清除已提交交易的状态
    void clearCommitted(BlockHeight height);
    
private:
    // txId -> (key -> value)
    std::unordered_map<TxID, 
        std::unordered_map<StorageKey, bytes, StorageKey::Hasher>> tempStates;
};

// DPP主类
class DPP {
public:
    DPP();
    
    // 执行DPP预测，获取读写集
    SimResult predict(const Transaction& tx, 
                     MultiVersionState& mvState,
                     BranchPredictor& predictor);
    
    // FastPath：仅追踪SLOAD/SSTORE
    SimResult fastPath(const Transaction& tx,
                      BranchPredictor& predictor);
    
    // SimulateFull：基于多版本状态层的精确模拟
    SimResult simulateFull(const Transaction& tx,
                          MultiVersionState& mvState);
    
    // 识别SV-branch（通过栈追踪）
    bool identifySVBranch(const Transaction& tx);
    
private:
    BranchPredictor predictor_;
    MultiVersionState mvState_;
    
    // 自适应阈值
    int32_t threshold_;
    void adjustThreshold(float confidence);
};

} // namespace scheduler
} // namespace chase
