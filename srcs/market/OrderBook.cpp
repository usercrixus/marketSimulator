#include "OrderBook.hpp"
#include "../agent/Agent.hpp"
#include <iterator>
#include <algorithm>
#include <iostream>

void OrderBook::processOrder(Order &order)
{
    switch (order.type)
    {
    case Order::Type::MARKET:
        matchMarket(order);
        break;
    case Order::Type::LIMIT:
        matchLimit(order);
        break;
    case Order::Type::POST_ONLY_LIMIT:
        matchPostOnlyLimit(order);
        break;
    case Order::Type::CANCEL:
        cancel(order);
        break;
    case Order::Type::MODIFY:
        modify(order);
        break;
    }
}

void OrderBook::matchMarket(Order &order)
{
    if (order.side == Order::Side::BUY)
        matchMarketAgainst(order, _asks, false);
    else
        matchMarketAgainst(order, _bids, true);
}

void OrderBook::matchMarketAgainst(Order &order, std::map<double, std::list<Order>> &providers, bool isBidSide)
{
    // std::cout << "try " << order.quantity << " " <<  !providers.empty() << std::endl;
    while (order.quantity > 0 && !providers.empty())
    {
        std::map<double, std::list<Order>>::iterator orderBookLevel = (isBidSide) ? std::prev(providers.end()) : providers.begin();
        double priceLevel = orderBookLevel->first;
        std::list<Order> &providersLevel = orderBookLevel->second;
        std::list<Order>::iterator providerLevel = providersLevel.begin();
        while (providerLevel != providersLevel.end() && order.quantity > 0)
        {
            double tradeQty = std::min(order.quantity, providerLevel->quantity);
            // Execute the trade
            manageTrade(order, *providerLevel, priceLevel, tradeQty);
            // Decrement both sides’ quantities
            order.quantity -= tradeQty;
            providerLevel->quantity -= tradeQty;
            if (providerLevel->quantity == 0)
            {
                // Fully filled ⇒ remove from this price level + notify agent
                providerLevel->agent->removePendingOrder(*providerLevel);
                providerLevel = providersLevel.erase(providerLevel);
            }
            else
                ++providerLevel;
        }
        // If no more orders remain at this price, erase that entire price key
        if (providersLevel.empty())
            providers.erase(orderBookLevel);
    }
}

void OrderBook::matchLimit(Order &order)
{
    matchMarket(order);
    if (order.quantity > 0)
    {
        if (order.side == Order::Side::BUY)
        {
            std::list<Order> &levelList = _bids[order.price];
            levelList.push_back(order);
            order.agent->addPendingOrder(levelList.back());
        }
        else
        {
            std::list<Order> &levelList = _asks[order.price];
            levelList.push_back(order);
            order.agent->addPendingOrder(levelList.back());
        }
    }
}

void OrderBook::matchPostOnlyLimit(const Order &order)
{
    if (order.quantity <= 0)
        return;

    bool crossesBook = false;
    if (order.side == Order::Side::BUY)
    {
        if (!_asks.empty())
        {
            double bestAsk = _asks.begin()->first;
            if (order.price >= bestAsk)
                crossesBook = true;
        }
    }
    else
    {
        if (!_bids.empty())
        {
            double bestBid = std::prev(_bids.end())->first;
            if (order.price <= bestBid)
                crossesBook = true;
        }
    }
    if (!crossesBook)
    {
        if (order.side == Order::Side::BUY)
        {
            std::list<Order> &levelList = _bids[order.price];
            levelList.push_back(order);
            order.agent->addPendingOrder(levelList.back());
        }
        else
        {
            auto &levelList = _asks[order.price];
            levelList.push_back(order);
            order.agent->addPendingOrder(levelList.back());
        }
    }
}

