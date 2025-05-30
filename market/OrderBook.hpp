
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
     * routing to manage order processing
     */
    void processOrder(const Order &o);

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
     * For market order, try to fill it with the providers.
     */
    template <typename Providers>
    int matchMarketAgainst(const Order &order, Providers &providers);

    /**
     * routing of market order (buy or sell)
     */
    int matchMarket(const Order &order);

    /**
     * handle limit order registration
     */
    void matchLimit(const Order &order);

    /**
     * handle order limit order cancelation
     */
    bool cancel(const Order &order);

    /**
     * handle limit order modification
     */
    bool modify(const Order &order);

    /**
     * Manage the agent asset portfolio
     */
    void manageTrade(const Order &taker, const Order &maker, double price, int qty);

    std::multiset<Order, BidCmp> getBids() const;
    std::multiset<Order, AskCmp> getAsks() const;
};

template <typename Providers>
int OrderBook::matchMarketAgainst(const Order &order, Providers &providers)
{
    int remaining = order.quantity;
    auto it = providers.begin();
    while (it != providers.end() && remaining > 0)
    {
        bool priceOk = (order.side == Side::BUY ? (it->price <= order.price) : (it->price >= order.price));
        if (!priceOk)
            break;
        int tradeQty = std::min(remaining, it->quantity);
        manageTrade(order, *it, it->price, tradeQty);
        remaining -= tradeQty;
        Order updated = *it;
        updated.quantity -= tradeQty;
        it = providers.erase(it);
        if (updated.quantity > 0)
            providers.insert(std::move(updated));
    }
    return remaining;
}