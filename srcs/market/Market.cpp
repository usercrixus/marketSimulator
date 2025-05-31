#include "Market.hpp"

#include "../agent/Agent.hpp"

Market::Market(int epochs)
    : _epochs(epochs),
      _currentEpoch(0),
      _orders(epochs)
{
}

void Market::registerAgent(Agent &agent)
{
    _agents.push_back(&agent);
}

void Market::submitOrder(Order &order)
{
    static int id = 0;

    order.id = id++;
    _orders[_currentEpoch].push_back(order);
}

void Market::run()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    for (_currentEpoch = 0; _currentEpoch < _epochs; ++_currentEpoch)
    {
        // give each agent a chance to look at the book & submit
        for (auto *agent : _agents)
            agent->onEpoch(_statistics);
        // grab this epoch’s bucket
        std::vector<Order> &bucket = _orders[_currentEpoch];
        // shuffle for fairness
        std::shuffle(bucket.begin(), bucket.end(), gen);
        // process
        for (const Order &o : bucket)
            _orderBook.processOrder(o);
        // clear for memory management
        bucket.clear();
        // record a snapshot of the orderbook for statistics
        _orderBook.recordSnapShot();
        // record the statistics
        _statistics.record(_orderBook);
        // reward the agent
        for (auto *agent : _agents)
            agent->onReward();
    }
}

OrderBook Market::getOrderBook() const
{
    return (_orderBook);
}
