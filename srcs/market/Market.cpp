// --- srcs/market/Market.cpp ---
#include "Market.hpp"
#include "../agent/Agent.hpp"
#include <fstream>
#include <algorithm>
#include <random>
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
    {
        std::cerr << "ERROR: could not open binance_snapshots_clean.json\n";
        return false;
    }

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
    return true;
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

void Market::printStatus()
{
    double lastMid = _statistics.getMidPrices().empty() ? 0.0 : _statistics.getMidPrices().back();
    double lastAsk = _statistics.getBestAsks().empty() ? 0.0 : _statistics.getBestAsks().back();
    double lastBid = _statistics.getBestBids().empty() ? 0.0 : _statistics.getBestBids().back();
    double lastSpd = _statistics.getSpreads().empty() ? 0.0 : _statistics.getSpreads().back();

    std::cout
        << "Epoch: " << _currentEpoch << "/" << _epochs
        << "  midPrice: " << lastMid
        << "  bestAsk: " << lastAsk
        << "  bestBid: " << lastBid
        << "  spread: " << lastSpd
        << std::endl;
}

void Market::run()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    for (_currentEpoch = 0; _currentEpoch < _epochs; ++_currentEpoch)
    {
        printStatus();

        std::vector<Agent *> shuffledAgents = _agents;
        std::shuffle(shuffledAgents.begin(), shuffledAgents.end(), gen);
        for (Agent *agent : shuffledAgents)
        {
            agent->onStepBegin(_statistics, *this);
            std::vector<Order> &orders = _orders[_currentEpoch];
            if (!orders.empty())
            {
                for (Order &order : orders)
                    _orderBook.processOrder(order);
                orders.clear();

                _orderBook.recordSnapShot();
                _statistics.record(_orderBook);
            }
            agent->onEndStep(_statistics);
        }
    }
    for (Agent *agentPtr : _agents)
        agentPtr->onEpoch(_statistics);
}

const OrderBook &Market::getOrderBook() const
{
    return _orderBook;
}
