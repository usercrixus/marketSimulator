#pragma once

class Market;

class Agent
{
private:
	int asset;

public:
	virtual ~Agent() = default;
	virtual void onEpoch(Market &market) = 0;
	void incrementAsset(int value);
};
