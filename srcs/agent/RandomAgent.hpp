#pragma once

#include <torch/torch.h>
#include <memory>
#include "Agent.hpp"

class Order;
class Market;

/**
 * RandomAgent’s network now outputs 6 Q‐values (for 6 discrete actions).
 * Each action maps to a (side, price) pair as described below.
 */
class RandomAgent : public Agent {
public:
    RandomAgent();

    void onEpoch(Statistics &statistics, Market &market) override;
    void onReward() override;

private:
    torch::nn::Sequential model{nullptr};
    std::unique_ptr<torch::optim::Optimizer> optimizer;

    torch::Device device = torch::kCPU;

    torch::Tensor lastState;
    int lastAction = -1;

    double gamma = 0.99;

    // A small price increment (e.g. 1 tick = $0.01). Tweak as needed.
    double tickSize = 0.01;
};
