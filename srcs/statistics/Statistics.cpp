#include "Statistics.hpp"
#include "../market/OrderBook.hpp"

#include <algorithm>

Statistics::Statistics()
{
}

Statistics::~Statistics()
{
}

void Statistics::trim(std::deque<double> &data)
{
    while (data.size() > SNAPSHOT_LEN)
        data.pop_front();
}

void Statistics::recordMidPrice(OrderBook &orderBook)
{
    const auto &asksVec = orderBook.getAsksSnapShots();
    const auto &bidsVec = orderBook.getBidsSnapShots();
    if (!asksVec.empty() && !bidsVec.empty() && !asksVec.back().empty() && !bidsVec.back().empty())
    {
        const auto &asksMap = asksVec.back();
        const auto &bidsMap = bidsVec.back();
        double bestAsk = asksMap.begin()->first;
        double bestBid = bidsMap.rbegin()->first;
        _midPrices.push_back((bestAsk + bestBid) / 2.0);
    }
    else
        _midPrices.push_back(_midPrices.empty() ? 0.0 : _midPrices.back());
    trim(_midPrices);
}

void Statistics::recordBestPrices(OrderBook &orderBook)
{
    const auto &asksVec = orderBook.getAsksSnapShots();
    if (!asksVec.empty() && !asksVec.back().empty())
        _bestAsks.push_back(asksVec.back().begin()->first);
    else
        _bestAsks.push_back(_bestAsks.empty() ? 0.0 : _bestAsks.back());
    trim(_bestAsks);

    const auto &bidsVec = orderBook.getBidsSnapShots();
    if (!bidsVec.empty() && !bidsVec.back().empty())
        _bestBids.push_back(bidsVec.back().rbegin()->first);
    else
        _bestBids.push_back(_bestBids.empty() ? 0.0 : _bestBids.back());
    trim(_bestBids);
}

void Statistics::recordSpread(OrderBook &orderBook)
{
    const auto &asksVec = orderBook.getAsksSnapShots();
    const auto &bidsVec = orderBook.getBidsSnapShots();
    if (!asksVec.empty() && !bidsVec.empty() && !asksVec.back().empty() && !bidsVec.back().empty())
    {
        double bestAsk = asksVec.back().begin()->first;
        double bestBid = bidsVec.back().rbegin()->first;
        _spreads.push_back(bestAsk - bestBid);
    }
    else
        _spreads.push_back(_spreads.empty() ? 0.0 : _spreads.back());
    trim(_spreads);
}

void Statistics::recordTrade(OrderBook &orderBook)
{
    auto trades = orderBook.getTradesSnapShots();
    if (!trades.empty() && !trades.back().empty())
        _trades.push_back(trades.back()[0]);
    else
        _trades.push_back(_trades.empty() ? 0.0 : _trades.back());
    trim(_trades);
}

void Statistics::record(OrderBook &orderBook)
{
    recordMidPrice(orderBook);
    recordBestPrices(orderBook);
    recordSpread(orderBook);
    recordTrade(orderBook);
}

void Statistics::initStats(OrderBook &orderBook)
{
    auto asksSnapShots = orderBook.getAsksSnapShots().rbegin();
    auto bidsSnapShots = orderBook.getBidsSnapShots().rbegin();
    auto tradeSnapshots = orderBook.getTradesSnapShots().rbegin();

    // Walk until any one of the three runs out
    while (asksSnapShots != orderBook.getAsksSnapShots().rend() &&
           bidsSnapShots != orderBook.getBidsSnapShots().rend() &&
           tradeSnapshots != orderBook.getTradesSnapShots().rend())
    {
        // Extract best ask / best bid exactly as before
        auto asksSnapshot = asksSnapShots->begin();
        auto bidsSnapshot = bidsSnapShots->rbegin();
        if (asksSnapshot != asksSnapShots->end() &&
            bidsSnapshot != bidsSnapShots->rend())
        {
            double bestAsk = asksSnapshot->first;
            double bestBid = bidsSnapshot->first;
            _midPrices.push_back((bestAsk + bestBid) / 2.0);
            _spreads.push_back(bestAsk - bestBid);
            _bestAsks.push_back(bestAsk);
            _bestBids.push_back(bestBid);
        }
        else
        {
            _midPrices.push_back(0.0);
            _spreads.push_back(0.0);
            _bestAsks.push_back(0.0);
            _bestBids.push_back(0.0);
        }
        trim(_midPrices);
        trim(_spreads);
        trim(_bestAsks);
        trim(_bestBids);

        // Now also record trades for this snapshot:
        const std::vector<double> &tradeVec = *tradeSnapshots;
        double sumTrades = 0.0;
        for (double q : tradeVec)
        {
            sumTrades += q;
        }
        _trades.push_back(sumTrades);
        trim(_trades);

        ++asksSnapShots;
        ++bidsSnapShots;
        ++tradeSnapshots;
    }
}

