#include "Market.hpp"
#include "../agent/Agent.hpp"
#include "fstream"
#include <json.hpp>
using Json = nlohmann::json;

Market::Market(int epochs)
    : _epochs(epochs),
      _currentEpoch(0),
      _orders(epochs)
{
}

bool Market::initMarket()
{
    std::ifstream inFile("./utils/scrapper/binance_snapshots_clean.json");
    if (!inFile.is_open())
        return (std::cerr << "ERROR: could not open binance_snapshots_clean.json\n", false);

    Json json;
    inFile >> json;
    inFile.close();

    for (const auto &stepElement : json)
    {
        std::map<double, std::list<Order>> bidsMap;
        for (const auto &bidsLevel : stepElement.at("bids"))
        {
            double price = bidsLevel.at(0).get<double>();
            double qty = bidsLevel.at(1).get<double>();
            Order order = Order::makeLimit(-1, Order::Side::BUY, qty, price, nullptr);
            bidsMap[price].push_back(std::move(order));
        }

        std::map<double, std::list<Order>> asksMap;
        for (const auto &asksLevel : stepElement.at("asks"))
        {
            double price = asksLevel.at(0).get<double>();
            double qty = asksLevel.at(1).get<double>();
            Order o = Order::makeLimit(-1, Order::Side::SELL, qty, price, nullptr);
            asksMap[price].push_back(std::move(o));
        }

        std::vector<double> tradeVec;
        for (const auto &trade : stepElement.at("trades"))
        {
            double price = trade.at("price").get<double>();
            double qty = trade.at("qty").get<double>();
            tradeVec.push_back(price * qty);
        }

        _orderBook.forceSnapshot(bidsMap, asksMap, tradeVec);
    }
    _statistics.initStats(_orderBook);
    return (true);
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
        std::cout << "Epoch: " << _currentEpoch << "/" << _epochs << " midPrice: " << _statistics.getMidPrices().back() << std::endl;
        // give each agent a chance to look at the book & submit
        for (auto *agent : _agents)
            agent->onEpoch(_statistics, *this);
        // grab this epoch’s bucket
        std::vector<Order> &bucket = _orders[_currentEpoch];
        // shuffle for fairness
        std::shuffle(bucket.begin(), bucket.end(), gen);
        // process
        for (Order &order : bucket)
            _orderBook.processOrder(order);
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

const OrderBook &Market::getOrderBook() const
{
    return (_orderBook);
}
