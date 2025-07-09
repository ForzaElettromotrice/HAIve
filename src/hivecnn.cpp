//
// Created by filip on 15/05/2025.
//

#include "hivecnn.hpp"

#include <filesystem>

#include "heuristics.hpp"
#include "tree.hpp"

// HiveNet
HiveNet::HiveNet()
{
}

// HiveCNN
torch::Tensor HiveCNNImpl::forward(const Context_t *context)
{
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
    //torch::Tensor y = torch::zeros(paramSize);
    Processor::boardToTensor(context->idToPos, x);
    // setHeuristicParams(context, y);


    x = x.clone().set_requires_grad(true);
    x = x.to(torch::kFloat);
    if (torch::cuda::is_available())
        x = x.to(torch::kCUDA);
    x = x.unsqueeze(0);
    // y = y.unsqueeze(0);

    pthread_mutex_lock(&this->mutex);
    x = conv_layers->forward(x);
    x = x.mean({2, 3});
    x = x.view({1, -1});

    // x = torch::cat({x, y}, 1);
    x = fc_layers->forward(x);
    pthread_mutex_unlock(&this->mutex);
    // Optional activation:
    // x = torch::tanh(x);

    return x;
}

void HiveCNNEnhancedImpl::batchForward(BatchContext_t *batchContext) {

    const auto options = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor x = torch::zeros({batchContext->count, sizeLayer, BOARD_Y / 2, BOARD_X}, options);
    pthread_t threads[batchContext->count];

    for (uint8_t i = 0; i < batchContext->count; i++)
    {
        ProcessorArgs_t args = {x[i], batchContext->nodes[i]->context.idToPos};
        pthread_create(&threads[i], nullptr, Processor::boardToTensor_mt, &args);
    }

    for (uint8_t i = 0; i < batchContext->count; i++) {
        pthread_join(threads[i], nullptr);
    }

    x = x.clone().set_requires_grad(true);
    x = x.to(torch::kFloat);
    if (torch::cuda::is_available())
        x = x.to(torch::kCUDA);

    pthread_mutex_lock(&this->mutex);

    x = conv_layers->forward(x);
    x = x.mean({2, 3});
    x = x.view({1, -1});
    x = fc_layers->forward(x);

    pthread_mutex_unlock(&this->mutex);

    for (uint8_t i = 0; i < batchContext->count; i++) {
        batchContext->result[i] = x[i].item<float>();
    }

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

void HiveCNNEnhancedImpl::save_partial(const std::shared_ptr<torch::optim::Optimizer> &optimizer, int part) const
{
    torch::serialize::OutputArchive archive;
    const std::filesystem::path path(checkpoint_file + std::to_string(part) + ".pt");
    auto parent_dir = path.parent_path();
    if (!parent_dir.empty() && !std::filesystem::exists(parent_dir))
    {
        std::filesystem::create_directories(parent_dir);
    }

    this->save(archive);
    optimizer->save(archive);
    archive.save_to(checkpoint_file);
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
    Piece_t *moves;
    getMoves(context, &moves);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    bool moved = false;

    for (uint_fast8_t piece = B_QUEEN; piece < MOVES_ARRAYS; piece++)
    {
        for (uint16_t i = 0; moves[MMtA(piece, i)].id != NULLPIECE; i++)
        {
            Context_t newContext;
            copyContext(context, &newContext);

            moved = true;
            addOurMove(&newContext, &moves[MMtA(piece, i)]);
            if ((tmp = negamax_net(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, net)) > maxVal)
            {
                maxVal = tmp;
                curBestMove = moves[MMtA(piece, i)];
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

    free(moves);

    if (depth == 0)
    {
        *bestMove = curBestMove;
        return 0;
    }
    return -maxVal;
}

float negamax_heuristic(Context_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, const std::function<float(Context_t *)> &heuristicFunc)
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
    Piece_t *moves;
    getMoves(context, &moves);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const uint_fast8_t end = start + 14;
    bool moved = false;

    Context_t newContext;
    for (uint_fast8_t piece = 0; piece < MOVES_ARRAYS; piece++)
    {
        for (uint16_t i = 0; moves[MMtA(piece, i)].id != NULLPIECE; i++)
        {
            if (i >= 120)
            {
                logE(stderr, "Too many moves!\n");
            }
            copyContext(context, &newContext);

            moved = true;
            addOurMove(&newContext, &moves[MMtA(piece, i)]);
            if ((tmp = negamax_heuristic(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc)) > maxVal)
            {
                maxVal = tmp;
                curBestMove = moves[MMtA(piece, i)];
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

    free(moves);

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
    Piece_t *bestMove, const std::function<float(Context_t *)> &heuristicFunc,
    float alpha, float beta,
    const std::chrono::high_resolution_clock::time_point &startTime, int maxDurationMs,
    Hashmap_t *hashtable
)
{
    auto now = std::chrono::high_resolution_clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    if (elapsed >= maxDurationMs)
    {
        return -9999.0f;
    }

    if (depth >= maxDepth)
    {
        const uint64_t hash = hashAll(context->board, context->idToPos, context->curColor);
        if (const auto result = static_cast<float *>(getByHash(hash, hashtable)); result != nullptr)
        {
            return *result;
        }
        const float res = heuristicFunc(context);
        setByHash(hash, &res, sizeof(float), hashtable);
        return res;
    }

    const GameStatus_t gameStat = getGameStatus(context);
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    Piece_t *moves;
    getMoves(context, &moves);
    float maxVal = -2, tmp;
    Piece_t curBestMove;

    const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const uint_fast8_t end = start + 14;
    bool moved = false;

    Context_t newContext;
    for (uint_fast8_t piece = 0; piece < MOVES_ARRAYS; piece++)
    {
        for (uint16_t i = 0; moves[MMtA(piece, i)].id != NULLPIECE; i++)
        {
            if (i >= 120)
            {
                logE(stderr, "Too many moves!\n");
            }
            copyContext(context, &newContext);

            auto check = std::chrono::high_resolution_clock::now();
            int elapsedLoop = std::chrono::duration_cast<std::chrono::milliseconds>(check - startTime).count();
            if (elapsedLoop >= maxDurationMs)
            {
                free(moves);
                return -9999.0f; // Early abort
            }

            moved = true;
            addOurMove(&newContext, &moves[MMtA(piece, i)]);
            tmp = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha, startTime, maxDurationMs, hashtable);
            cleanContext(&newContext);

            if (tmp == -9999.0f)
            {
                free(moves);
                return -9999.0f; // propagate timeout
            }

            if (tmp > maxVal)
            {
                maxVal = tmp;
                curBestMove = moves[MMtA(piece, i)];
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
        maxVal = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha, startTime, maxDurationMs, hashtable);

        cleanContext(&newContext);
    }

    free(moves);

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

bool isPass(Piece_t *moves)
{
    for (uint8_t i = 0; i < MOVES_ARRAYS; i++)
    {
        if (moves[MMtA(i, 0)].id != NULLPIECE)
            return false;
    }
    return true;
}

bool battleAgainstRandom(bool areWeWhite)
{
    Context_t context;
    Piece_t bestMove;
    Piece_t *moves;
    initContext(&context);

    Hashmap_t hashtable;
    initHashmap(&hashtable, 8192);

    while (!isContextEnded(&context))
    {
        if (context.turn % 10 == 0)
            printf("Turn %d\n", context.turn);
        if ((areWeWhite && context.curColor == WHITE) || (!areWeWhite && context.curColor == BLACK))
        {
            negamax_heuristic_ab(&context, 0, 2, context.curColor == WHITE, &bestMove, mzingaHeuristic, -2, 2, std::chrono::high_resolution_clock::now(), 1000000, &hashtable);
            addOurMove(&context, &bestMove);
        } else
        {
            getMoves(&context, &moves);
            uint_fast8_t chosenPiece;
            uint16_t chosenMove;
            if (context.idToPos[context.curColor == WHITE ? W_QUEEN : B_QUEEN].z == -1)
            {
                chosenPiece = 14;
            } else
            {
                if (isPass(moves))
                {
                    addOurMove(&context, &pass);
                    free(moves);
                    continue;
                } else
                {
                    do
                    {
                        chosenPiece = rand() % MOVES_ARRAYS;
                    } while (moves[MMtA(chosenPiece, 0)].id == NULLPIECE);
                }
            }
            chosenMove = rand() % getMovesSize(&moves[MMtA(chosenPiece, 0)]);
            addOurMove(&context, &moves[MMtA(chosenPiece, chosenMove)]);
            free(moves);
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

    // const int MAX_TIME_MS = 4900;
    // auto startTime = std::chrono::high_resolution_clock::now();
    //
    // Piece_t finalBestMove = pass;
    //
    // Context_t ourContext;
    // copyContext(originalContext, &ourContext);
    // Hashmap_t hashtable;
    // initHashmap(&hashtable, 8192);
    //
    // for (int depth = 1; depth <= 100; ++depth)
    // {
    //     auto now = std::chrono::high_resolution_clock::now();
    //     long int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    //     if (elapsed >= MAX_TIME_MS)
    //         break;
    //
    //     Piece_t currentBestMove = pass;
    //
    //     float res = negamax_heuristic_ab(
    //         &ourContext,
    //         0, // current depth
    //         depth, // maxDepth
    //         ourContext.curColor == WHITE,
    //         &currentBestMove,
    //         mzingaHeuristic, // Your heuristic lambda or function
    //         -2, 2, // alpha-beta initial bounds
    //         startTime, MAX_TIME_MS,
    //         &hashtable
    //     );
    //
    //     // If we finished cleanly before timeout, update final best
    //     auto after = std::chrono::high_resolution_clock::now();
    //     long int elapsedAfter = std::chrono::duration_cast<std::chrono::milliseconds>(after - startTime).count();
    //     if (res == -9999.0f)
    //         break;
    //     if (elapsedAfter < MAX_TIME_MS)
    //     {
    //         finalBestMove = currentBestMove;
    //     }
    // }

    const Piece_t *move = getBestChild();

    printMove(originalContext, move);
}
