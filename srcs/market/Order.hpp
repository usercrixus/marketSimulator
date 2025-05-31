#pragma once

#include <string>

class Agent;

class Order
{
public:
    /**
     * Side of the order (buy or sell)
     */
    enum class Side
    {
        BUY,
        SELL
    };

    /**
     * Type of the order (limit market cancel or modify)
     */
    enum class Type
    {
        LIMIT,
        POST_ONLY_LIMIT,
        MARKET,
        CANCEL,
        MODIFY
    };

    int id;       // id of the order
    Type type;    // type (limit market cancel modify)
    Side side;    // sell or buy
    int quantity; // number of asset to buy/sell
    double price; // for LIMIT or MODIFY
    int targetId; // for CANCEL or MODIFY
    Agent *agent; // who submitted it

    // Named constructors
    static Order makeLimit(int id, Side side, int quantity, double price, Agent *agent);
    static Order makeMarket(int id, Side side, int quantity, Agent *agent);
    static Order makeCancel(int id, int toCancel, Agent *agent);
    static Order makeModify(int id, int toModify, int newQty, double newPrice, Agent *agent);

private:
    Order() = default; // force use of named factories
};
