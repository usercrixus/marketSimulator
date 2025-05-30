// Order.cpp
#include "Order.hpp"

Order Order::makeLimit(int id, Side side, int quantity, double price, Agent *agent)
{
    Order order;
    order.id = id;
    order.type = Type::POST_ONLY_LIMIT;
    order.side = side;
    order.quantity = quantity;
    order.price = price;
    order.targetId = -1;
    order.agent = agent;
    return order;
}

Order Order::makeMarket(int id, Side side, int quantity, Agent *agent)
{
    Order order;
    order.id = id;
    order.type = Type::MARKET;
    order.side = side;
    order.quantity = quantity;
    order.price = 0.0;
    order.targetId = -1;
    order.agent = agent;
    return order;
}

Order Order::makeCancel(int id, int toCancel, Agent *agent)
{
    Order order;
    order.id = id;
    order.type = Type::CANCEL;
    order.side = Side::BUY; // unused but must be set
    order.quantity = 0;
    order.price = 0.0;
    order.targetId = toCancel;
    order.agent = agent;
    return order;
}

Order Order::makeModify(int id, int toModify, int newQty, double newPrice, Agent *agent)
{
    Order order;
    order.id = id;
    order.type = Type::MODIFY;
    order.side = Side::BUY; // unused but must be set
    order.quantity = newQty;
    order.price = newPrice;
    order.targetId = toModify;
    order.agent = agent;
    return order;
}
