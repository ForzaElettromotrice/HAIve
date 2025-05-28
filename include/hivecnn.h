//
// Created by filip on 15/05/2025.
//

#ifndef HIVECNN_H
#define HIVECNN_H

#include <torch/torch.h>
#include <preprocessor.h>
#include <iostream>
#include <format>
#include <filesystem>

struct HiveCNNImpl : torch::nn::Module {
    std::string checkpoint_file;
    torch::nn::Sequential conv_layers{nullptr}, fc_layers{nullptr};
    torch::nn::MaxPool2d pool{nullptr};

    explicit HiveCNNImpl(std::string checkpoint = "model_checkpoint.pt")
        : checkpoint_file(std::move(checkpoint)),
          pool(torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2).stride(2)))
    {

        // Build conv layers
        auto conv =  torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(16, 24, /*kernel_size=*/3).padding(1).bias(true)),
            torch::nn::BatchNorm2d(24),
            torch::nn::Functional(torch::relu),

            torch::nn::Conv2d(torch::nn::Conv2dOptions(24, 32, /*kernel_size=*/3).padding(1).bias(true)),
            torch::nn::BatchNorm2d(32),
            torch::nn::Functional(torch::relu),

            torch::nn::Conv2d(torch::nn::Conv2dOptions(32, 48, /*kernel_size=*/3).padding(1).bias(true)),
            torch::nn::BatchNorm2d(48),
            torch::nn::Functional(torch::relu)
        );
        conv_layers = register_module("conv_layers", conv);

        // Fully connected layers
        fc_layers = register_module("fc_layers", torch::nn::Sequential(

            torch::nn::Linear(512 * 48, 2048),
            torch::nn::Functional(torch::relu),

            torch::nn::Linear(2048, 512),
            torch::nn::Functional(torch::relu),

            torch::nn::Linear(512, 64),
            torch::nn::Functional(torch::relu),

            torch::nn::Linear(64, 1)
        ));

        register_module("pool", pool);
    }

    torch::Tensor forward(torch::Tensor x);

    void save_model(const std::shared_ptr<torch::optim::Optimizer>& optimizer = nullptr) const;

    void load_model(const std::shared_ptr<torch::optim::Optimizer>& optimizer = nullptr);

};
TORCH_MODULE(HiveCNN);

Result resultOf(const Context* context);
float negamax_net(Context_t* context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t* bestMove, HiveCNN& net);

#endif //HIVECNN_H