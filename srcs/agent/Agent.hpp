#pragma once

#include <vector>
#include "../market/Order.hpp"

class Statistics;
class Market;

class Agent
{
public:
    Agent();
    virtual ~Agent();

    /**
     * Called once per “step” to let the agent submit/modify orders
     */
    virtual void onStep(Market &market) = 0;
    /**
     *  Called exactly once, after all steps, so that agent can compute reward
     */
    virtual void onEpoch(Market &market) = 0;
    /**
     * Reset all internal state (cash/inventory/prevNetValue/pending orders)
     */
    virtual void reset();
    /**
     * Add a pending order to the agent pendingsOrder vector (keep a trace from the market)
     */
    void addPendingOrder(const Order &order);
    /**
     * Remove a pending order.
     * When an order is fully filled, or after a cancel order, the order is removed from the market
     * and should also be removed from then pendingOrder vector of the agent
     */
    void removePendingOrder(const Order &order);
    /**
     * Called by OrderBook whenever a trade actually executes against this agent
     */
    void updatePosition(double price, double qty, Order::Side side);

    double getNetValue(double midPrice) const;
    double getCash() const;
    double getInventory() const;

protected:
    double cash;                               // agent’s cash
    double inventory;                          // agent’s holdings
    bool isUpdated;                            // did the agent trade this run?
    std::vector<const Order *> pendingsOrders; // orders registred to the order book
};
