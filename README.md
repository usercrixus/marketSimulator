# marketSimulator

A high-performance market simulation engine designed to train and evaluate trading agents (especially reinforcement learning agents) in a competitive, realistic environment.

## 🧠 Purpose

This simulator models a financial exchange with a central order book. Agents can place market or limit orders and compete for profits across multiple epochs. The goal is to evolve and evaluate agent trading strategies — and eventually fine-tune top-performing agents for use in real-world markets.

## 👤 Agent Types

Basic Players: Simple, risk-averse agents who lose more often than they win. They provide minimal liquidity.  
Medium Players: More competent agents using market orders and moderate liquidity strategies.  
Pro Players:  
  Market Makers (full limit order)  
  Takers (full market order)  
  Hybrids (mix of both)  

## 🔄 Simulation Mode

Runs in discrete epochs for fast RL training.  
Agents submit orders, which are shuffled, matched via an order book, and settled.  
Agents evolve over time through competition.  

## ✅ Current Features

Central limit order book (price-time priority).  
Market, limit, cancel, and modify order types.  
Real-time matching logic.  
Asset tracking per agent.  

## 🚀 Getting Started

Entry point:  
srcs/market/Market.hpp/cpp  
Start the simulation, register agents, and call .run().  
