#pragma once

#include <vector>
#include "Order.hpp"
#include "OrderBook.hpp"
#include "../statistics/Statistics.hpp"

class Agent;

class Market
{
public:
    Market(int epochs);
    ~Market();
    /**
     * init the market history with real data
     * this should be called with at least 100 step history
     */
    bool initMarket();
    /**
     * Register an agent that will be notified multiple times:
     * At the begin of step, when it is his turn to do some action on the market
     * At the end of epoch, as he can have a chance to look at that his action, and have a feed back on it
     */
    void registerAgent(Agent &agent);
    /**
     * Agents (or external code) call this to queue an order for a future epoch
     */
    void submitOrder(Order &order);
    /**
     * print the status of the market (current step and statistic for this step)
     */
    void printStatus();
    /**
     * Run all epochs in sequence. Manage the whole market.
     */
    void run();

    const OrderBook &getOrderBook() const;
    const Statistics &getStatistics() const;

private:
    int _totalSteps;                         // total number of epoch
    int _currentStep;                        // current epoch
    std::vector<std::vector<Order>> _orders; // per-epoch order buckets
    std::vector<Agent *> _agents;            // subscribed agents
    OrderBook _orderBook;                    // the order book of the market
    Statistics _statistics;                  // the market statistics
};
