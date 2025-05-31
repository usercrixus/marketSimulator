#include "Agent.hpp"
#include "../statistics/Statistics.hpp"
#include "../market/Order.hpp"

void Agent::incrementAsset(int value)
{
	asset += value;
}

void Agent::addPendingOrder(const Order *order)
{
    pendingsOrders.push_back(order);
}

void Agent::removePendingOrder(const Order &order)
{
    std::vector<const Order *>::iterator it = pendingsOrders.begin();
    while (it != pendingsOrders.end())
    {
        if ((*it)->id == order.id)
        {
            pendingsOrders.erase(it);
            break;
        }
        it++;
    }
}