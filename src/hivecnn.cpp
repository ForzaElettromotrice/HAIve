//
// Created by filip on 15/05/2025.
//

#include "hivecnn.h"

#include <filesystem>

#include "heuristics.h"

// HiveNet
HiveNet::HiveNet() {

}

// HiveCNN
torch::Tensor HiveCNNImpl::forward(const Context_t* context) {
    torch::Tensor x;
    Processor::boardToTensor(context->idToPos, x);

    x = x.clone().set_requires_grad(true);
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

torch::Tensor HiveCNNEnhancedImpl::forward(const Context_t *context)
{
    torch::Tensor x;
    torch::Tensor y = torch::zeros(paramSize);
    Processor::boardToTensor(context->idToPos, x);
    setHeuristicParams(context, y);

    x = x.clone().set_requires_grad(true);
    x = x.to(torch::kFloat);
    if (torch::cuda::is_available())
        x = x.to(torch::kCUDA);
    x = x.unsqueeze(0);
    x = conv_layers->forward(x);
    x = x.mean({2, 3});
    x = x.view({1, -1});

    y = y.unsqueeze(0);
    x = torch::cat({x, y}, 1);
    x = fc_layers->forward(x);

    // Optional activation:
    // x = torch::tanh(x);

    return x;
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
    float y = model.forward(context).item<float>();
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

float negamax_heuristic_ab(
    Context_t *context, const int depth, const int maxDepth,
    const bool isWhiteTurn,
    Piece_t *bestMove, const std::function<float(Context_t *)>& heuristicFunc,
    float alpha, float beta,
    const std::chrono::high_resolution_clock::time_point &startTime, int maxDurationMs
    )
{
    auto now = std::chrono::high_resolution_clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    if (elapsed >= maxDurationMs) {
        return -9999.0f;
    }

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

            auto check = std::chrono::high_resolution_clock::now();
            int elapsedLoop = std::chrono::duration_cast<std::chrono::milliseconds>(check - startTime).count();
            if (elapsedLoop >= maxDurationMs) {
                freeMoves(moves);
                return -9999.0f; // Early abort
            }

            moved = true;
            addOurMove(&newContext, &moves[piece][i]);
            tmp = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha, startTime, maxDurationMs);
            cleanContext(&newContext);

            if (tmp == -9999.0f) {
                freeMoves(moves);
                return -9999.0f; // propagate timeout
            }

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
        maxVal = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha, startTime, maxDurationMs);

        cleanContext(&newContext);
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
    auto model = HiveCNNEnhanced("model_checkpoint");
    model->eval();
    initContext(&context);

    while (!isContextEnded(&context))
    {
        if (context.turn % 10 == 0)
            printf("Turn %d\n", context.turn);
        if ((areWeWhite && context.curColor == WHITE) || (!areWeWhite && context.curColor == BLACK))
        {
            negamax_heuristic_ab(&context, 0, 2, context.curColor == WHITE, &bestMove, mzingaHeuristic, -2, 2, std::chrono::high_resolution_clock::now(), 1000000);
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
    srand(time(NULL));

    for (size_t i = 0; i < 100; i++)
    {
        if (battleAgainstRandom(i % 2 == 0))
            won++;
        played++;

        printf("%d / %d\n", won, played);
    }
}

void bestMove(const Context_t *originalContext)
{
    //TODO: prendi il figlio con valore maggiore dall'albero

    const int MAX_TIME_MS = 4900;
    auto startTime = std::chrono::high_resolution_clock::now();

    Piece_t finalBestMove = pass;

    Context_t ourContext;
    copyContext(originalContext, &ourContext);
    Hashmap_t* hashtable; initHashmap(hashtable, 8192);

    for (int depth = 1; depth <= 100; ++depth)
    {
        auto now = std::chrono::high_resolution_clock::now();
        long int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        if (elapsed >= MAX_TIME_MS)
            break;

        Piece_t currentBestMove = pass;

        float res = negamax_heuristic_ab(
            &ourContext,
            0,                // current depth
            depth,            // maxDepth
            ourContext.curColor == WHITE,
            &currentBestMove,
            mzingaHeuristic,    // Your heuristic lambda or function
            -1.0f, 1.0f,       // alpha-beta initial bounds
            startTime, MAX_TIME_MS
        );

        // If we finished cleanly before timeout, update final best
        auto after = std::chrono::high_resolution_clock::now();
        long int elapsedAfter = std::chrono::duration_cast<std::chrono::milliseconds>(after - startTime).count();
        if (res == -9999.0f)
            break;
        if (elapsedAfter < MAX_TIME_MS) {
            finalBestMove = currentBestMove;
        }
    }

    printMove(originalContext, finalBestMove);

}
