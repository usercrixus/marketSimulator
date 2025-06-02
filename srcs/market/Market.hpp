#pragma once

#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include "Order.hpp"
#include "OrderBook.hpp"
#include "../statistics/Statistics.hpp"

class Agent;

class Market
{
public:
    Market(int epochs);
    /**
     * init the market history with real data
     */
    bool initMarket();
    /**
     * Register an agent that will be notified once per epoch.
     */
    void registerAgent(Agent &agent);
    /**
     * Agents (or external code) call this to queue an order for a future epoch
     */
    void submitOrder(Order &order);
    /**
     * 
     */
    void printStatus();
    /**
     * Run all epochs in sequence: call agents → shuffle → match → clear
     */
    void run();
    const OrderBook &getOrderBook() const;

private:
    int _epochs;                             // total number of epoch
    int _currentEpoch;                       // current epoch
    std::vector<std::vector<Order>> _orders; // per-epoch order buckets
    OrderBook _orderBook;                    // the order book of the market
    std::vector<Agent *> _agents;            // subscribed agents
    Statistics _statistics;                  // the market statistics
};
