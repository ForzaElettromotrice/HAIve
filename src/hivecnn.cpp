//
// Created by filip on 15/05/2025.
//

#include "hivecnn.h"

#include <filesystem>

#include "heuristics.h"

// HiveNet
HiveNet::HiveNet() {

}

HiveNet::~HiveNet() {

}


// HiveCNN
float HiveCNNImpl::forward(const Context_t* context) {
    torch::Tensor x;
    Processor::boardToTensor(context->idToPos, x);

    x = x.to(torch::kFloat);
    x = conv_layers->forward(x);
    x = pool->forward(x);
    x = x.view({1, -1});
    x = fc_layers->forward(x);
    x = x.mean();

    // Optional activation:
    // x = torch::tanh(x);

    return x.item().toFloat();
}

void HiveCNNImpl::save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer) const
{
    torch::serialize::OutputArchive archive;
    std::filesystem::path path(checkpoint_file);
    auto parent_dir = path.parent_path();
    if (!parent_dir.empty() && !std::filesystem::exists(parent_dir))
    {
        std::filesystem::create_directories(parent_dir);
    }

    this->save(archive);
    optimizer->save(archive);
    archive.save_to(checkpoint_file);
}

void HiveCNNImpl::load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer)
{
    torch::serialize::InputArchive archive;
    archive.load_from(checkpoint_file);
    this->load(archive);
    if (optimizer)
    {
        optimizer->load(archive);
    }
}

// HiveCNNEnhanced

float HiveCNNEnhancedImpl::forward(const Context_t *context)
{
    torch::Tensor x;
    torch::Tensor y = torch::zeros(paramSize);
    Processor::boardToTensor(context->idToPos, x);
    setHeuristicParams(context, y);

    x = x.to(torch::kFloat);
    x = x.unsqueeze(0);
    x = conv_layers->forward(x);
    x = x.view({1, -1});

    y = y.unsqueeze(0);
    x = torch::cat({x, y}, 1);
    x = fc_layers->forward(x);

    // Optional activation:
    // x = torch::tanh(x);

    return x.item().toFloat();
}

void HiveCNNEnhancedImpl::save_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer) const
{
    torch::serialize::OutputArchive archive;
    const std::filesystem::path path(checkpoint_file);
    auto parent_dir = path.parent_path();
    if (!parent_dir.empty() && !std::filesystem::exists(parent_dir))
    {
        std::filesystem::create_directories(parent_dir);
    }

    this->save(archive);
    optimizer->save(archive);
    archive.save_to(checkpoint_file);
}

void HiveCNNEnhancedImpl::load_model(const std::shared_ptr<torch::optim::Optimizer> &optimizer)
{
    torch::serialize::InputArchive archive;
    archive.load_from(checkpoint_file);
    this->load(archive);
    if (optimizer)
    {
        optimizer->load(archive);
    }
}

// Other stuff

float evaluate(const Context_t *context, HiveNet &model, const bool isWhiteTurn)
{
    float y = model.forward(context);
    y *= !isWhiteTurn ? -1 : 1;
    if (y > 1) return 1;
    if (y < -1) return -1;
    return y;
}

float negamax_net(const Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, HiveNet &net)
{
    if (depth >= maxDepth)
    {
        return evaluate(context, net, true);
    }

    const GameStatus_t gameStat = getGameStatus(context);
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    Piece_t *moves[15];
    getMoves(context, moves);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    bool moved = false;

    for (uint_fast8_t piece = B_QUEEN; piece < MOVES_ARRAYS; piece++)
    {
        for (uint16_t i = 0; moves[piece][i].id != NULLPIECE; i++)
        {
            Context_t newContext;
            copyContext(context, &newContext);

            moved = true;
            addOurMove(&newContext, &moves[piece][i]);
            if ((tmp = negamax_net(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, net)) > maxVal)
            {
                maxVal = tmp;
                curBestMove = moves[piece][i];
            }

            cleanContext(&newContext);
        }
    }

    if (!moved)
    {
        Context_t newContext;
        copyContext(context, &newContext);

        addOurMove(&newContext, &pass);
        maxVal = negamax_net(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, net);

        cleanContext(&newContext);
    }

    freeMoves(moves);

    if (depth == 0)
    {
        *bestMove = curBestMove;
        return 0;
    }
    return -maxVal;
}

