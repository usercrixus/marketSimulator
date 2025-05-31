#include "Statistics.hpp"
#include "../market/OrderBook.hpp"

Statistics::Statistics(/* args */)
{
}

Statistics::~Statistics()
{
}

void Statistics::trim(std::deque<double> &data)
{
    if (data.size() > maxSize)
        data.pop_front();
}

void Statistics::recordMidPrice(OrderBook &orderBook)
{
    if (!orderBook.getAsks().empty() && !orderBook.getBids().empty())
        midPrices.push_back(orderBook.getAsks().begin()->first /  std::prev(orderBook.getBids().end())->first);
    else
        midPrices.push_back(0.0);
    trim(midPrices);
}

void Statistics::recordBestPrices(OrderBook &orderBook)
{
    if (!orderBook.getAsks().empty())
        bestAsks.push_back(orderBook.getAsks().begin()->first);
    else
        bestAsks.push_back(0);
    trim(bestAsks);
    if (!orderBook.getBids().empty())
        bestBids.push_back(std::prev(orderBook.getBids().end())->first);
    else
        bestBids.push_back(0);
    trim(bestBids);
}

void Statistics::recordSpread(OrderBook &orderBook)
{
    if (!orderBook.getAsks().empty() && !orderBook.getBids().empty())
        spreads.push_back(orderBook.getAsks().begin()->first - std::prev(orderBook.getBids().end())->first);
    else
        spreads.push_back(0.0);
    trim(spreads);
}

void Statistics::record(OrderBook &orderBook)
{
    recordMidPrice(orderBook);
    recordBestPrices(orderBook);
    recordSpread(orderBook);
}