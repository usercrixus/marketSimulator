#pragma once

#include <map>
#include <list>
#include <deque>
#include <vector>
#include <mutex>
#include <algorithm>
#include "Order.hpp"

class OrderBook
{
public:
    void processOrder(Order &order);
    void recordSnapShot();
    void forceSnapshot(const std::map<double, std::list<Order>> &bids, const std::map<double, std::list<Order>> &asks, const std::vector<double> &trades);
    void printBook();

    const std::map<double, std::list<Order>> &getBids() const;
    const std::map<double, std::list<Order>> &getAsks() const;
    const std::deque<std::map<double, std::list<Order>>> &getBidsSnapShots() const;
    const std::deque<std::map<double, std::list<Order>>> &getAsksSnapShots() const;
    const std::deque<std::vector<double>> &getTradeSnapShots() const;

private:
    std::map<double, std::list<Order>> _bids;
    std::map<double, std::list<Order>> _asks;
    std::vector<double> _trades;
    std::deque<std::map<double, std::list<Order>>> _bidsSnapShots;
    std::deque<std::map<double, std::list<Order>>> _asksSnapShots;
    std::deque<std::vector<double>> _tradeSnapShots;

    static constexpr size_t maxSnapshots = 100;
    std::mutex _ordersMutex;

    void matchMarket(Order &order);
    void matchLimit(Order &order);
    bool isCrossBook(const Order &order);
    void matchPostOnlyLimit(const Order &order);
    void matchCancel(const Order &order);
    void matchModify(const Order &order);
    void matchMarketAgainst(Order &order, std::map<double, std::list<Order>> &book, std::map<double, std::list<Order>>::iterator &bookLevel);
    void manageTrade(const Order &taker, const Order &maker, double price, double qty);
    /**
     * @return true if the book book level was incremented (meaning the order list was empty after the remove of the order)
     */
    bool removeOrder(std::list<Order>::iterator &order, std::list<Order> &orderList, std::map<double, std::list<Order>>::iterator &bookLevel, std::map<double, std::list<Order>> &book);
    template<typename any>
    void trim(std::deque<any> &data);
};

template<typename any>
void OrderBook::trim(std::deque<any> &data)
{
    while (data.size() > maxSnapshots)
        data.pop_front();
}