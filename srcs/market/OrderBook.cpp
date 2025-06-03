#include "OrderBook.hpp"
#include "../agent/Agent.hpp"
#include <iterator>
#include <algorithm>
#include <iostream>

void OrderBook::processOrder(Order &order)
{
    std::lock_guard<std::mutex> lock(_ordersMutex);
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
        matchCancel(order);
        break;
    case Order::Type::MODIFY:
        matchModify(order);
        break;
    }
}

void OrderBook::matchMarket(Order &order)
{
    while (order.quantity > 0)
    {
        std::map<double, std::list<Order>> &book = (order.side == Order::Side::BUY ? _asks : _bids);
        if (book.empty())
            break;
        std::map<double, std::list<Order>>::iterator bookLevel = (order.side == Order::Side::BUY ? book.begin() : std::prev(book.end()));
        matchMarketAgainst(order, book, bookLevel);
    }
}

void OrderBook::matchMarketAgainst(Order &order, std::map<double, std::list<Order>> &book, std::map<double, std::list<Order>>::iterator &bookLevel)
{
    double priceLevel = bookLevel->first;
    std::list<Order> &orderList = bookLevel->second;
    std::list<Order>::iterator provider = orderList.begin();

    while (provider != orderList.end() && order.quantity > 0)
    {
        double tradeQty = std::min(order.quantity, provider->quantity);
        manageTrade(order, *provider, priceLevel, tradeQty);
        order.quantity -= tradeQty;
        provider->quantity -= tradeQty;
        if (provider->quantity == 0)
        {
            if (removeOrder(provider, orderList, bookLevel, book))
                break;
        }
        else
            ++provider;
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

bool OrderBook::isCrossBook(const Order &order)
{
    if (order.side == Order::Side::BUY)
    {
        if (!_asks.empty())
        {
            double bestAsk = _asks.begin()->first;
            if (order.price >= bestAsk)
                return (true);
        }
    }
    else
    {
        if (!_bids.empty())
        {
            double bestBid = std::prev(_bids.end())->first;
            if (order.price <= bestBid)
                return (true);
        }
    }
    return (false);
}

void OrderBook::matchPostOnlyLimit(const Order &order)
{
    if (order.quantity > 0 && !isCrossBook(order))
    {
        if (order.side == Order::Side::BUY)
        {
            std::list<Order> &levelOrders = _bids[order.price];
            levelOrders.push_back(order);
            order.agent->addPendingOrder(levelOrders.back());
        }
        else
        {
            std::list<Order> &levelOrders = _asks[order.price];
            levelOrders.push_back(order);
            order.agent->addPendingOrder(levelOrders.back());
        }
    }
}

void OrderBook::matchCancel(const Order &order)
{
    std::map<double, std::list<Order>> &book = (order.side == Order::Side::BUY) ? _bids : _asks;
    std::map<double, std::list<Order>>::iterator bookLevel = book.begin();
    while (bookLevel != book.end())
    {
        std::list<Order> &orderList = bookLevel->second;
        std::list<Order>::iterator targetOrder = std::find_if(orderList.begin(), orderList.end(), [&](const Order &o)
                                                              { return o.id == order.targetId; });
        if (targetOrder != orderList.end())
        {
            removeOrder(targetOrder, orderList, bookLevel, book);
            break;
        }
        else
            ++bookLevel;
    }
}

void OrderBook::matchModify(const Order &order)
{
    std::map<double, std::list<Order>> &book = (order.side == Order::Side::BUY) ? _bids : _asks;
    std::map<double, std::__cxx11::list<Order>>::iterator bookLevel = book.begin();
    while (bookLevel != book.end())
    {
        std::list<Order> &orderList = bookLevel->second;
        std::list<Order>::iterator targetIt = std::find_if(orderList.begin(), orderList.end(), [&](const Order &o)
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
            removeOrder(targetIt, orderList, bookLevel, book);
            break;
        }
        else
            ++bookLevel;
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
    }
    else
    {
        if (taker.agent)
            taker.agent->updatePosition(price, qty, Order::Side::SELL);
        if (maker.agent)
            maker.agent->updatePosition(price, qty, Order::Side::BUY);
    }
    _trades.push_back(qty);
}

bool OrderBook::removeOrder(std::list<Order>::iterator &order, std::list<Order> &orderList, std::map<double, std::list<Order>>::iterator &bookLevel, std::map<double, std::list<Order>> &book)
{
    order->agent->removePendingOrder(*order);
    order = orderList.erase(order);
    if (orderList.empty())
    {
        bookLevel = book.erase(bookLevel);
        return (true);
    }
    return (false);
}

void OrderBook::recordSnapShot()
{
    _bidsSnapShots.push_back(_bids);
    _asksSnapShots.push_back(_asks);
    if (_trades.empty())
        _trades.push_back(0.0);
    _tradeSnapShots.push_back(_trades);
    _trades.clear();

    trim(_bidsSnapShots);
    trim(_asksSnapShots);
    trim(_tradeSnapShots);
}

void OrderBook::forceSnapshot(const std::map<double, std::list<Order>> &bids, const std::map<double, std::list<Order>> &asks, const std::vector<double> &trades)
{
    _bidsSnapShots.push_back(bids);
    _asksSnapShots.push_back(asks);
    _tradeSnapShots.push_back(trades);

    trim(_bidsSnapShots);
    trim(_asksSnapShots);
    trim(_tradeSnapShots);
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

const std::map<double, std::list<Order>> &OrderBook::getBids() const
{
    return _bids;
}

const std::map<double, std::list<Order>> &OrderBook::getAsks() const
{
    return _asks;
}

const std::deque<std::map<double, std::list<Order>>> &OrderBook::getBidsSnapShots() const
{
    return _bidsSnapShots;
}

const std::deque<std::map<double, std::list<Order>>> &OrderBook::getAsksSnapShots() const
{
    return _asksSnapShots;
}

const std::deque<std::vector<double>> &OrderBook::getTradeSnapShots() const
{
    return _tradeSnapShots;
}
