# marketSimulator

A high-performance market simulation engine designed to train and evaluate trading agents (especially reinforcement learning agents) in a competitive, realistic environment.

## 🧠 Purpose

This simulator models a financial exchange with a central order book. Agents can place market or limit orders and compete for profits across multiple steps. The goal is to evolve and evaluate agent trading strategies — and eventually fine-tune top-performing agents for use in real-world markets.

## 🔄 Simulation Mode

Runs in discrete epochs for fast RL training.  
Agents evolve over time through competition.  

## ✅ Current Features

Central limit order book (price-time priority).  
Market, limit, post-only, cancel, and modify order types.  
Real-time matching logic.  
Asset tracking per agent.  

## 🚀 Getting Started

Install torch (ubuntu): 
Make libtorch  
If not already installed, install cuda for GPU:  
https://developer.nvidia.com/cuda-12-6-0-download-archive?target_os=Linux&target_arch=x86_64&Distribution=Ubuntu&target_version=24.04&target_type=deb_network  
  
Create some agents (Market maker + takers)  
Then adapt the main as it register all agents. 
  
launch the scrapper (utils/scrapper.py) as you can safely initialize the market.  
  
Entry point:  
srcs/main.cpp  
srcs/market/Market.hpp/cpp  
