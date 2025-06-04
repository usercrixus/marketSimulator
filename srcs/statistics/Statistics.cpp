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

    double newMid = 0.0;

    if (!asksVec.empty() && !bidsVec.empty())
    {
        const auto &asksMap = asksVec.back();
        const auto &bidsMap = bidsVec.back();
        if (!asksMap.empty() && !bidsMap.empty())
        {
            double bestAsk = asksMap.begin()->first;
            double bestBid = bidsMap.rbegin()->first;
            newMid = (bestAsk + bestBid) / 2.0;
        }
        else if (!asksMap.empty())
            newMid = asksMap.begin()->first;
        else if (!bidsMap.empty())
            newMid = bidsMap.rbegin()->first;
        else
            newMid = _midPrices.empty() ? 0.0 : _midPrices.back();
    }
    else if (!asksVec.empty())
    {
        const auto &asksMap = asksVec.back();
        newMid = asksMap.empty() ? (_midPrices.empty() ? 0.0 : _midPrices.back()) : asksMap.begin()->first;
    }
    else if (!bidsVec.empty())
    {
        const auto &bidsMap = bidsVec.back();
        newMid = bidsMap.empty() ? (_midPrices.empty() ? 0.0 : _midPrices.back()) : bidsMap.rbegin()->first;
    }
    else
        newMid = _midPrices.empty() ? 0.0 : _midPrices.back();
    _midPrices.push_back(newMid);
    trim(_midPrices);
}

void Statistics::recordBestPrices(OrderBook &orderBook)
{
    const auto &asksVec = orderBook.getAsksSnapShots();
    if (!asksVec.empty() && !asksVec.back().empty())
        _bestAsks.push_back(asksVec.back().begin()->first);
    else
        _bestAsks.push_back(_bestAsks.back());
    trim(_bestAsks);

    const auto &bidsVec = orderBook.getBidsSnapShots();
    if (!bidsVec.empty() && !bidsVec.back().empty())
        _bestBids.push_back(bidsVec.back().rbegin()->first);
    else
        _bestBids.push_back(_bestBids.back());
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
        _spreads.push_back(_spreads.back());
    trim(_spreads);
}

void Statistics::recordTrade(OrderBook &orderBook)
{
    auto trades = orderBook.getTradesSnapShots();
    if (!trades.empty() && !trades.back().empty())
        _trades.push_back(trades.back()[0]);
    trim(_trades);
}

void Statistics::record(OrderBook &orderBook)
{
    recordMidPrice(orderBook);
    recordBestPrices(orderBook);
    recordSpread(orderBook);
    recordTrade(orderBook);
}

void Statistics::initStats(OrderBook &orderBook) {
    auto asksSnapShots  = orderBook.getAsksSnapShots().rbegin();
    auto bidsSnapShots  = orderBook.getBidsSnapShots().rbegin();
    auto tradeSnapshots = orderBook.getTradesSnapShots().rbegin(); // assumes OrderBook exposes this

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
        } else {
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
        for (double q : tradeVec) {
            sumTrades += q;
        }
        _trades.push_back(sumTrades);
        trim(_trades);

        ++asksSnapShots;
        ++bidsSnapShots;
        ++tradeSnapshots;
    }
}

std::vector<double> Statistics::normalizeDeque(const std::deque<double> &data)
{
    std::vector<double> result;
    result.reserve(data.size());

    if (data.empty())
        return result;

    double mn = *std::min_element(data.begin(), data.end());
    double mx = *std::max_element(data.begin(), data.end());

    if (mn == mx)
    {
        result.assign(data.size(), 0.0);
        return result;
    }

    double range = mx - mn;
    for (double x : data)
    {
        double scaled = 2.0 * (x - mn) / range - 1.0;
        result.push_back(scaled);
    }
    return result;
}

double Statistics::unNormalize(double value, double max, double min)
{
    double mid = max - min;
    double result = ((value + 1.0) / 2.0) * mid + min;
    return result;
}
