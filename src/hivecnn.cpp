//
// Created by filip on 15/05/2025.
//

#include "hivecnn.h"

#include <filesystem>

#include "heuristics.h"

// HiveCNN
torch::Tensor HiveCNNImpl::forward(torch::Tensor x) {
    x = x.to(torch::kFloat);
    x = conv_layers->forward(x);
    x = pool->forward(x);
    x = x.view({1, -1});
    x = fc_layers->forward(x);
    x = x.mean();

    // Optional activation:
    // x = torch::tanh(x);

    return x;
}

void HiveCNNImpl::save_model(const std::shared_ptr<torch::optim::Optimizer>& optimizer) const {
    torch::serialize::OutputArchive archive;
    std::filesystem::path path(checkpoint_file);
    auto parent_dir = path.parent_path();
    if (!parent_dir.empty() && !std::filesystem::exists(parent_dir)) {
        std::filesystem::create_directories(parent_dir);
    }

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
    Processor::boardToTensor(context->idToPos, x);
    float y = model->forward(x).item().toFloat();
    y *= !isWhiteTurn ? -1 : 1;
    if (y > 1) return 1;
    if (y < -1) return -1;
    return y;

}

float negamax_net(const Context_t* context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t* bestMove, HiveCNN& net) {

    if (depth >= maxDepth) {
        return evaluate(context, net, true);
    }

    const GameStatus_t gameStat = getGameStatus(context);
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    Piece_t** moves;
    getMoves(context, &moves[0]);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const uint_fast8_t end = start + 14; bool moved = false;

    for (uint_fast8_t piece = context->curColor == WHITE ? W_QUEEN : B_QUEEN; piece < end; piece++) {
        for (uint16_t i = 0; moves[piece][i].id != NULLPIECE; i++){
            Context_t newContext;
            initContext(&newContext);
            copyContext(context, &newContext);

            moved = true;
            addOurMove(&newContext, moves[piece][i]);
            if ((tmp = negamax_net(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, net)) > maxVal) {
                maxVal = tmp;
                curBestMove = moves[piece][i];
            }

            cleanContext(&newContext);
        }
    }

    if (!moved) {
        Context_t newContext;
        initContext(&newContext);
        copyContext(context, &newContext);

        addOurMove(&newContext, pass);
        maxVal = negamax_net(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, net);

        cleanContext(&newContext);
    }


    if (depth == 0) {
        *bestMove = curBestMove;
        return 0;
    } return -maxVal;

}

float negamax_heuristic(const Context_t* context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t* bestMove, std::function<float(const Context_t*)> heuristicFunc) {

    if (depth >= maxDepth) {
        return heuristicFunc(context);
    }

    const GameStatus_t gameStat = getGameStatus(context);
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    Piece_t** moves;
    getMoves(context, &moves[0]);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const uint_fast8_t end = start + 14; bool moved = false;

    for (uint_fast8_t piece = context->curColor == WHITE ? W_QUEEN : B_QUEEN; piece < end; piece++) {
        for (uint16_t i = 0; moves[piece][i].id != NULLPIECE; i++){
            Context_t newContext;
            initContext(&newContext);
            copyContext(context, &newContext);

            moved = true;
            addOurMove(&newContext, moves[piece][i]);
            if ((tmp = negamax_heuristic(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc)) > maxVal) {
                maxVal = tmp;
                curBestMove = moves[piece][i];
            }

            cleanContext(&newContext);
        }
    }

    if (!moved) {
        Context_t newContext;
        initContext(&newContext);
        copyContext(context, &newContext);

        addOurMove(&newContext, pass);
        maxVal = negamax_heuristic(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc);

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

bool battleAgainstRandom(bool areWeWhite) {

    Context_t context; Piece_t bestMove; Piece_t** moves;
    initContext(&context); resetContext(&context);
    srand(time(NULL));

    while (!isContextEnded(&context)) {

        if ( (areWeWhite && context.curColor == WHITE) || (!areWeWhite && context.curColor == BLACK) ) {

            negamax_heuristic(&context, 0, 2, context.curColor == WHITE, &bestMove, mzingaHeuristic);
            addOurMove(&context, bestMove);

        } else {
            getMoves(&context, &moves[0]);
            uint_fast8_t chosenPiece; uint16_t chosenMove;
            do {
                chosenPiece = rand() % MOVES_ARRAYS;
            } while (moves[chosenPiece][0].id == NULLPIECE);
            chosenMove = rand() % getMovesSize(moves[chosenPiece]);
            addOurMove(&context, moves[chosenPiece][chosenMove]);
        }

    }

    if (areWeWhite && context.gameStatus == WHITE_WON)
        return true;
    if (!areWeWhite && context.gameStatus == BLACK_WON)
        return true;
    return false;

}

void testAgainstRandom() {

    int played = 0;
    int won = 0;

    for (size_t i = 0; i < 100; i++) {
        if (battleAgainstRandom(i % 2 == 0))
            won++;
        played++;

        printf("%d / %d", won, played);
    }

}