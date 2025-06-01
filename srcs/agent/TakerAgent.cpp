// TakerAgent.cpp
#include <torch/torch.h>
#include "TakerAgent.hpp"
#include "../statistics/Statistics.hpp"
#include "../market/Market.hpp"
#include <algorithm>

TakerAgent::TakerAgent()
    : Agent(), model(4, 2)
{
    model.to(device, torch::kFloat32);
    optimizer_ = std::make_unique<torch::optim::Adam>(model.parameters(), 1e-4);
}

void TakerAgent::onEpoch(Statistics &statistics, Market &market)
{
    const auto &midDeque = statistics.getMidPrices();
    const auto &bidDeque = statistics.getBestBids();
    const auto &askDeque = statistics.getBestAsks();
    const auto &spdDeque = statistics.getSpreads();

    if (midDeque.size() < 100 || bidDeque.size() < 100 || askDeque.size() < 100 || spdDeque.size() < 100)
        return;

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

    auto seq100x4 = torch::tensor(flat_data, torch::TensorOptions().dtype(torch::kFloat32)).view({100, 4});
    auto input = seq100x4.unsqueeze(0);

    auto out = model.forward(input);
    lastOut = out;
    isUpdated = true;

    float bidNorm = out[0][0].item<float>();
    float askNorm = out[0][1].item<float>();

    auto mn_mid = *std::min_element(midDeque.begin(), midDeque.end());
    auto mx_mid = *std::max_element(midDeque.begin(), midDeque.end());
    float range_mid = static_cast<float>(mx_mid - mn_mid);

    double bidPrice = ((bidNorm + 1.0f) / 2.0f) * range_mid + static_cast<float>(mn_mid);
    double askPrice = ((askNorm + 1.0f) / 2.0f) * range_mid + static_cast<float>(mn_mid);

    static thread_local std::mt19937 gen{
        static_cast<unsigned long>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count())};
    std::uniform_real_distribution<float> dist(0.1f, 0.3f);
    double quantity = dist(gen);

    Order buyOrder = Order::makeLimit(-1, Order::Side::BUY, quantity, bidPrice, this);
    Order sellOrder = Order::makeLimit(-1, Order::Side::SELL, quantity, askPrice, this);
    market.submitOrder(buyOrder);
    market.submitOrder(sellOrder);
    previousAsset = asset;
}

void TakerAgent::onReward()
{
    if (!isUpdated)
        return;

    double reward = asset - previousAsset;
    auto rewardTensor = torch::tensor(
        static_cast<float>(reward),
        torch::TensorOptions().dtype(torch::kFloat32));
    auto loss = -rewardTensor * lastOut.sum();
    optimizer_->zero_grad();
    loss.backward();
    optimizer_->step();
    isUpdated = false;
}
