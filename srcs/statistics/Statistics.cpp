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
    if (orderBook.getAsks().begin() != orderBook.getAsks().end() && orderBook.getBids().begin() != orderBook.getBids().end())
        midPrices.push_back(orderBook.getAsks().begin()->price / orderBook.getBids().begin()->price);
    else
        midPrices.push_back(0.0);
    trim(midPrices);
}

void Statistics::recordBestPrices(OrderBook &orderBook)
{
    if (orderBook.getAsks().begin() != orderBook.getAsks().end())
        bestAsks.push_back(orderBook.getAsks().begin()->price);
    else
        bestAsks.push_back(0);
    trim(bestAsks);
    if (orderBook.getBids().begin() != orderBook.getBids().end())
        bestBids.push_back(orderBook.getBids().begin()->price);
    else
        bestBids.push_back(0);
    trim(bestBids);
}

void Statistics::recordSpread(OrderBook &orderBook)
{
    if (orderBook.getAsks().begin() != orderBook.getAsks().end() && orderBook.getBids().begin() != orderBook.getBids().end())
        spreads.push_back(orderBook.getAsks().begin()->price - orderBook.getBids().begin()->price);
    else
        spreads.push_back(0.0); 
}

void Statistics::record(OrderBook &orderBook)
{
    recordMidPrice(orderBook);
    recordBestPrices(orderBook);
    recordSpread(orderBook);
}