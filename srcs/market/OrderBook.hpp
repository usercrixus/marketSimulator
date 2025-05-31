#pragma once

#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include "Order.hpp"

class OrderBook
{
public:
    void processOrder(Order &order);
    void recordSnapShot();

    const std::map<double, std::list<Order>> &getBids() const;
    const std::map<double, std::list<Order>> &getAsks() const;

    const std::vector<std::map<double, std::list<Order>>> &getBidsSnapShots() const;
    const std::vector<std::map<double, std::list<Order>>> &getAsksSnapShots() const;
    const std::vector<std::vector<double>> &getTradeSnapShots() const;

private:
    std::map<double, std::list<Order>> _bids;
    std::map<double, std::list<Order>> _asks;
    std::vector<double> _trades;

    std::vector<std::map<double, std::list<Order>>> _bidsSnapShots;
    std::vector<std::map<double, std::list<Order>>> _asksSnapShots;
    std::vector<std::vector<double>> _tradeSnapShots;

    void matchMarket(Order &order);
    void matchLimit(Order &order);
    void matchPostOnlyLimit(const Order &order);
    void cancel(const Order &order);
    void modify(const Order &order);
    void manageTrade(const Order &taker, const Order &maker, double price, int qty);
    void matchMarketAgainst(Order &order, std::map<double, std::list<Order>> &providers, bool isBidSide);
};