float negamax_heuristic(Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, const std::function<float(Context_t *)>& heuristicFunc)
{
    if (depth >= maxDepth)
    {
        return heuristicFunc(context);
    }

    const GameStatus_t gameStat = getGameStatus(context);
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    Piece_t *moves[15];
    getMoves(context, moves);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const uint_fast8_t end = start + 14;
    bool moved = false;

    Context_t newContext;
    for (uint_fast8_t piece = 0; piece < MOVES_ARRAYS; piece++) {
        for (uint16_t i = 0; moves[piece][i].id != NULLPIECE; i++){
            if (i >= 120)
            {
                logE(stderr, "Too many moves!\n");
            }
            copyContext(context, &newContext);

            moved = true;
            addOurMove(&newContext, &moves[piece][i]);
            if ((tmp = negamax_heuristic(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc)) > maxVal)
            {
                maxVal = tmp;
                curBestMove = moves[piece][i];
            }
            cleanContext(&newContext);
        }
    }

    if (!moved)
    {
        copyContext(context, &newContext);

        addOurMove(&newContext, &pass);
        maxVal = negamax_heuristic(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc);
        cleanContext(&newContext);
    }

    freeMoves(moves);

    if (depth == 0)
    {
        *bestMove = curBestMove;
        return 0;
    }
    return -maxVal;
}

float negamax_heuristic_ab(Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, const std::function<float(Context_t *)>& heuristicFunc, float alpha, float beta)
{
    if (depth >= maxDepth)
    {
        return heuristicFunc(context) * (isWhiteTurn ? 1 : -1);
    }

    const GameStatus_t gameStat = getGameStatus(context);
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    Piece_t *moves[15];
    getMoves(context, moves);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const uint_fast8_t end = start + 14;
    bool moved = false;

    Context_t newContext;
    for (uint_fast8_t piece = 0; piece < MOVES_ARRAYS; piece++) {
        for (uint16_t i = 0; moves[piece][i].id != NULLPIECE; i++){
            if (i >= 120)
            {
                logE(stderr, "Too many moves!\n");
            }
            copyContext(context, &newContext);

            moved = true;
            addOurMove(&newContext, &moves[piece][i]);
            tmp = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha);
            cleanContext(&newContext);
            if (tmp > maxVal)
            {
                maxVal = tmp;
                curBestMove = moves[piece][i];
            }
            if (maxVal > alpha)
                alpha = maxVal;
            if (alpha >= beta)
                break;
        }
    }

    if (!moved)
    {
        copyContext(context, &newContext);

        addOurMove(&newContext, &pass);
        maxVal = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha);
    }

    freeMoves(moves);

    if (depth == 0)
    {
        *bestMove = curBestMove;
        return 0;
    }
    return maxVal;
}


Result resultOf(const Context *context)
{
    const GameStatus_t gameStat = context->gameStatus;
    if (gameStat == WHITE_WON)
        return Result::RESULT_WHITE_WON;
    if (gameStat == BLACK_WON)
        return Result::RESULT_BLACK_WON;
    return Result::RESULT_DRAW;

}

bool isPass(Piece_t **moves) {

    for (uint8_t i = 0; i < MOVES_ARRAYS; i++) {
        if (moves[i][0].id != NULLPIECE)
            return false;
    } return true;

}

bool battleAgainstRandom(bool areWeWhite) {

    Context_t context; Piece_t bestMove; Piece_t *moves[15];
    initContext(&context);

    while (!isContextEnded(&context))
    {
        if ((areWeWhite && context.curColor == WHITE) || (!areWeWhite && context.curColor == BLACK))
        {
            negamax_heuristic_ab(&context, 0, 2, context.curColor == WHITE, &bestMove, mzingaHeuristic, -1, 1);
            addOurMove(&context, &bestMove);
        } else
        {
            getMoves(&context, moves);
            uint_fast8_t chosenPiece;
            uint16_t chosenMove;
            if (context.idToPos[context.curColor == WHITE ? W_QUEEN : B_QUEEN].z == -1)
            {
                chosenPiece = 14;
            } else {
                if (isPass(moves)) {
                    addOurMove(&context, &pass);
                    freeMoves(moves);
                    continue;
                }
                else {
                    do {
                        chosenPiece = rand() % MOVES_ARRAYS;
                    } while (moves[chosenPiece][0].id == NULLPIECE);
                }
            }
            chosenMove = rand() % getMovesSize(moves[chosenPiece]);
            addOurMove(&context, &moves[chosenPiece][chosenMove]);
            freeMoves(moves);
        }
    }

    const GameStatus_t gameStat = getGameStatus(&context);
    cleanContext(&context);

    if (areWeWhite && gameStat == WHITE_WON)
        return true;
    if (!areWeWhite && gameStat == BLACK_WON)
        return true;
    return false;
}

void testAgainstRandom()
{
    int played = 0;
    int won = 0;
    // srand(time(NULL));

    for (size_t i = 0; i < 100; i++)
    {
        if (battleAgainstRandom(i % 2 == 0))
            won++;
        played++;

        printf("%d / %d", won, played);
    }
}
