#pragma once

class Statistics;

class Agent
{
private:
	int asset;
	int previousAsset;

public:
	virtual ~Agent() = default;
	virtual void onEpoch(Statistics &statistics) = 0;
	virtual void onReward() = 0;
	void incrementAsset(int value);
};
