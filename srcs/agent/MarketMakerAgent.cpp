// --- srcs/agent/MarketMakerAgent.cpp ---
#include <torch/torch.h>
#include "MarketMakerAgent.hpp"
#include "../statistics/Statistics.hpp"
#include "../market/Market.hpp"
#include <algorithm>

MarketMakerAgent::MarketMakerAgent()
    : Agent(), model(4, 2)
{
    model.to(device, torch::kFloat32);
    optimizer_ = std::make_unique<torch::optim::Adam>(model.parameters(), 1e-4);
}

at::Tensor MarketMakerAgent::buildInputTensor(Statistics &statistics)
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

void MarketMakerAgent::manageOrder(Market &market, double quantity, double bidPrice, double askPrice)
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

void MarketMakerAgent::onEpoch(Statistics &statistics, Market &market)
{

    auto input = buildInputTensor(statistics);
    auto out = model.forward(input);
    logitsHistory.push_back(out);

    // 3) Convert logits → continuous bid/ask in [min(mid), max(mid)]
    float bidNorm = out[0][0].item<float>();
    float askNorm = out[0][1].item<float>();

    double mn_mid = *std::min_element(statistics.getMidPrices().begin(), statistics.getMidPrices().end());
    double mx_mid = *std::max_element(statistics.getMidPrices().begin(), statistics.getMidPrices().end());
    float range_mid = static_cast<float>(mx_mid - mn_mid);

    double bidPrice = ((bidNorm + 1.0f) / 2.0f) * range_mid + static_cast<float>(mn_mid);
    double askPrice = ((askNorm + 1.0f) / 2.0f) * range_mid + static_cast<float>(mn_mid);

    // 4) Decide quantity randomly (just as before)
    static thread_local std::mt19937 gen{
        static_cast<unsigned long>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count())};
    std::uniform_real_distribution<float> dist(8.0f, 16.0f);
    double quantity = dist(gen);
    // 5) For each existing pending order, modify; otherwise submit a new post‐only limit
    manageOrder(market, quantity, bidPrice, askPrice);
}

void MarketMakerAgent::onReward(Statistics &statistics)
{
    double reward;
    if (!isUpdated)
        reward = -1000; // If the agent never actually placed anything in any of the 100 steps,
    else
    {
        // 1) Compute final mid‐price (from the last snapshot)
        const auto &mids = statistics.getMidPrices();
        double finalMid = mids.back();
        // 2) Compute new net‐worth = cash + inventory * finalMid
        double newNet = cash + inventory * finalMid;
        // 3) Reward is the difference over the last epoch:
        reward = newNet - prevNetValue;
    }

    // 4) Now build a single “epoch‐return” loss that sums up each step’s log‐proba
    //    Here, outHistory[t] is shape [1,2] raw logits. We want to push them “up”
    //    by +reward if, say, we interpret all logits as “we wanted wide bid/ask spreads.”
    //
    //    Concretely, if we simply do  –R * (sum of logitsHistory[t].sum()), that
    //    means “encourage each output to be large (in whichever direction the LSTM chose),
    //    weighted by the same final reward.”
    //
    //    In practice you might do something more sophisticated—e.g.
    //    1) convert logits→probabilities via softmax, pick an “action index” (0=bid,1=ask),
    //    2) store the chosen index at each step in chosenActions[t], and then
    //    3) do –R * ∑_{t} log_prob(chosenActions[t])
    //
    //    But if you just want to replicate “–R * out.sum()” at every step, you can loop below.

    torch::Tensor totalLoss = torch::zeros({1}, torch::kFloat32);
    for (auto &stepLogits : logitsHistory)
    {
        // stepLogits is a [1×2] tensor
        totalLoss += -static_cast<float>(reward) * stepLogits.sum();
    }

    // 5) Backpropagate all at once:
    optimizer_->zero_grad();
    totalLoss.backward();
    optimizer_->step();

    // 6) Store newNet for next episode, and mark isUpdated=false so we don’t double‐update:
    isUpdated = false;

    // 7) Clear history now that we’ve used it:
    logitsHistory.clear();
    chosenActions.clear();
}
