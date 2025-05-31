/**
 * THIS FILE IS NOT YET RELEVANT
 * IT WAS CREATED FOR ARCHITECTURE PURPOSE
 * DO NOT READ IT
 */

#pragma once

#include "Agent.hpp"
#include "../market/Market.hpp"
#include "../market/Order.hpp"
#include <torch/torch.h>


class RandomAgent : public Agent
{
public:
    RandomAgent();

    void onEpoch(Market &market) override;

private:
    torch::nn::Sequential model;
    torch::Device device = torch::kCPU;
};
