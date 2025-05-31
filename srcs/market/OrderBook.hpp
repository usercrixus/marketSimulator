
#pragma once

#include <vector>
#include <optional>
#include "Order.hpp"
#include <algorithm>
#include <set>

class OrderBook
{
public:
    /**
     * routing to manage order processing, meaning try to buy/sell, records limit orders in the OrderBook and so on.
     */
    void processOrder(const Order &o);

    /**
     * record snapshots of order book, trade ans so on, at end of epoch.
     */
    void recordSnapShot();

private:
    /**
     * Automatic sorted container for bid
     */
    struct BidCmp
    {
        bool operator()(Order const &a, Order const &b) const
        {
            // higher price first; if tie, lower id first
            if (a.price != b.price)
                return a.price > b.price;
            return a.id < b.id;
        }
    };

    /**
     * Automatic sorted container for ask
     */
    struct AskCmp
    {
        bool operator()(Order const &a, Order const &b) const
        {
            // lower price first; if tie, lower id first
            if (a.price != b.price)
                return a.price < b.price;
            return a.id < b.id;
        }
    };

    /**
     * bid order book
     */
    std::multiset<Order, BidCmp> _bids;

    /**
     * ask order book
     */
    std::multiset<Order, AskCmp> _asks;

    /**
     * bids orderbook history
     */
    std::vector<std::multiset<Order, BidCmp>> _bidsSnapShots;

    /**
     * asks history
     */
    std::vector<std::multiset<Order, AskCmp>> _asksSnapShots;

    /**
     * trades for the epoch
     */
    std::vector<double> _trades;

    /**
     * trade history
     */
    std::vector<std::vector<double>> _tradeSnapShots;

    /**
     * For market order, try to fill it with the providers.
     */
    template <typename Providers>
    int matchMarketAgainst(const Order &order, Providers &providers);

    /**
     * routing of market order (try to buy or to sell)
     */
    int matchMarket(const Order &order);

    /**
     * handle limit order (try to record a limit order)
     */
    void matchLimit(const Order &order);

    /**
     * handle post only limit order registration (try to records a post only limit order)
     */
    bool matchPostOnlyLimit(const Order &order);

    /**
     * handle order limit order cancelation (try to cancel an order)
     */
    bool cancel(const Order &order);

    /**
     * handle limit order modification (try to modify an order, price, quantity)
     */
    bool modify(const Order &order);

    /**
     * Manage the agent asset portfolio. Record the trade.
     */
    void manageTrade(const Order &taker, const Order &maker, double price, int qty);

    std::multiset<Order, BidCmp> getBids() const;
    std::multiset<Order, AskCmp> getAsks() const;
    std::vector<std::multiset<Order, BidCmp>> getBidsSnapShots();
    std::vector<std::multiset<Order, AskCmp>> getAsksSnapShots();
    std::vector<std::vector<double>> getTradeSnapShots();

};

#include "OrderBook.ipp"