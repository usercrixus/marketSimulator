// srcs/agent/models/ModelLSTM.cpp
#include "ModelLSTM.hpp"
#include <torch/torch.h>

ModelLSTM::ModelLSTM(int inputSize, int outputSize)
    : lstm1(torch::nn::LSTMOptions(inputSize, 128).num_layers(1).batch_first(true)),
      lstm2(torch::nn::LSTMOptions(128, 64).num_layers(1).batch_first(true)),
      lstm3(torch::nn::LSTMOptions(64, 32).num_layers(1).batch_first(true)),
      lstm4(torch::nn::LSTMOptions(32, 32).num_layers(1).batch_first(true)),
      fc(32, outputSize)
{
    register_module("lstm1", lstm1);
    register_module("lstm2", lstm2);
    register_module("lstm3", lstm3);
    register_module("lstm4", lstm4);
    register_module("fc", fc);
}

torch::Tensor ModelLSTM::forward(torch::Tensor x)
{
    auto out1 = lstm1->forward(x);
    auto seq1 = std::get<0>(out1);
    auto out2 = lstm2->forward(seq1);
    auto seq2 = std::get<0>(out2);
    auto out3 = lstm3->forward(seq2);
    auto seq3 = std::get<0>(out3);
    auto out4 = lstm4->forward(seq3);
    auto seq4 = std::get<0>(out4);
    auto last_hidden = seq4.select(1, x.size(1) - 1);
    auto output = fc->forward(last_hidden);
    return output;
}
