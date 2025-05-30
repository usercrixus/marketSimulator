#include "Market.hpp"

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

void Market::submitOrder(const Order &order)
{
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
            agent->onEpoch(*this);
        // grab this epoch’s bucket
        std::vector<Order> &bucket = _orders[_currentEpoch];
        // shuffle for fairness
        std::shuffle(bucket.begin(), bucket.end(), gen);
        // process
        for (const Order &o : bucket)
            _orderBook.processOrder(o);
        // clear
        bucket.clear();
    }
}

OrderBook Market::getOrderBook() const
{
    return (_orderBook);
}
