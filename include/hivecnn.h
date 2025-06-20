//
// Created by filip on 15/05/2025.
//

#ifndef HIVECNN_H
#define HIVECNN_H

#include <torch/torch.h>
#include <preprocessor.h>
#include <iostream>
#include <filesystem>

struct HiveCNNImpl : torch::nn::Module
{
    std::string checkpoint_file;
    torch::nn::Sequential conv_layers{nullptr}, fc_layers{nullptr};
    torch::nn::MaxPool2d pool{nullptr};

    explicit HiveCNNImpl(std::string checkpoint = "model_checkpoint.pt")
        : checkpoint_file(std::move(checkpoint)),
          pool(torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2).stride(2)))
    {
        // Build conv layers
        auto conv = torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(16, 24, /*kernel_size=*/2).padding(1).bias(true)),
            torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2)),

            torch::nn::Conv2d(torch::nn::Conv2dOptions(24, 32, /*kernel_size=*/2).padding(1).bias(true)),
            torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2)),

            torch::nn::Conv2d(torch::nn::Conv2dOptions(32, 48, /*kernel_size=*/2).padding(1).bias(true)),
            torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2))
        );
        conv_layers = register_module("conv_layers", conv);

        // Fully connected layers
        fc_layers = register_module("fc_layers", torch::nn::Sequential(

                                        torch::nn::Linear(288, 128),
                                        torch::nn::Dropout(0.2),
                                        torch::nn::Functional(torch::relu),

                                        torch::nn::Linear(128, 32),
                                        torch::nn::Dropout(0.2),
                                        torch::nn::Functional(torch::relu),

                                        torch::nn::Linear(32, 8),
                                        torch::nn::Dropout(0.05),
                                        torch::nn::Functional(torch::relu)
                                    ));

        register_module("pool", pool);
    }

    torch::Tensor forward(torch::Tensor x);

    void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const;

    void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr);
};

TORCH_MODULE(HiveCNN);

Result resultOf(const Context *context);
float negamax_net(const Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, HiveCNN &net);
void testAgainstRandom();

#endif //HIVECNN_H
