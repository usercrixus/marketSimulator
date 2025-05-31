#pragma once

#include "OrderBook.hpp"

template <typename Providers>
int OrderBook::matchMarketAgainst(const Order &order, Providers &providers)
{
    int remaining = order.quantity;
    auto it = providers.begin();
    while (it != providers.end() && remaining > 0)
    {
        bool priceOk = (order.side == Order::Side::BUY ? (it->price <= order.price) : (it->price >= order.price));
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