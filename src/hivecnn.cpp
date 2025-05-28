//
// Created by filip on 15/05/2025.
//

#include "hivecnn.h"

// HiveCNN
torch::Tensor HiveCNNImpl::forward(torch::Tensor x) {
    x = x.to(torch::kFloat);
    x = conv_layers->forward(x);
    x = pool->forward(x);
    x = x.view({x.size(0), -1});
    x = fc_layers->forward(x);

    // Optional activation:
    // x = torch::tanh(x);

    return x;
}

void HiveCNNImpl::save_model(const std::shared_ptr<torch::optim::Optimizer>& optimizer) const {
    torch::serialize::OutputArchive archive;
    this->save(archive);
    optimizer->save(archive);
    archive.save_to(checkpoint_file);
}

void HiveCNNImpl::load_model(const std::shared_ptr<torch::optim::Optimizer>& optimizer) {
    torch::serialize::InputArchive archive;
    archive.load_from(checkpoint_file);
    this->load(archive);
    if (optimizer) {
        optimizer->load(archive);
    }
}

float evaluate(const Context_t* context, HiveCNN& model, const bool isWhiteTurn) {

    torch::Tensor x;
    Processor::getTensor(context, x);
    float y = model->forward(x).item().toFloat();
    y *= !isWhiteTurn ? -1 : 1;
    if (y > 1) return 1;
    if (y < -1) return -1;
    return y;

}

float negamax_net(Context_t* context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t* bestMove, HiveCNN& net) {

    if (depth >= maxDepth) {
        return evaluate(context, net, true);
    }

    const GameStatus_t gameStat = getGameStatus(context);
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    Piece_t* moves;
    uint_fast8_t mSize = 0;
    getMoves(context, &moves, &mSize);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    for (uint_fast8_t i = 0; i < mSize; i++) {
        Context_t newContext;
        initContext(&newContext);
        copyContext(context, &newContext);

        manageMove(&newContext, &moves[i]);
        if ((tmp = negamax(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove)) > maxVal) {
            maxVal = tmp;
            curBestMove = moves[i];
        }

        cleanContext(&newContext);
    }

    if (mSize == 0) {
        Context_t newContext;
        initContext(&newContext);
        copyContext(context, &newContext);

        manageMove(&newContext, NULL);
        maxVal = negamax(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove);

        cleanContext(&newContext);
    }


    if (depth == 0) {
        *bestMove = curBestMove;
        return 0;
    } return -maxVal;

}

Result resultOf(const Context* context) {

    const GameStatus_t gameStat = context->gameStatus;
    if (gameStat == WHITE_WON)
        return Result::RESULT_WHITE_WON;
    if (gameStat == BLACK_WON)
        return Result::RESULT_BLACK_WON;
    return Result::RESULT_DRAW;

}