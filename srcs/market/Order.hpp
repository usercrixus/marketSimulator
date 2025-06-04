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

    int id;          // id of the order
    Type type;       // type (limit market cancel modify)
    Side side;       // sell or buy
    double quantity; // number of asset to buy/sell
    double price;    // for LIMIT or MODIFY
    int targetId;    // for CANCEL or MODIFY
    Agent *agent;    // who submitted it

    // Named constructors
    static Order makeLimit(int id, Side side, double quantity, double price, Agent *agent);
    static Order makeMarket(int id, Side side, double quantity, Agent *agent);
    static Order makeCancel(int id, int toCancel, Agent *agent);
    static Order makeModify(int id, int toModify, double newQty, double newPrice, Agent *agent);
    static Order makePostOnlyLimit(int id, Side side, double quantity, double price, Agent *agent);

private:
    Order() = default; // force use of named factories
};
