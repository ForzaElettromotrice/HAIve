//
// Created by filip on 15/05/2025.
//

#ifndef HIVECNN_H
#define HIVECNN_H

#include <torch/torch.h>
#include <preprocessor.h>
#include <iostream>
#include <filesystem>
extern "C" {
    #include <logger.h>
}
struct HiveNet : torch::nn::Module {

    HiveNet();
    ~HiveNet();
    virtual float forward(const Context_t* context) = 0;

};

struct HiveCNNImpl : HiveNet
{
    std::string checkpoint_file;
    torch::nn::Sequential conv_layers{nullptr}, fc_layers{nullptr};
    torch::nn::MaxPool2d pool{nullptr};

    explicit HiveCNNImpl(std::string checkpoint = "model_checkpoint.pt")
        : checkpoint_file(std::move(checkpoint)),
          pool(torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2).stride(2)))
    {
        // Build conv layers
        conv_layers = register_module("conv_layers", torch::nn::Sequential(
            torch::nn::Conv2d(torch::nn::Conv2dOptions(16, 24, /*kernel_size=*/2).padding(1).bias(true)),
            torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2)),

            torch::nn::Conv2d(torch::nn::Conv2dOptions(24, 32, /*kernel_size=*/2).padding(1).bias(true)),
            torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2)),


            torch::nn::Conv2d(torch::nn::Conv2dOptions(32, 48, /*kernel_size=*/2).padding(1).bias(true)),
            torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2))
            ));

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

    float forward(const Context_t* context) override;
    void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const;
    void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr);
};

TORCH_MODULE(HiveCNN);

struct HiveCNNEnhancedImpl : HiveNet {
    std::string checkpoint_file;
    torch::nn::Sequential conv_layers{nullptr};
    torch::nn::Linear fc1{nullptr};
    uint16_t num_channels = 64;

    explicit HiveCNNEnhancedImpl(std::string checkpoint = "model_enh_checkpoint.pt")
        : checkpoint_file(std::move(checkpoint)) {

        conv_layers = register_module("conv_layers", torch::nn::Sequential(
                torch::nn::Conv2d(torch::nn::Conv2dOptions(16, num_channels, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(num_channels)),
                torch::nn::Functional(torch::relu),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(num_channels, num_channels, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(num_channels)),
                torch::nn::Functional(torch::relu),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(num_channels, num_channels, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(num_channels)),
                torch::nn::Functional(torch::relu),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(num_channels, num_channels, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(num_channels)),
                torch::nn::Functional(torch::relu),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(num_channels, 256, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(num_channels)),
                torch::nn::Functional(torch::relu),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(256, 16, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(num_channels)),
                torch::nn::Functional(torch::relu)
            ));

        // Fully connected layers
        fc1 = register_module("fc1", torch::nn::Linear(288, 1));

        // TODO: Add an action-prob layer?
    }

    float forward(const Context_t* context) override;
    void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const;
    void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr);
};

TORCH_MODULE(HiveCNNEnhanced);

Result resultOf(const Context *context);
float negamax_net(const Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, HiveCNN &net);
void testAgainstRandom();

#endif //HIVECNN_H
