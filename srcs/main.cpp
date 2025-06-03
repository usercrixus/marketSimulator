// --- srcs/main.cpp ---
#include "market/Market.hpp"
#include "agent/MarketMakerAgent.hpp"
#include "agent/TakerAgent.hpp"
#include "agent/UntrainedTakerAgent.hpp"
#include <iostream>

std::vector<MarketMakerAgent> mmAgents;
std::vector<TakerAgent> takerAgents;
std::vector<UntrainedTakerAgent> takerAgentsUntrained;

void initAgent()
{
    mmAgents.reserve(20);
    for (int i = 0; i < 20; i++)
        mmAgents.emplace_back();

    takerAgents.reserve(20);
    for (int i = 0; i < 20; i++)
        takerAgents.emplace_back();

    takerAgentsUntrained.reserve(60);
    for (int i = 0; i < 60; i++)
        takerAgentsUntrained.emplace_back();
}

void resetAgent()
{
    for (auto &m : mmAgents)
        m.reset();
    for (auto &u : takerAgentsUntrained)
        u.reset();
    for (auto &t : takerAgents)
        t.reset();
}

void registerAgent(Market &market)
{
    for (auto &m : mmAgents)
        market.registerAgent(m);

    for (auto &t : takerAgents)
        market.registerAgent(t);

    for (auto &t : takerAgentsUntrained)
        market.registerAgent(t);
}

void printAgentStatus(double finalMid)
{
    std::cout << "→ MarketMakerAgents (trained):\n";
    for (auto &m : mmAgents)
    {
        double net = m.getCash() + m.getInventory() * finalMid;
        std::cout << "   net = " << net
                  << "  (cash=" << m.getCash()
                  << ", inv=" << m.getInventory() << ")\n";
    }

    std::cout << "→ TakerAgents:\n";
    for (auto &t : takerAgents)
    {
        double net = t.getCash() + t.getInventory() * finalMid;
        std::cout << "   net = " << net
                  << "  (cash=" << t.getCash()
                  << ", inv=" << t.getInventory() << ")\n";
    }
    std::cout << "→ TakerAgentsUntrained:\n";
    for (auto &t : takerAgentsUntrained)
    {
        double net = t.getCash() + t.getInventory() * finalMid;
        std::cout << "   net = " << net
                  << "  (cash=" << t.getCash()
                  << ", inv=" << t.getInventory() << ")\n";
    }
}

int main()
{
    initAgent();

    // 2) Repeat “mini‐markets” 1000 times
    for (int iteration = 0; iteration < 100; ++iteration)
    {
        std::cout << "\n==== Starting mini-market #" << iteration << " ====\n";
        resetAgent();
        Market market(500);
        registerAgent(market);
        if (!market.initMarket())
            return (std::cerr << "Failed to init the market\n", 1);
        std::cout << "Market initialized\n";
        market.run();
        printAgentStatus(market.getStatistics().getMidPrices().back());
    }

    return 0;
}