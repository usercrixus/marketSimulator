#pragma once
#include <torch/torch.h>

// A simple single‐layer LSTM: input_size=4 → hidden_size=128 → output_size=2
class ModelLSTM : public torch::nn::Module
{
private:
	// Each LSTM layer is its own module:
	torch::nn::LSTM lstm1{nullptr}; // 4 → 128
	torch::nn::LSTM lstm2{nullptr}; // 128 → 64
	torch::nn::LSTM lstm3{nullptr}; // 64  → 32
	torch::nn::LSTM lstm4{nullptr}; // 32  → 32

	// Final linear from hidden_size=32 → output_size=2
	torch::nn::Linear fc{nullptr};

public:
	ModelLSTM(int inputSize, int outputSize);
	torch::Tensor forward(torch::Tensor x);
};
