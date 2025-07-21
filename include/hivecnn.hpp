//
// Created by filip on 15/05/2025.
//

#ifndef HIVECNN_H
#define HIVECNN_H

#include <torch/torch.h>
#include <preprocessor.hpp>
#include <filesystem>
#include <tree.hpp>

extern "C" {
#include <logger.h>
}

struct HiveNet : torch::nn::Module
{
    HiveNet();
    virtual ~HiveNet()
    {
    };
    virtual torch::Tensor forward(const Context_t *context) = 0;
    virtual void batchForward(BatchContext_t *batchContext) = 0;
    virtual void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const = 0;
    virtual void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) = 0;
};

struct HiveCNNImpl : HiveNet
{
    std::string checkpoint_file;
    torch::nn::Sequential conv_layers{nullptr}, fc_layers{nullptr};
    torch::nn::MaxPool2d pool{nullptr};
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    explicit HiveCNNImpl(std::string checkpoint = "model_checkpoint.pt")
        : checkpoint_file(std::move(checkpoint)),
          pool(torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2).stride(2)))
    {
        // Build conv layers
        conv_layers = register_module("conv_layers", torch::nn::Sequential(
                                          torch::nn::Conv2d(torch::nn::Conv2dOptions(18, 32, /*kernel_size=*/3).padding(1)),
                                          torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2)),
                                          torch::nn::LeakyReLU(),

                                          torch::nn::Conv2d(torch::nn::Conv2dOptions(32, 64, /*kernel_size=*/3).padding(1)),
                                          torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2)),
                                          torch::nn::LeakyReLU(),
                                          torch::nn::Dropout(0.2),

                                          torch::nn::Conv2d(torch::nn::Conv2dOptions(64, 128, /*kernel_size=*/3).padding(1)),
                                          torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2)),
                                          torch::nn::LeakyReLU(),
                                          torch::nn::Dropout(0.2),

                                            torch::nn::Conv2d(torch::nn::Conv2dOptions(128, 256, /*kernel_size=*/3).stride(2).padding(1).bias(false)),
                                            torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(256)),
                                            torch::nn::LeakyReLU(),
                                            torch::nn::Dropout(0.3),

                                            torch::nn::Conv2d(torch::nn::Conv2dOptions(256, 512, /*kernel_size=*/2).stride(1).padding(1).bias(false)),
                                            torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(512)),
                                            torch::nn::ReLU(),
                                            torch::nn::Dropout(0.3)
                                      ));

        // Fully connected layers
        fc_layers = register_module("fc_layers", torch::nn::Sequential(

                                        torch::nn::Linear(512, 32),
                                        torch::nn::ReLU(),
                                        torch::nn::Dropout(0.2),

                                        torch::nn::Linear(32, 1)
                                    ));
    }

    torch::Tensor forward(const Context_t *context) override;
    void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const override;
    void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) override;
    void batchForward(BatchContext_t *batchContext) override;
};

TORCH_MODULE(HiveCNN);

struct HiveCNNEnhancedImpl : HiveNet
{
    std::string checkpoint_file;
    torch::nn::Sequential conv_layers{nullptr};
    torch::nn::Sequential fc_layers{nullptr};
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    explicit HiveCNNEnhancedImpl(std::string checkpoint = "model_enh_checkpoint.pt")
        : checkpoint_file(std::move(checkpoint))
    {
        conv_layers = register_module("conv_layers", torch::nn::Sequential(
            // First conv block (18 -> 32)
            torch::nn::Conv2d(torch::nn::Conv2dOptions(18, 32, 3).stride(1).padding(1).bias(false)),
            torch::nn::BatchNorm2d(32),
            torch::nn::LeakyReLU(),

            // Second conv block (32 -> 64)
            torch::nn::Conv2d(torch::nn::Conv2dOptions(32, 64, 3).stride(1).padding(1).bias(false)),
            torch::nn::BatchNorm2d(64),
            torch::nn::LeakyReLU(),
            torch::nn::Dropout(0.2),

            // Third conv block (64 -> 128)
            torch::nn::Conv2d(torch::nn::Conv2dOptions(64, 128, 3).stride(1).padding(1).bias(false)),
            torch::nn::BatchNorm2d(128),
            torch::nn::LeakyReLU(),
            torch::nn::Dropout(0.2),

            // Fourth conv block (128 -> 256)
            torch::nn::Conv2d(torch::nn::Conv2dOptions(128, 256, 3).stride(2).padding(1).bias(false)),
            torch::nn::BatchNorm2d(256),
            torch::nn::LeakyReLU(),
            torch::nn::Dropout(0.3),

            // Fifth conv block (256 -> 512)
            torch::nn::Conv2d(torch::nn::Conv2dOptions(256, 512, 2).stride(1).padding(1).bias(false)),
            torch::nn::BatchNorm2d(512),
            torch::nn::ReLU(),
            torch::nn::Dropout(0.3)
        ));

        // Fully connected layers
        fc_layers = register_module("fc1", torch::nn::Sequential(
            torch::nn::Linear(torch::nn::LinearOptions(512, 32).bias(false)),
            torch::nn::LeakyReLU(),
            torch::nn::Dropout(0.2),
            torch::nn::Linear(torch::nn::LinearOptions(32, 1).bias(false))
        ));

        // TODO: Add an action-prob layer?
    }

    torch::Tensor forward(const Context_t *context) override;
    void batchForward(BatchContext_t *batchContext) override;
    void save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) const override;
    void save_partial(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr, int part = 0) const;
    void load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer = nullptr) override;
};

TORCH_MODULE(HiveCNNEnhanced);

Result resultOf(const Context *context);
float negamax_net(const Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, HiveNet &net);
void testAgainstRandom();
void bestMove(const Context_t *context);

#endif //HIVECNN_H
