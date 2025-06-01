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

void Statistics::recordFromSnapshot(
    const std::map<double,std::list<Order>> &bidsSnapshot,
    const std::map<double,std::list<Order>> &asksSnapshot)
{
    // mid = (bestAsk + bestBid)/2 or bestAsk/bestBid or however you like
    if (!asksSnapshot.empty() && !bidsSnapshot.empty()) {
        double bestAsk = asksSnapshot.begin()->first;
        double bestBid = std::prev(bidsSnapshot.end())->first;
        midPrices.push_back((bestAsk + bestBid)/2.0);
        spreads.push_back(bestAsk - bestBid);
        bestAsks.push_back(bestAsk);
        bestBids.push_back(bestBid);
    }
    else {
        midPrices.push_back(0.0);
        spreads.push_back(0.0);
        bestAsks.push_back(0.0);
        bestBids.push_back(0.0);
    }
    trim(midPrices);
    trim(spreads);
    trim(bestAsks);
    trim(bestBids);
}
