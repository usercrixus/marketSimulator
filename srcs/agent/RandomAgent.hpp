/**
 * THIS FILE IS NOT YET RELEVANT
 * IT WAS CREATED FOR ARCHITECTURE PURPOSE
 * DO NOT READ IT
 */

#pragma once

#include <torch/torch.h>
#include <memory>
#include "Agent.hpp"

class Order;
class Market;

class RandomAgent : public Agent {
public:
    RandomAgent();

    void onEpoch(Statistics &statistics, Market &market);
    void onReward() override;

private:
    torch::nn::Sequential model{nullptr};
    std::unique_ptr<torch::optim::Optimizer> optimizer;

    torch::Device device = torch::kCPU;

    torch::Tensor lastState;
    int lastAction = -1;

    double gamma = 0.99;
};
