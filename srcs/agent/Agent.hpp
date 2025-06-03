// --- srcs/agent/Agent.hpp ---
#pragma once

#include <vector>
#include "../market/Order.hpp"

class Statistics;
class Market;

class Agent
{
protected:
    double cash;         // agent’s cash
    double inventory;    // agent’s holdings
    double prevNetValue; // cash + inventory·mid at last reward
    bool isUpdated;      // did the agent trade this run?

    std::vector<const Order *> pendingsOrders;

public:
    Agent();
    virtual ~Agent();

    // Called once per “step” to let the agent submit/modify orders:
    virtual void onStepBegin(Statistics &statistics, Market &market) = 0;

    // Called once per “step” to let the agent submit/modify orders:
    virtual void onEndStep(Statistics &statistics) = 0;

    // Called exactly once, after 100 steps, so that agent can compute reward:
    virtual void onEpoch(Statistics &statistics) = 0;

    // Reset all internal state (cash/inventory/prevNetValue/pending orders)
    virtual void reset();

    void addPendingOrder(const Order &order);
    void removePendingOrder(const Order &order);

    // Called by OrderBook whenever a trade actually executes against this agent:
    void updatePosition(double price, double qty, Order::Side side);

    double getNetValue(double midPrice) const;
    double getCash() const;
    double getInventory() const;
};