void OrderBook::cancel(const Order &order)
{
    // Choose which side to search
    std::map<double, std::list<Order>> &book = (order.side == Order::Side::BUY) ? _bids : _asks;
    std::map<double, std::list<Order>>::iterator bookLevel = book.begin();
    while (bookLevel != book.end())
    {
        std::list<Order> &orderList = bookLevel->second;
        auto targetOrder = std::find_if(orderList.begin(), orderList.end(), [&](const Order &o)
                                        { return o.id == order.targetId; });
        if (targetOrder != orderList.end())
        {
            // Tell the agent to remove it from their pending list
            targetOrder->agent->removePendingOrder(*targetOrder);
            // Erase this order from the list
            orderList.erase(targetOrder);
            // If no more orders at that price level, erase the entire level
            if (orderList.empty())
                bookLevel = book.erase(bookLevel);
            break;
        }
        else
            ++bookLevel;
    }
}

void OrderBook::modify(const Order &order)
{
    auto &book = (order.side == Order::Side::BUY) ? _bids : _asks;

    auto it = book.begin();
    while (it != book.end())
    {
        auto &orderList = it->second;
        auto targetIt = std::find_if(orderList.begin(), orderList.end(), [&](const Order &o)
                                     { return o.id == order.targetId; });
        if (targetIt != orderList.end())
        {
            if (targetIt->type == Order::Type::POST_ONLY_LIMIT)
            {
                Order repost = Order::makePostOnlyLimit(targetIt->id, targetIt->side, order.quantity, order.price, targetIt->agent);
                matchPostOnlyLimit(repost);
            }
            else
            {
                Order relimit = Order::makeLimit(targetIt->id, targetIt->side, order.quantity, order.price, targetIt->agent);
                matchLimit(relimit);
            }

            // Remove from the agent’s pending list:
            if (targetIt->agent)
                targetIt->agent->removePendingOrder(*targetIt);
            // Erase from this price level:
            orderList.erase(targetIt);
            // If that price level is now empty, erase the map key:
            if (orderList.empty())
                it = book.erase(it);
            break;
        }
        else
            ++it;
    }
}

void OrderBook::manageTrade(const Order &taker, const Order &maker, double price, double qty)
{
    if (taker.side == Order::Side::BUY)
    {
        if (taker.agent)
            taker.agent->updatePosition(price, qty, Order::Side::BUY);
        if (maker.agent)
            maker.agent->updatePosition(price, qty, Order::Side::SELL);
        _trades.push_back(price * qty);
        // std::cout << "trade done, price: " << price << " quantity: " << qty << std::endl;
    }
    else
    {
        if (taker.agent)
            taker.agent->updatePosition(price, qty, Order::Side::SELL);
        if (maker.agent)
            maker.agent->updatePosition(price, qty, Order::Side::BUY);
        _trades.push_back(price * qty);
        // std::cout << "trade done, price: " << price << " quantity: " << qty << std::endl;
    }
}

void OrderBook::recordSnapShot()
{
    _bidsSnapShots.push_back(_bids);
    _asksSnapShots.push_back(_asks);
    _tradeSnapShots.push_back(_trades);
    _trades.clear();
}

void OrderBook::forceSnapshot(const std::map<double, std::list<Order>> &bids, const std::map<double, std::list<Order>> &asks, const std::vector<double> &trades)
{
    _bidsSnapShots.push_back(bids);
    _asksSnapShots.push_back(asks);
    _tradeSnapShots.push_back(trades);
}

void OrderBook::printBook()
{
    std::cout << "Bid: " << std::endl;
    for (auto bid : _bids)
        std::cout << bid.first << std::endl;
    std::cout << "Ask: " << std::endl;
    for (auto ask : _asks)
        std::cout << ask.first << std::endl;
}

const std::map<double, std::list<Order>> &
OrderBook::getBids() const
{
    return _bids;
}

const std::map<double, std::list<Order>> &
OrderBook::getAsks() const
{
    return _asks;
}

const std::vector<std::map<double, std::list<Order>>> &
OrderBook::getBidsSnapShots() const
{
    return _bidsSnapShots;
}

const std::vector<std::map<double, std::list<Order>>> &
OrderBook::getAsksSnapShots() const
{
    return _asksSnapShots;
}

const std::vector<std::vector<double>> &
OrderBook::getTradeSnapShots() const
{
    return _tradeSnapShots;
}
