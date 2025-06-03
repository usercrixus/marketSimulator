#include <torch/torch.h>
#include "TakerAgent.hpp"
#include "../statistics/Statistics.hpp"
#include "../market/Market.hpp"
#include <algorithm>

TakerAgent::TakerAgent()
    : Agent(), model(4, 3) // outputSize = 3 now
{
    model.to(device, torch::kFloat32);
    optimizer_ = std::make_unique<torch::optim::Adam>(model.parameters(), 1e-4);
}

void TakerAgent::onStep(Statistics &statistics, Market &market)
{
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

void TakerAgent::onEndStep(Statistics &statistics)
{
    if (!isUpdated)
        return;

    double finalMid = statistics.getMidPrices().back();
    double newNet = getNetValue(finalMid);
    double reward = newNet - prevNetValue;
    prevNetValue = newNet;
    isUpdated = false;

    auto rewardTensor = torch::tensor(
        static_cast<float>(reward),
        torch::TensorOptions().dtype(torch::kFloat32));
    auto log_probs = torch::log_softmax(lastOut, /*dim=*/1);
    auto chosen_index_tensor = std::get<1>(log_probs.exp().max(/*dim=*/1));
    int chosen_idx = chosen_index_tensor.item<int>();
    auto chosen_logprob = log_probs[0][chosen_idx];
    auto loss = -chosen_logprob * rewardTensor;

    optimizer_->zero_grad();
    loss.backward();
    optimizer_->step();
}

void TakerAgent::onEpoch(Statistics &statistics)
{
    (void)statistics;
    // do nothing
}