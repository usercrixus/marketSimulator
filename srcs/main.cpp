#include "market/Market.hpp"
#include "agent/Agent.hpp"
#include "agent/MarketMakerAgent.hpp"
#include "agent/TakerAgent.hpp"

int main()
{
    std::vector<MarketMakerAgent> marketMakerAgent;
    marketMakerAgent.reserve(100);
    for (int i = 0; i < 20; i++)
        marketMakerAgent.emplace_back();

    std::vector<TakerAgent> takerAgent;
    takerAgent.reserve(100);
    for (int i = 0; i < 80; i++)
        takerAgent.emplace_back();

    int epoch = 0;
    while (epoch < 1000)
    {
        Market market(100);
        for (auto &agent : marketMakerAgent)
            market.registerAgent(agent);
        for (auto &agent : takerAgent)
            market.registerAgent(agent);
        if (!market.initMarket())
            return (std::cout << "failed to init the market" << std::endl, 1);
        else
            std::cout << "Market initialized" << std::endl;
        market.run();
        for (auto &agent : marketMakerAgent)
            std::cout << "agent asset:" << agent.getAsset() << std::endl;
        epoch++;
    }
    return 0;
}
