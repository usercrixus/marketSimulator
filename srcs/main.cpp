#include "market/Market.hpp"
#include <iostream>

// void resetAgent()
// {
//     for (auto &m : Agents)
//         m.reset();
// }

// void registerAgent(Market &market)
// {
//     for (auto &m : Agents)
//         market.registerAgent(m);
// }

// void printAgentStatus(double finalMid)
// {
//     std::cout << "Agent :\n";
//     for (auto &m : Agents)
//     {
//         double net = m.getCash() + m.getInventory() * finalMid;
//         std::cout << "   net = " << net
//                   << "  (cash=" << m.getCash()
//                   << ", inv=" << m.getInventory() << ")\n";
//     }
// }

int main()
{
    for (int epoch = 0; epoch < 100; ++epoch)
    {
        std::cout << "\n==== Starting mini-market #" << epoch << " ====\n";
        // resetAgent();
        Market market(150);
        // registerAgent(market);
        if (!market.initMarket())
            return (std::cerr << "Failed to init the market\n", 1);
        std::cout << "Market initialized\n";
        market.run();
        // printAgentStatus(market.getStatistics().getMidPrices().back());
    }
    return 0;
}