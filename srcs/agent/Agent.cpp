#include "Agent.hpp"
#include "../statistics/Statistics.hpp"
#include "../market/Order.hpp"

Agent::Agent(): asset(100000), previousAsset(100000)
{
}

void Agent::incrementAsset(double value)
{
	asset += value;
}

double Agent::getAsset()
{
    return (asset);
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