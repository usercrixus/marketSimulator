#include "market/Market.hpp"
#include "agent/Agent.hpp"
#include "agent/RandomAgent.hpp"

int main()
{
    Market market(100);
    std::vector<RandomAgent> agents;
    agents.reserve(100);
    for (int i = 0; i < 100; i++)
    {
        agents.emplace_back();
        market.registerAgent(agents.back());
    }
    if (!market.initMarket())
        return (std::cout << "failed to init the market" << std::endl, 1);
    else
        std::cout << "Market initialized" << std::endl;

    market.run();

    for (auto &agent : agents)
    {
        std::cout << "agent asset:" << agent.getAsset() << std::endl;
    }
    return 0;
}
