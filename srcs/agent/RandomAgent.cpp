/**
 * THIS FILE IS NOT YET RELEVANT
 * IT WAS CREATED FOR ARCHITECTURE PURPOSE
 * DO NOT READ IT
 */

#include "RandomAgent.hpp"
#include "../market/Order.hpp"
#include "../market/Market.hpp"

RandomAgent::RandomAgent()
{
    model = torch::nn::Sequential(
        torch::nn::Linear(2, 64),
        torch::nn::ReLU(),
        torch::nn::Linear(64, 2) // Output: Q-values: [BUY, SELL]
    );
    optimizer = std::make_unique<torch::optim::Adam>(model->parameters(), torch::optim::AdamOptions(1e-3));
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

    Order::Side side = (action == 0) ? Order::Side::BUY : Order::Side::SELL;
    Order order = Order::makeLimit(-1, side, 10, midPrice, this);
    market.submitOrder(order);

    lastState = state;
    lastAction = action;
}

void RandomAgent::onReward()
{
    if (!lastState.defined())
        return;

    double reward = 0.0; // Placeholder — in the future, compute PnL or asset delta

    torch::Tensor newState = lastState; // Ideally, use current market state

    torch::NoGradGuard no_grad;
    torch::Tensor target = model->forward(lastState).clone();
    torch::Tensor futureQ = std::get<0>(model->forward(newState).max(1));
    target[0][lastAction] = reward + gamma * futureQ.item<float>();

    optimizer->zero_grad();
    torch::Tensor prediction = model->forward(lastState);
    torch::Tensor loss = torch::nn::functional::mse_loss(prediction, target);
    loss.backward();
    optimizer->step();
}
