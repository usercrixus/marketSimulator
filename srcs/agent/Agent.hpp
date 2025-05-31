#pragma once

#include <vector>

class Statistics;
class Order;

class Agent
{
private:
	int asset;
	int previousAsset;
	std::vector<const Order *> pendingsOrders;

public:
	virtual ~Agent() = default;
	virtual void onEpoch(Statistics &statistics) = 0;
	virtual void onReward() = 0;

	void addPendingOrder(const Order *order);
	void removePendingOrder(const Order &order);
	void incrementAsset(int value);
};
