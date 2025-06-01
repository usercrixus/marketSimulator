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
    if (!orderBook.getAsksSnapShots().empty() && !orderBook.getBidsSnapShots().empty())
        midPrices.push_back((orderBook.getAsksSnapShots().rbegin()->begin()->first + orderBook.getBidsSnapShots().rbegin()->rbegin()->first) / 2);
    else
        midPrices.push_back(0.0);
    trim(midPrices);
}

void Statistics::recordBestPrices(OrderBook &orderBook)
{
    if (!orderBook.getAsksSnapShots().empty())
        bestAsks.push_back(orderBook.getAsksSnapShots().rbegin()->begin()->first);
    else
        bestAsks.push_back(0);
    trim(bestAsks);
    if (!orderBook.getBidsSnapShots().empty())
        bestBids.push_back(orderBook.getBidsSnapShots().rbegin()->rbegin()->first);
    else
        bestBids.push_back(0);
    trim(bestBids);
}

void Statistics::recordSpread(OrderBook &orderBook)
{
    if (!orderBook.getAsksSnapShots().empty() && !orderBook.getBidsSnapShots().empty())
        spreads.push_back(orderBook.getAsksSnapShots().rbegin()->begin()->first - orderBook.getBidsSnapShots().rbegin()->rbegin()->first);
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

void Statistics::initStats(OrderBook &orderBook)
{
    auto asksSnapShots = orderBook.getAsksSnapShots().rbegin();
    auto bidsSnapShots = orderBook.getBidsSnapShots().rbegin();

    while (asksSnapShots != orderBook.getAsksSnapShots().rend() && bidsSnapShots != orderBook.getBidsSnapShots().rend())
    {
        auto asksSnapshot = asksSnapShots->begin();
        auto bidsSnapshot = bidsSnapShots->rbegin();

        if (asksSnapshot != asksSnapShots->end() && bidsSnapshot != bidsSnapShots->rend())
        {
            double bestAsk = asksSnapshot->first;
            double bestBid = bidsSnapshot->first;
            midPrices.push_back((bestAsk + bestBid) / 2.0);
            spreads.push_back(bestAsk - bestBid);
            bestAsks.push_back(bestAsk);
            bestBids.push_back(bestBid);
        }
        else
        {
            midPrices.push_back(0.0);
            spreads.push_back(0.0);
            bestAsks.push_back(0.0);
            bestBids.push_back(0.0);
        }
        trim(midPrices);
        trim(spreads);
        trim(bestAsks);
        trim(bestBids);

        ++asksSnapShots;
        ++bidsSnapShots;
    }
}
