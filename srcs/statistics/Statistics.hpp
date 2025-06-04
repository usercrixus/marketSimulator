#pragma once

#include <deque>
#include <vector>
#include "../param.hpp"

class OrderBook;
class Order;

class Statistics
{
public:
    Statistics();
    ~Statistics();
    /**
     * Normalize a full deque container (for AI agent)
     */
    static std::vector<double> normalizeDeque(const std::deque<double> &data);
    /**
     * Unormalize a value
     */
    static double unNormalize(double value, double max, double min);
    /**
     * records the statistics from the orderbook last history
     */
    void record(OrderBook &orderBook);
    /**
     * records statistic from a whole order book history
     */
    void initStats(OrderBook &orderBook);

    const std::deque<double> &getMidPrices() const { return _midPrices; }
    const std::deque<double> &getBestBids() const { return _bestBids; }
    const std::deque<double> &getBestAsks() const { return _bestAsks; }
    const std::deque<double> &getSpreads() const { return _spreads; }
    const std::deque<double> &getTrades() const { return _trades; }

private:
    std::deque<double> _midPrices; // mid prices history
    std::deque<double> _bestBids;  // best bids history
    std::deque<double> _bestAsks;  // best asks history
    std::deque<double> _spreads;   // spreads history
    std::deque<double> _trades;    // trades history

    void trim(std::deque<double> &data);
    /**
     * Record current mid price from the order book
     */
    void recordMidPrice(OrderBook &orderBook);
    /**
     * record current best ask and bid from the order book
     */
    void recordBestPrices(OrderBook &orderBook);
    /**
     * record current spread from the order book
     */
    void recordSpread(OrderBook &orderBook);
    /**
     * records last trade of the order book
     */
    void recordTrade(OrderBook &orderBook);
};
