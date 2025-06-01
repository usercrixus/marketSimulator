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

	void onEpoch(Statistics &statistics, Market &market) override;
	void onReward() override;

private:
	ModelLSTM model;
	torch::Device device = torch::kCPU;
    torch::Tensor lastOut;
	std::unique_ptr<torch::optim::Adam> optimizer_;
};
