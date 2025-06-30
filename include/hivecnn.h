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
    virtual ~HiveNet() {};
    virtual torch::Tensor forward(const Context_t* context) = 0;

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
            torch::nn::Conv2d(torch::nn::Conv2dOptions(18, 24, /*kernel_size=*/2).padding(1).bias(true)),
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

    torch::Tensor forward(const Context_t* context) override;
    void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const;
    void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr);
};

TORCH_MODULE(HiveCNN);

struct HiveCNNEnhancedImpl : HiveNet {
    std::string checkpoint_file;
    torch::nn::Sequential conv_layers{nullptr};
    torch::nn::Sequential fc_layers{nullptr};

    explicit HiveCNNEnhancedImpl(std::string checkpoint = "model_enh_checkpoint.pt")
        : checkpoint_file(std::move(checkpoint)) {

        conv_layers = register_module("conv_layers", torch::nn::Sequential(
                torch::nn::Conv2d(torch::nn::Conv2dOptions(18, 32, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(32)),
                torch::nn::ReLU(),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(32, 64, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(64)),
                torch::nn::ReLU(),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(64, 128, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(128)),
                torch::nn::ReLU(),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(128, 256, /*kernel_size=*/3).stride(2).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(256)),
                torch::nn::ReLU(),
                torch::nn::Dropout(0.1),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(256, 512, /*kernel_size=*/3).stride(2).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(512)),
                torch::nn::ReLU(),
                torch::nn::Dropout(0.1),

                torch::nn::Conv2d(torch::nn::Conv2dOptions(512, 128, /*kernel_size=*/3).stride(1).padding(1).bias(true)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(128)),
                torch::nn::ReLU(),

                torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2))
            ));

        // Fully connected layers
        fc_layers = register_module("fc1", torch::nn::Sequential(
            torch::nn::Linear(240, 16),
                torch::nn::ReLU(),
                torch::nn::Dropout(0.1),
                torch::nn::Linear(16, 1)
            ));

        // TODO: Add an action-prob layer?
    }

    torch::Tensor forward(const Context_t* context) override;
    void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const;
    void save_partial(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr, int part = 0) const;
    void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr);
};

TORCH_MODULE(HiveCNNEnhanced);

Result resultOf(const Context *context);
float negamax_net(const Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, HiveNet &net);
void testAgainstRandom();
void bestMove(const Context_t *context);

#endif //HIVECNN_H
