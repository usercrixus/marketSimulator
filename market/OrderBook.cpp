// OrderBook.cpp
#include "OrderBook.hpp"
#include <algorithm>
#include <iostream>

void OrderBook::processOrder(const Order &order)
{
    switch (order.type)
    {
    case Order::Type::MARKET:
        matchMarket(order);
        break;
    case Order::Type::LIMIT:
        matchLimit(order);
        break;
    case Order::Type::CANCEL:
        cancel(order);
        break;
    case Order::Type::MODIFY:
        modify(order);
        break;
    }
}

int OrderBook::matchMarket(const Order &order)
{
    if (order.side == Order::Side::BUY)
        return matchMarketAgainst(order, _asks);
    else
        return matchMarketAgainst(order, _bids);
}

void OrderBook::matchLimit(const Order &order)
{
    if (order.quantity > 0 && ((order.side == Order::Side::BUY && order.price > _asks.begin()->price || (order.side == Order::Side::BUY && order.price > _bids.begin()->price))))
    {
        Order resting = Order::makeLimit(order.id, order.side, order.quantity, order.price, order.agent);
        if (order.side == Order::Side::BUY)
            _bids.insert(std::move(resting));
        else
            _asks.insert(std::move(resting));
    }
}

bool OrderBook::cancel(const Order &order)
{
    // try bids
    auto bidIt = std::find_if(_bids.begin(), _bids.end(), [&](Order const &x)
                              { return x.id == order.targetId; });
    if (bidIt != _bids.end())
        return (_bids.erase(bidIt), true);
    // try asks
    auto askIt = std::find_if(_asks.begin(), _asks.end(), [&](Order const &x)
                              { return x.id == order.targetId; });
    if (askIt != _asks.end())
        return (_asks.erase(askIt), true);
    return (false);
}

bool OrderBook::modify(const Order &order)
{
    // first look in bids
    auto bidIt = std::find_if(_bids.begin(), _bids.end(), [&](Order const &existing)
                              { return existing.id == order.targetId; });
    if (bidIt != _bids.end())
    {
        Order updated = Order::makeLimit(bidIt->id, bidIt->side, order.quantity, order.price, bidIt->agent);
        _bids.erase(bidIt);
        _bids.insert(std::move(updated));
        return true;
    }
    // then looks in asks
    auto askIt = std::find_if(_asks.begin(), _asks.end(), [&](Order const &existing)
                              { return existing.id == order.targetId; });
    if (askIt != _asks.end())
    {
        Order updated = Order::makeLimit(askIt->id, askIt->side, order.quantity, order.price, askIt->agent);
        _asks.erase(askIt);
        _asks.insert(std::move(updated));
        return true;
    }
    return false;
}

void OrderBook::manageTrade(const Order &taker, const Order &maker, double price, int qty)
{
    if (taker.side == Order::Side::BUY)
    {
        taker.agent->incrementAsset(price * qty * -1);
        maker.agent->incrementAsset(price * qty);
    }
    else
    {
        taker.agent->incrementAsset(price * qty);
        maker.agent->incrementAsset(price * qty * -1);
    }
}

std::multiset<Order, OrderBook::BidCmp> OrderBook::getBids() const
{
    return (_bids);
}

std::multiset<Order, OrderBook::AskCmp> OrderBook::getAsks() const
{
    return (_asks);
}
