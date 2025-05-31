#pragma once

#include <deque>

class OrderBook;

class Statistics
{
private:
    std::deque<double> midPrices;
    std::deque<double> bestBids;
    std::deque<double> bestAsks;
    std::deque<double> spreads;
    const size_t maxSize = 100;

    void trim(std::deque<double> &data);
    void recordMidPrice(OrderBook &orderBook);
    void recordBestPrices(OrderBook &orderBook);
    void recordSpread(OrderBook &orderBook);

public:
    Statistics();
    ~Statistics();
    /**
     * records the statistics from the orderbook
     */
    void record(OrderBook &orderBook);

    const std::deque<double> &getMidPrices() const { return midPrices; }
    const std::deque<double> &getBestBids() const { return bestBids; }
    const std::deque<double> &getBestAsks() const { return bestAsks; }
    const std::deque<double> &getSpreads() const { return spreads; }
};
