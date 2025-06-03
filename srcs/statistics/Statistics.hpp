#pragma once

#include <deque>
#include <map>
#include <list>
#include <vector>

class OrderBook;
class Order;

class Statistics
{
private:
    std::deque<double> midPrices;
    std::deque<double> bestBids;
    std::deque<double> bestAsks;
    std::deque<double> spreads;
    std::deque<double> _trades;
    const size_t maxSize = 100;

    void trim(std::deque<double> &data);
    void recordMidPrice(OrderBook &orderBook);
    void recordBestPrices(OrderBook &orderBook);
    void recordSpread(OrderBook &orderBook);
    void recordTrade(OrderBook &orderBook);

public:
    static std::vector<double> normalizeDeque(const std::deque<double> &data);
    static double unNormalize(double value, double max, double min);

    Statistics();
    ~Statistics();
    /**
     * records the statistics from the orderbook
     */
    void record(OrderBook &orderBook);
    void initStats(OrderBook &orderBook);

    const std::deque<double> &getMidPrices() const { return midPrices; }
    const std::deque<double> &getBestBids() const { return bestBids; }
    const std::deque<double> &getBestAsks() const { return bestAsks; }
    const std::deque<double> &getSpreads() const { return spreads; }
    const std::deque<double> &getTrades() const { return _trades; }
};
