// --- srcs/main.cpp ---
#include "market/Market.hpp"
#include "agent/MarketMakerAgent.hpp"
#include "agent/TakerAgent.hpp"
#include <iostream>

int main()
{
    // 1) Create 20 market‐makers and 80 takers
    std::vector<MarketMakerAgent> mmAgents;
    mmAgents.reserve(20);
    for (int i = 0; i < 20; i++)
        mmAgents.emplace_back();

    std::vector<TakerAgent> takerAgents;
    takerAgents.reserve(80);
    for (int i = 0; i < 80; i++)
        takerAgents.emplace_back();

    // 2) Repeat “mini‐markets” 1000 times
    for (int iteration = 0; iteration < 1000; ++iteration)
    {
        std::cout << "\n==== Starting mini-market #" << iteration << " ====\n";

        // Before each 100-step run, reset every agent
        for (auto &m : mmAgents)    m.reset();
        for (auto &t : takerAgents) t.reset();

        // Build a fresh Market with 100 steps
        Market market(100);

        // Register all agents
        for (auto &m : mmAgents)    market.registerAgent(m);
        for (auto &t : takerAgents) market.registerAgent(t);

        if (!market.initMarket()) {
            std::cerr << "Failed to init the market\n";
            return 1;
        }
        std::cout << "Market initialized\n";

        // Run the 100-step simulation (rewards happen only at the end)
        market.run();

        // Print out each agent’s final net‐worth:
        //   We’ll use the final best ask as a stand-in for “current mid.”
        double finalMid = market.getOrderBook().getAsksSnapShots().back().begin()->first;
        std::cout << "→ Market-makers:\n";
        for (auto &m : mmAgents) {
            double net = m.getCash() + m.getInventory() * finalMid;
            std::cout << "   net = " << net
                      << "  (cash=" << m.getCash()
                      << ", inv=" << m.getInventory() << ")\n";
        }

        std::cout << "→ Takers:\n";
        for (auto &t : takerAgents) {
            double net = t.getCash() + t.getInventory() * finalMid;
            std::cout << "   net = " << net
                      << "  (cash=" << t.getCash()
                      << ", inv=" << t.getInventory() << ")\n";
        }
    }

    return 0;
}
