// --- srcs/agent/MarketMakerAgent.hpp ---
#pragma once

#include <torch/torch.h>
#include <memory>
#include "Agent.hpp"
#include "models/ModelLSTM.hpp"

class Order;
class Market;

class MarketMakerAgent : public Agent {
public:
    MarketMakerAgent();

    at::Tensor buildInputTensor(Statistics &statistics);

    void manageOrder(Market &market, double quantity, double bidPrice, double askPrice);

    // Called every sub‐step (100 times per mini‐market)
    void onEpoch(Statistics &statistics, Market &market) override;

    // Called once, after the 100 sub‐steps finish
    void onReward(Statistics &statistics) override;

    // (Optional) if you need to clear buffers between mini-markets
    void reset() override {
        Agent::reset();
        logitsHistory.clear();
        // If you were recording chosen actions, clear those too:
        chosenActions.clear();
    }

private:
    ModelLSTM model;
    torch::Device device = torch::kCPU;

    std::unique_ptr<torch::optim::Adam> optimizer_;

    // Instead of a single `lastOut`, we keep a history of every sub-step’s logits:
    std::vector<torch::Tensor> logitsHistory;

    // If you do a discrete action (e.g. choose buy vs. sell vs. no‐op), you might also record:
    std::vector<int> chosenActions;
};
