// --- srcs/agent/UntrainedMarketMakerAgent.hpp ---
#pragma once

#include <torch/torch.h>
#include <memory>
#include "Agent.hpp"
#include "models/ModelLSTM.hpp"

class Order;
class Market;

class UntrainedMarketMakerAgent : public Agent {
public:
    UntrainedMarketMakerAgent();
    // build input 1d tensor from statistics deque
    at::Tensor buildInputTensor(Statistics &statistics);
    // manage market maker limit order (modify or set)
    void manageOrder(Market &market, double quantity, double bidPrice, double askPrice);
    // Called every sub‐step (100 times per mini‐market)
    void onStep(Statistics &statistics, Market &market) override;
    // Called every sub‐step (100 times per mini‐market)
    void onEpoch(Statistics &statistics) override;
    // Called every sub‐step (100 times per mini‐market)
    void onEndStep(Statistics &statistics) override;

private:
    ModelLSTM model;
    torch::Device device = torch::kCPU;
    std::unique_ptr<torch::optim::Adam> optimizer_;
    std::vector<torch::Tensor> outputHistory;
};
