// --- srcs/agent/UntrainedMarketMakerAgent.cpp ---
#include <torch/torch.h>
#include "UntrainedMarketMakerAgent.hpp"
#include "../statistics/Statistics.hpp"
#include "../market/Market.hpp"
#include <algorithm>

UntrainedMarketMakerAgent::UntrainedMarketMakerAgent()
    : Agent(), model(4, 2)
{
    model.to(device, torch::kFloat32);
    optimizer_ = std::make_unique<torch::optim::Adam>(model.parameters(), 1e-4);
}

at::Tensor UntrainedMarketMakerAgent::buildInputTensor(Statistics &statistics)
{
    const auto &midDeque = statistics.getMidPrices();
    const auto &bidDeque = statistics.getBestBids();
    const auto &askDeque = statistics.getBestAsks();
    const auto &spdDeque = statistics.getSpreads();

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

    auto seq100x4 = torch::tensor(flat_data, torch::TensorOptions().dtype(torch::kFloat32).device(device)).view({100, 4});
    auto input = seq100x4.unsqueeze(0);
    return input;
}

void UntrainedMarketMakerAgent::manageOrder(Market &market, double quantity, double bidPrice, double askPrice)
{
    bool shouldBuy = true;
    bool shouldSell = true;

    for (auto *order : pendingsOrders)
    {
        if (order->side == Order::Side::BUY)
        {
            Order m = Order::makeModify(-1, order->id, quantity, bidPrice, this);
            market.submitOrder(m);
            shouldBuy = false;
        }
        else
        {
            Order m = Order::makeModify(-1, order->id, quantity, askPrice, this);
            market.submitOrder(m);
            shouldSell = false;
        }
    }

    if (shouldBuy)
    {
        Order m = Order::makePostOnlyLimit(-1, Order::Side::BUY, quantity, bidPrice, this);
        market.submitOrder(m);
    }
    if (shouldSell)
    {
        Order m = Order::makePostOnlyLimit(-1, Order::Side::SELL, quantity, askPrice, this);
        market.submitOrder(m);
    }
}

void UntrainedMarketMakerAgent::onStepBegin(Statistics &statistics, Market &market)
{
    auto input = buildInputTensor(statistics);
    auto out = model.forward(input);
    outputHistory.push_back(out);

    // 3) Convert logits → continuous bid/ask in [min(mid), max(mid)]
    float bidNorm = out[0][0].item<float>();
    float askNorm = out[0][1].item<float>();
    double mn_mid = *std::min_element(statistics.getMidPrices().begin(), statistics.getMidPrices().end());
    double mx_mid = *std::max_element(statistics.getMidPrices().begin(), statistics.getMidPrices().end());
    double bidPrice = Statistics::unNormalize(bidNorm, mx_mid, mn_mid);
    double askPrice = Statistics::unNormalize(askNorm, mx_mid, mn_mid);

    // 4) Decide quantity randomly (just as before)
    std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(8.0f, 16.0f);
    double quantity = dist(gen);
    // 5) For each existing pending order, modify; otherwise submit a new post‐only limit
    manageOrder(market, quantity, bidPrice, askPrice);
}

void UntrainedMarketMakerAgent::onEpoch(Statistics &statistics)
{
    (void)statistics;
    outputHistory.clear();
}
void UntrainedMarketMakerAgent::onEndStep(Statistics &statistics)
{
    (void)statistics;
}