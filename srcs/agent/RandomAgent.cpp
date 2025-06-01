#include "RandomAgent.hpp"
#include "../market/Order.hpp"
#include "../market/Market.hpp"
#include <iostream>

RandomAgent::RandomAgent() : Agent()
{
    model = torch::nn::Sequential(
        torch::nn::Linear(2, 64),
        torch::nn::ReLU(),
        torch::nn::Linear(64, 6));
    optimizer = std::make_unique<torch::optim::Adam>(
        model->parameters(),
        torch::optim::AdamOptions(1e-3));
    model->to(device);
}

void RandomAgent::onEpoch(Statistics &statistics, Market &market)
{
    if (statistics.getMidPrices().empty() || statistics.getSpreads().empty())
        return;

    double midPrice = statistics.getMidPrices().back();
    double spread = statistics.getSpreads().back();

    torch::Tensor state = torch::tensor({{midPrice, spread}}, device);

    torch::Tensor q_values = model->forward(state);
    int action = q_values.argmax(1).item<int>();

    Order::Side side;
    double limitPrice = midPrice;

    if (action < 3)
    {
        side = Order::Side::BUY;
        int buyBucket = action; // 0,1,2
        limitPrice = midPrice + (buyBucket - 1) * tickSize;
    }
    else
    {
        side = Order::Side::SELL;
        int sellBucket = action - 3;
        limitPrice = midPrice + (sellBucket - 1) * tickSize;
    }

    std::cout << "[onEpoch] midPrice=" << midPrice << " spread=" << spread << " → action=" << action << (side == Order::Side::BUY ? "(BUY" : "(SELL") << " @ " << limitPrice << ")\n";

    Order order = Order::makeLimit(-1, side, 1, limitPrice, this);
    market.submitOrder(order);

    lastState = state;
    lastAction = action;
}

void RandomAgent::onReward()
{
    if (!lastState.defined())
        return;

    // 1) Compute reward = (current asset) – (previousAsset)
    double currentAsset = asset;
    double prevAsset = previousAsset;
    double reward = currentAsset - prevAsset;

    // std::cout << "[onReward] reward=" << reward  << " | asset was " << prevAsset  << ", now " << currentAsset << "\n";

    // 2) Update previousAsset
    previousAsset = currentAsset;

    // 3) Compute target Q‐values under NoGradGuard
    torch::Tensor target;
    {
        torch::NoGradGuard no_grad;
        // Q(s,·)  shape [1,6]
        torch::Tensor currentQ = model->forward(lastState);
        target = currentQ.clone(); // copy without gradient

        // Q(next,·) — here we naively use lastState as “next state”
        torch::Tensor nextQ = model->forward(lastState).detach();
        torch::Tensor futureQ = std::get<0>(nextQ.max(1)); // shape [1]

        float updatedValue = static_cast<float>(reward + gamma * futureQ.item<float>());
        target[0][lastAction] = updatedValue;
    }

    // 4) Backprop: zero_grad, forward again, compute MSE loss, backward, step
    optimizer->zero_grad();
    torch::Tensor prediction = model->forward(lastState);
    torch::Tensor loss = torch::nn::functional::mse_loss(prediction, target);
    loss.backward();
    optimizer->step();
}
