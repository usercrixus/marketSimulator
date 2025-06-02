// --- srcs/agent/Agent.cpp ---
#include "Agent.hpp"
#include "../market/Order.hpp"

Agent::Agent()
    : cash(100000.0),
      inventory(0.0),
      prevNetValue(100000.0),
      isUpdated(false)
{
}

Agent::~Agent() {}

void Agent::addPendingOrder(const Order &order)
{
    pendingsOrders.push_back(&order);
}

void Agent::removePendingOrder(const Order &order)
{
    for (auto it = pendingsOrders.begin(); it != pendingsOrders.end(); ++it)
    {
        if ((*it)->id == order.id)
        {
            pendingsOrders.erase(it);
            break;
        }
    }
}

void Agent::updatePosition(double price, double qty, Order::Side side)
{
    if (side == Order::Side::BUY)
    {
        inventory += qty;
        cash -= price * qty;
    }
    else
    {
        inventory -= qty;
        cash += price * qty;
    }
    isUpdated = true;
}

void Agent::reset()
{
    cash = 100000.0;
    inventory = 0.0;
    prevNetValue = 100000.0;
    isUpdated = false;
    pendingsOrders.clear();
}

double Agent::getNetValue(double midPrice) const
{
    return cash + inventory * midPrice;
}

double Agent::getCash() const
{
    return cash;
}

double Agent::getInventory() const
{
    return inventory;
}