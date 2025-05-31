/**
 * THIS FILE IS NOT YET RELEVANT
 * IT WAS CREATED FOR ARCHITECTURE PURPOSE
 * DO NOT READ IT
 */


#include "RandomAgent.hpp"
#include <iostream>

RandomAgent::RandomAgent() {
    model = torch::nn::Sequential(
        torch::nn::Linear(2, 64),
        torch::nn::ReLU(),
        torch::nn::Linear(64, 2) // output: [buy_prob, sell_prob]
    );
    model->to(device);
}

void RandomAgent::onEpoch(Market &market) {
    // Basic input: [mid_price, spread]
    auto orderBook = market.getOrderBook();
    if (orderBook.getBids().empty() || orderBook.getAsks().empty())
        return;

    double bestBid = orderBook.getBids().begin()->price;
    double bestAsk = orderBook.getAsks().begin()->price;
    double midPrice = (bestBid + bestAsk) / 2.0;
    double spread = bestAsk - bestBid;

    torch::Tensor input = torch::tensor({{midPrice, spread}}, device);
    torch::Tensor output = model->forward(input);

    int action = output.argmax(1).item<int>();
    Order::Side side = (action == 0) ? Order::Side::BUY : Order::Side::SELL;

    Order order = Order::makeLimit(-1, side, 10, midPrice, this);
    market.submitOrder(order);
}
