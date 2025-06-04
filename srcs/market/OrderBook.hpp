#pragma once

#include <map>
#include <list>
#include <deque>
#include <vector>

#include "Order.hpp"
#include "../param.hpp"

class OrderBook
{
public:
    /**
     * Manage routing for orders depending of there type (limit/market etc)
     */
    void processOrder(Order &order);
    /**
     * record a snapshot of the order book
     */
    void recordSnapShot();
    /**
     * force a snapshot to be recorded with the specified parameter (for initialization)
     */
    void forceSnapshot(const std::map<double, std::list<Order>> &bids, const std::map<double, std::list<Order>> &asks, const std::vector<double> &trades);
    /**
     * print the order book (bids and asks)
     */
    void printBook();

    const std::map<double, std::list<Order>> &getBids() const;
    const std::map<double, std::list<Order>> &getAsks() const;
    const std::deque<std::map<double, std::list<Order>>> &getBidsSnapShots() const;
    const std::deque<std::map<double, std::list<Order>>> &getAsksSnapShots() const;
    const std::deque<std::vector<double>> &getTradesSnapShots() const;

private:
    std::map<double, std::list<Order>> _bids;                      // represent the bids order (buy)
    std::map<double, std::list<Order>> _asks;                      // represent the asks order (sell)
    std::vector<double> _trades;                                   // represent the trades done
    std::deque<std::map<double, std::list<Order>>> _bidsSnapShots; // historic of the bids book
    std::deque<std::map<double, std::list<Order>>> _asksSnapShots; // historic of the ask book
    std::deque<std::vector<double>> _tradeSnapShots;               // historic of the trades

    /**
     * Entry point to manage market orders.
     */
    void matchMarket(Order &order);
    /**
     * Entry point to manage limit orders.
     */
    void matchLimit(Order &order);
    /**
     * Entry point to manage post only orders.
     */
    void matchPostOnlyLimit(const Order &order);
    /**
     * Entry point to manage cancel orders.
     */
    void matchCancel(const Order &order);
    /**
     * Entry point to manage modify orders.
     */
    void matchModify(const Order &order);
    /**
     * Notify agents that a trade happen.
     * Record the trade to the order book.
     */
    void manageTrade(const Order &taker, const Order &maker, double price, double qty);
    /**
     * Remove an order from the book.
     * If the book level become empty, remove the book level from the book.
     * @return true if the book book level was incremented (meaning the order list was empty after the remove of the order).
     */
    bool removeOrder(std::list<Order>::iterator &order, std::list<Order> &orderList, std::map<double, std::list<Order>>::iterator &bookLevel, std::map<double, std::list<Order>> &book);
    /**
     * @return true if the the order cross the book (if a post only order go against the rules of the market)
     */
    bool isCrossBook(const Order &order);
    /**
     * trim the deque contrainers.
     */
    template <typename any>
    void trim(std::deque<any> &data);
};

template <typename any>
void OrderBook::trim(std::deque<any> &data)
{
    while (data.size() > SNAPSHOT_LEN)
        data.pop_front();
}