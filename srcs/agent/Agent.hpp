#pragma once

#include <vector>

class Statistics;
class Order;
class Market;

class Agent
{
protected:
	double asset;
	double previousAsset;
	std::vector<const Order *> pendingsOrders;

public:
	Agent();
	virtual ~Agent();
	virtual void onEpoch(Statistics &statistics, Market &market) = 0;
	virtual void onReward() = 0;

	void addPendingOrder(const Order &order);
	void removePendingOrder(const Order &order);
	void incrementAsset(double value);

	double getAsset();
};
