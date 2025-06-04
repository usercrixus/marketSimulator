#pragma once

#include <torch/torch.h>
#include <memory>
#include "Agent.hpp"
#include "models/ModelLSTM.hpp"

class Order;
class Market;

/**
 * RandomAgent’s network now outputs 6 Q‐values (for 6 discrete actions).
 * Each action maps to a (side, price) pair as described below.
 */
class TakerAgent : public Agent
{
public:
	TakerAgent();
    // build input 1d tensor from statistics deque
    at::Tensor buildInputTensor(const Statistics &statistics);
	void onStepBegin(Market &market) override;
    // Called every sub‐step (100 times per mini‐market)
    void onEpoch(const Statistics &statistics) override;
    void reward(const Statistics &statistics);

private:
	ModelLSTM model;
    torch::Tensor lastOut;
	std::unique_ptr<torch::optim::Adam> optimizer_;
    torch::Device device;
};
