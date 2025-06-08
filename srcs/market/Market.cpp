#include "../agent/Agent.hpp"
#include "Market.hpp"

#include <fstream>
#include <algorithm>
#include <random>
#include <iostream>
#include <json.hpp>
using Json = nlohmann::json;

Market::Market(int totalSteps)
    : _totalSteps(totalSteps),
      _currentStep(0),
      _orders(totalSteps)
{
}

Market::~Market()
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
            double qty = trade.at("qty").get<double>();
            tradeVec.push_back(qty);
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
    _orders[_currentStep].push_back(order);
}

void Market::printStatus()
{
    std::cout
        << "Step: " << _currentStep << "/" << _totalSteps
        << " midPrice: " << _statistics.getMidPrices().back()
        << " bestAsk: " << _statistics.getBestAsks().back()
        << " bestBid: " << _statistics.getBestBids().back()
        << " spread: " << _statistics.getSpreads().back()
        << " trades: " << _statistics.getTrades().back()
        << std::endl;
}

void Market::run()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    for (_currentStep = 0; _currentStep < _totalSteps; ++_currentStep)
    {
        printStatus();
        std::vector<Agent *> shuffledAgents = _agents;
        std::shuffle(shuffledAgents.begin(), shuffledAgents.end(), gen);
        for (Agent *agent : shuffledAgents)
        {
            agent->onStep(*this);
            std::vector<Order> ordersToProcess;
            ordersToProcess.swap(_orders[_currentStep]);
            for (Order &order : ordersToProcess)
                _orderBook.processOrder(order);
            _orderBook.recordSnapShot();
            _statistics.record(_orderBook);
        }
    }
    for (Agent *agentPtr : _agents)
        agentPtr->onEpoch(*this);
}

const OrderBook &Market::getOrderBook() const
{
    return (_orderBook);
}

const Statistics &Market::getStatistics() const
{
    return (_statistics);
}
