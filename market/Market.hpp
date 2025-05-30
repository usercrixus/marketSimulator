#pragma once

#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include "OrderBook.hpp"
#include "Order.hpp"

class Agent;

class Market
{
public:
    Market(int epochs);
    /**
     * Register an agent that will be notified once per epoch.
     */
    void registerAgent(Agent &agent);
    /**
     * Agents (or external code) call this to queue an order for a future epoch
     */
    void submitOrder(const Order &order);
    /**
     * Run all epochs in sequence: call agents → shuffle → match → clear
     */
    void run();
    OrderBook getOrderBook() const;

private:
    int _epochs; // total number of epoch
    int _currentEpoch; // current epoch 
    std::vector<std::vector<Order>> _orders; // per-epoch order buckets
    OrderBook _orderBook; // the order book of the market
    std::vector<Agent *> _agents; // subscribed agents
};
