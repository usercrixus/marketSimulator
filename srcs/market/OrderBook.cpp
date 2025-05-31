#include "OrderBook.hpp"
#include "../agent/Agent.hpp"

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

int OrderBook::matchMarket(const Order &order)
{
    if (order.side == Order::Side::BUY)
        return matchMarketAgainst(order, _asks);
    else
        return matchMarketAgainst(order, _bids);
}

void OrderBook::matchLimit(const Order &order)
{
    int remaining = matchMarket(order);

    if (remaining > 0)
    {
        Order resting = Order::makeLimit(order.id, order.side, remaining, order.price, order.agent);
        if (order.side == Order::Side::BUY)
            _bids.insert(std::move(resting));
        else
            _asks.insert(std::move(resting));
    }
}

bool OrderBook::matchPostOnlyLimit(const Order &order)
{
    if (order.quantity <= 0)
        return false;

    bool crossesBook = false;
    if (order.side == Order::Side::BUY)
    {
        if (!_asks.empty() && order.price >= _asks.begin()->price)
            crossesBook = true;
    }
    else
    {
        if (!_bids.empty() && order.price <= _bids.begin()->price)
            crossesBook = true;
    }
    if (crossesBook)
        return (false); // cancel if it would match
    // otherwise, safe to rest the order
    Order resting = Order::makeLimit(order.id, order.side, order.quantity, order.price, order.agent);
    if (order.side == Order::Side::BUY)
        _bids.insert(std::move(resting));
    else
        _asks.insert(std::move(resting));
    return (true);
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
        _trades.push_back(price * qty);
    }
    else
    {
        taker.agent->incrementAsset(price * qty);
        maker.agent->incrementAsset(price * qty * -1);
        _trades.push_back(price * qty * -1);
    }
}

void OrderBook::recordSnapShot()
{
    _bidsSnapShots.push_back(_bids);
    _asksSnapShots.push_back(_asks);
    _tradeSnapShots.push_back(_trades);
    _trades.clear();
}

const std::multiset<Order, OrderBook::BidCmp> &OrderBook::getBids() const
{
    return (_bids);
}

const std::multiset<Order, OrderBook::AskCmp> &OrderBook::getAsks() const
{
    return (_asks);
}

const std::vector<std::multiset<Order, OrderBook::BidCmp>> &OrderBook::getBidsSnapShots() const
{
    return (_bidsSnapShots);
}

const std::vector<std::multiset<Order, OrderBook::AskCmp>> &OrderBook::getAsksSnapShots() const
{
    return (_asksSnapShots);
}

const std::vector<std::vector<double>> &OrderBook::getTradeSnapShots() const
{
    return (_tradeSnapShots);
}
