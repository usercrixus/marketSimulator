#include <torch/torch.h>
#include "UntrainedTakerAgent.hpp"
#include "../statistics/Statistics.hpp"
#include "../market/Market.hpp"
#include <algorithm>
#include <random>

UntrainedTakerAgent::UntrainedTakerAgent()
    : Agent(), model(4, 3), device(torch::cuda::is_available() ? torch::kCUDA : torch::kCPU)
{
    model.to(device, torch::kFloat32);
    optimizer_ = std::make_unique<torch::optim::Adam>(model.parameters(), 1e-4);
}

void UntrainedTakerAgent::onStepBegin(Market &market)
{
    const Statistics &statistics = market.getStatistics();

    const auto &midDeque = statistics.getMidPrices();
    const auto &bidDeque = statistics.getBestBids();
    const auto &askDeque = statistics.getBestAsks();
    const auto &spdDeque = statistics.getSpreads();

    if (midDeque.size() < 100 || bidDeque.size() < 100 || askDeque.size() < 100 || spdDeque.size() < 100)
        return;

    // Normalize each deque to [-1,1]
    std::vector<double> normMid = Statistics::normalizeDeque(midDeque);
    std::vector<double> normBids = Statistics::normalizeDeque(bidDeque);
    std::vector<double> normAsks = Statistics::normalizeDeque(askDeque);
    std::vector<double> normSpd = Statistics::normalizeDeque(spdDeque);

    std::vector<float> flat_data;
    flat_data.reserve(100 * 4);
    for (size_t t = 0; t < 100; ++t)
    {
        flat_data.push_back(static_cast<float>(normMid[t]));
        flat_data.push_back(static_cast<float>(normBids[t]));
        flat_data.push_back(static_cast<float>(normAsks[t]));
        flat_data.push_back(static_cast<float>(normSpd[t]));
    }

    auto seq100x4 = torch::tensor(
                        flat_data,
                        torch::TensorOptions().dtype(torch::kFloat32).device(device))
                        .view({100, 4});
    auto input = seq100x4.unsqueeze(0); // shape [1, 100, 4]

    auto logits = model.forward(input);
    lastOut = logits.cpu();
    isUpdated = true;

    auto probs = torch::softmax(logits, 1);
    int action_idx = std::get<1>(probs.max(1)).item<int>();

    // random quantity
    static thread_local std::mt19937 gen{
        static_cast<unsigned long>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count())};
    std::uniform_real_distribution<float> dist(0.1f, 0.3f);
    double quantity = dist(gen);

    if (action_idx == 0)
    {
        // immediately buy at market
        Order buyOrder = Order::makeMarket(-1, Order::Side::BUY, quantity, this);
        market.submitOrder(buyOrder);
    }
    else if (action_idx == 1)
    {
        // immediately sell at market
        Order sellOrder = Order::makeMarket(-1, Order::Side::SELL, quantity, this);
        market.submitOrder(sellOrder);
    }
}

void UntrainedTakerAgent::onEpoch(const Statistics &statistics)
{
    (void)statistics;
    // do nothing
}