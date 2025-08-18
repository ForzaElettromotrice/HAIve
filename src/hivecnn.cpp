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
torch::Tensor HiveCNNImpl::forward(const HAIveContext_t *context)
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
    try
    {
        // Load JIT traced/scripted model
        torch::jit::script::Module module = torch::jit::load(checkpoint_file);

        // Copy the loaded model parameters to this model
        auto loaded_params = module.parameters();
        auto this_params = this->parameters();

        if (loaded_params.size() != this_params.size())
        {
            throw std::runtime_error("Parameter count mismatch between loaded JIT model and current model");
        }

        auto loaded_it = loaded_params.begin();
        auto this_it = this_params.begin();

        for (; loaded_it != loaded_params.end() && this_it != this_params.end(); ++loaded_it, ++this_it)
        {
            this_it->copy_(*loaded_it);
        }

        // Note: JIT models don't typically store optimizer state
        // If optimizer state loading is needed, it would require a separate file
        if (optimizer)
        {
            std::cerr << "Warning: Optimizer state cannot be loaded from JIT model. Optimizer state will be reset." << std::endl;
        }
    } catch (const std::exception &e)
    {
        std::cerr << "Error loading JIT model: " << e.what() << std::endl;
        throw;
    }
}

void HiveCNNImpl::batchForward(BatchContext_t *batchContext)
{
    const auto options = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor x = torch::zeros({batchContext->count, sizeLayer, BOARD_Y / 2, BOARD_X}, options);
    pthread_t threads[batchContext->count];
    ProcessorArgs_t args[batchContext->count];
    for (uint8_t i = 0; i < batchContext->count; i++)
    {
        args[i] = {x[i], batchContext->nodes[i]->context.idToPos};
        pthread_create(&threads[i], nullptr, Processor::boardToTensor_mt, &args);
    }

    for (uint8_t i = 0; i < batchContext->count; i++)
    {
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

    for (uint8_t i = 0; i < batchContext->count; i++)
    {
        batchContext->result[i] = x[i].item<float>();
    }
}


// HiveCNNEnhanced

torch::Tensor HiveCNNEnhancedImpl::forward(const HAIveContext_t *context)
{
    torch::Tensor x;
    Processor::boardToTensor(context->idToPos, x);

    x = x.clone().set_requires_grad(true);
    x = x.to(torch::kFloat);
    if (torch::cuda::is_available())
        x = x.to(torch::kCUDA);

    // If we loaded a JIT model, use it directly
    if (use_jit_model && jit_module.has_value())
    {
        pthread_mutex_lock(&this->mutex);

        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(x);

        auto result = jit_module.value().forward(inputs).toTensor();

        pthread_mutex_unlock(&this->mutex);
        return result;
    }

    // Original implementation for non-JIT models
    x = x.unsqueeze(0);

    pthread_mutex_lock(&this->mutex);
    x = conv_layers->forward(x);
    x = x.mean({2, 3});
    x = x.view({1, -1});
    x = fc_layers->forward(x);
    pthread_mutex_unlock(&this->mutex);

    return x;
}

void HiveCNNEnhancedImpl::batchForward(BatchContext_t *batchContext)
{
    const auto options = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor x = torch::zeros({batchContext->count, sizeLayer, BOARD_Y / 2, BOARD_X}, options);
    pthread_t threads[batchContext->count];
    ProcessorArgs_t args[batchContext->count];
    for (uint8_t i = 0; i < batchContext->count; i++)
    {
        args[i] = {x[i], batchContext->nodes[i]->context.idToPos};
        pthread_create(&threads[i], nullptr, Processor::boardToTensor_mt, &args[i]);
    }

    for (uint8_t i = 0; i < batchContext->count; i++)
    {
        pthread_join(threads[i], nullptr);
    }

    x = x.clone().set_requires_grad(true);
    x = x.to(torch::kFloat);
    if (torch::cuda::is_available())
        x = x.to(torch::kCUDA);

    pthread_mutex_lock(&this->mutex);

    if (use_jit_model && jit_module.has_value())
    {
        // Process each sample individually with JIT model
        for (uint8_t i = 0; i < batchContext->count; i++)
        {
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(x[i]);

            auto result = jit_module.value().forward(inputs).toTensor();
            batchContext->result[i] = result.item<float>();
        }
    } else
    {
        // Original batch processing
        x = conv_layers->forward(x);
        x = x.mean({2, 3});
        x = x.view({1, -1});
        x = fc_layers->forward(x);

        for (uint8_t i = 0; i < batchContext->count; i++)
        {
            batchContext->result[i] = x[i].item<float>();
        }
    }

    pthread_mutex_unlock(&this->mutex);
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
    std::string jit_file = checkpoint_file;
    if (jit_file.substr(jit_file.length() - 3) != ".jit")
    {
        jit_file = checkpoint_file.substr(0, checkpoint_file.find_last_of('.')) + ".jit";
    }

    try
    {
        if (std::filesystem::exists(jit_file))
        {
            jit_module = torch::jit::load(jit_file);
            use_jit_model = true;

            if (torch::cuda::is_available())
            {
                jit_module.value().to(torch::kCUDA);
            }

            if (optimizer)
            {
                std::cerr << "Warning: Optimizer state cannot be loaded from JIT model. Optimizer state will be reset." << std::endl;
            }
            return;
        }
    } catch (const std::exception &e)
    {
        std::cout << "Failed to load JIT model from " << jit_file << ": " << e.what() << std::endl;
        use_jit_model = false;
        jit_module.reset();
    }

    try
    {
        std::cout << "Attempting to load original file as JIT model: " << checkpoint_file << std::endl;
        jit_module = torch::jit::load(checkpoint_file);
        use_jit_model = true;

        if (torch::cuda::is_available())
        {
            jit_module.value().to(torch::kCUDA);
        }

        std::cout << "Successfully loaded JIT model from " << checkpoint_file << std::endl;

        if (optimizer)
        {
            std::cerr << "Warning: Optimizer state cannot be loaded from JIT model. Optimizer state will be reset." << std::endl;
        }
        return;
    } catch (const std::exception &e)
    {
        std::cout << "Failed to load as JIT model, trying parameter copy method: " << e.what() << std::endl;
        use_jit_model = false;
        jit_module.reset();
    }

    try
    {
        torch::jit::script::Module module = torch::jit::load(checkpoint_file);

        auto loaded_params = module.parameters();
        auto this_params = this->parameters();

        if (loaded_params.size() != this_params.size())
        {
            throw std::runtime_error("Parameter count mismatch between loaded JIT model and current model");
        }

        auto loaded_it = loaded_params.begin();
        auto this_it = this_params.begin();

        for (; loaded_it != loaded_params.end() && this_it != this_params.end(); ++loaded_it, ++this_it)
        {
            this_it->copy_(*loaded_it);
        }

        std::cout << "Successfully loaded model parameters via parameter copying" << std::endl;

        if (optimizer)
        {
            std::cerr << "Warning: Optimizer state cannot be loaded from JIT model. Optimizer state will be reset." << std::endl;
        }
    } catch (const std::exception &e)
    {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        throw;
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

float evaluate(const HAIveContext_t *context, HiveNet &model, const bool isWhiteTurn)
{
    auto y = model.forward(context).item<float>();
    y *= !isWhiteTurn ? -1 : 1;
    if (y > 1) return 1;
    if (y < -1) return -1;
    return y;
}

// float negamax_net(const HAIveContext_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, HiveNet &net)
// {
//     if (depth >= maxDepth)
//     {
//         return evaluate(context, net, true);
//     }
//
//     const GameStatus_t gameStat = getGameStatus(context);
//     if (gameStat == WHITE_WON)
//         return isWhiteTurn ? 1 : -1;
//     if (gameStat == BLACK_WON)
//         return isWhiteTurn ? -1 : 1;
//
//     // Trova i figli
//     Piece_t *moves;
//     getMoves(context, &moves);
//     float maxVal = -2, tmp;
//     Piece_t curBestMove;
//
//     bool moved = false;
//
//     for (uint_fast8_t piece = B_QUEEN; piece < MOVES_ARRAYS; piece++)
//     {
//         for (uint16_t i = 0; moves[MMtA(piece, i)].id != NULLPIECE; i++)
//         {
//             HAIveContext_t newContext;
//             copyHAIveContext(context, &newContext);
//
//             moved = true;
//             addHAIveMove(&newContext, &moves[MMtA(piece, i)]);
//             if ((tmp = negamax_net(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, net)) > maxVal)
//             {
//                 maxVal = tmp;
//                 curBestMove = moves[MMtA(piece, i)];
//             }
//
//             cleanHAIveContext(&newContext);
//         }
//     }
//
//     if (!moved)
//     {
//         HAIveContext_t newContext;
//         copyHAIveContext(context, &newContext);
//
//         addHAIveMove(&newContext, &pass);
//         maxVal = negamax_net(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, net);
//
//         cleanHAIveContext(&newContext);
//     }
//
//     free(moves);
//
//     if (depth == 0)
//     {
//         *bestMove = curBestMove;
//         return 0;
//     }
//     return -maxVal;
// }

// float negamax_heuristic(HAIveContext_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, const std::function<float(HAIveContext_t *)> &heuristicFunc)
// {
//     if (depth >= maxDepth)
//     {
//         return heuristicFunc(context);
//     }
//
//     const GameStatus_t gameStat = getGameStatus(context);
//     if (gameStat == WHITE_WON)
//         return isWhiteTurn ? 1 : -1;
//     if (gameStat == BLACK_WON)
//         return isWhiteTurn ? -1 : 1;
//
//     // Trova i figli
//     Piece_t *moves;
//     getMoves(context, &moves);
//     float maxVal = -2, tmp;
//     Piece_t curBestMove;
//
//     // const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
//     // const uint_fast8_t end = start + 14;
//     bool moved = false;
//
//     HAIveContext_t newContext;
//     for (uint_fast8_t piece = 0; piece < MOVES_ARRAYS; piece++)
//     {
//         for (uint16_t i = 0; moves[MMtA(piece, i)].id != NULLPIECE; i++)
//         {
//             if (i >= 120)
//             {
//                 logE(stderr, "Too many moves!\n");
//             }
//             copyHAIveContext(context, &newContext);
//
//             moved = true;
//             addHAIveMove(&newContext, &moves[MMtA(piece, i)]);
//             if ((tmp = negamax_heuristic(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc)) > maxVal)
//             {
//                 maxVal = tmp;
//                 curBestMove = moves[MMtA(piece, i)];
//             }
//             cleanHAIveContext(&newContext);
//         }
//     }
//
//     if (!moved)
//     {
//         copyHAIveContext(context, &newContext);
//
//         addHAIveMove(&newContext, &pass);
//         maxVal = negamax_heuristic(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc);
//         cleanHAIveContext(&newContext);
//     }
//
//     free(moves);
//
//     if (depth == 0)
//     {
//         *bestMove = curBestMove;
//         return 0;
//     }
//     return -maxVal;
// }

// float negamax_heuristic_ab(
//     HAIveContext_t *context, const int depth, const int maxDepth,
//     const bool isWhiteTurn,
//     Piece_t *bestMove, const std::function<float(HAIveContext_t *)> &heuristicFunc,
//     float alpha, float beta,
//     const std::chrono::high_resolution_clock::time_point &startTime, int maxDurationMs,
//     Hashmap_t *hashtable
// )
// {
//     auto now = std::chrono::high_resolution_clock::now();
//     int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
//     if (elapsed >= maxDurationMs)
//     {
//         return -9999.0f;
//     }
//
//     if (depth >= maxDepth)
//     {
//         const uint64_t hash = hashAll(context->board, context->idToPos, context->curColor);
//         if (const auto result = static_cast<float *>(getByHash(hash, hashtable)); result != nullptr)
//         {
//             return *result;
//         }
//         const float res = heuristicFunc(context);
//         setByHash(hash, &res, sizeof(float), hashtable);
//         return res;
//     }
//
//     const GameStatus_t gameStat = getGameStatus(context);
//     if (gameStat == WHITE_WON)
//         return isWhiteTurn ? 1 : -1;
//     if (gameStat == BLACK_WON)
//         return isWhiteTurn ? -1 : 1;
//
//     // Trova i figli
//     Piece_t *moves;
//     getMoves(context, &moves);
//     float maxVal = -2, tmp;
//     Piece_t curBestMove;
//
//     // const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
//     // const uint_fast8_t end = start + 14;
//     bool moved = false;
//
//     HAIveContext_t newContext;
//     for (uint_fast8_t piece = 0; piece < MOVES_ARRAYS; piece++)
//     {
//         for (uint16_t i = 0; moves[MMtA(piece, i)].id != NULLPIECE; i++)
//         {
//             if (i >= 120)
//             {
//                 logE(stderr, "Too many moves!\n");
//             }
//             copyHAIveContext(context, &newContext);
//
//             auto check = std::chrono::high_resolution_clock::now();
//             int elapsedLoop = std::chrono::duration_cast<std::chrono::milliseconds>(check - startTime).count();
//             if (elapsedLoop >= maxDurationMs)
//             {
//                 free(moves);
//                 return -9999.0f; // Early abort
//             }
//
//             moved = true;
//             addHAIveMove(&newContext, &moves[MMtA(piece, i)]);
//             tmp = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha, startTime, maxDurationMs, hashtable);
//             cleanHAIveContext(&newContext);
//
//             if (tmp == -9999.0f)
//             {
//                 free(moves);
//                 return -9999.0f; // propagate timeout
//             }
//
//             if (tmp > maxVal)
//             {
//                 maxVal = tmp;
//                 curBestMove = moves[MMtA(piece, i)];
//             }
//             if (maxVal > alpha)
//                 alpha = maxVal;
//             if (alpha >= beta)
//                 break;
//         }
//     }
//
//     if (!moved)
//     {
//         copyHAIveContext(context, &newContext);
//
//         addHAIveMove(&newContext, &pass);
//         maxVal = -negamax_heuristic_ab(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, -beta, -alpha, startTime, maxDurationMs, hashtable);
//
//         cleanHAIveContext(&newContext);
//     }
//
//     free(moves);
//
//     if (depth == 0)
//     {
//         *bestMove = curBestMove;
//         return 0;
//     }
//     return maxVal;
// }


Result resultOf(const HAIveContext_t *context)
{
    const GameStatus_t gameStat = context->gameStatus;
    if (gameStat == WHITE_WON)
        return Result::RESULT_WHITE_WON;
    if (gameStat == BLACK_WON)
        return Result::RESULT_BLACK_WON;
    return Result::RESULT_DRAW;
}

// bool isPass(Piece_t *moves)
// {
//     for (uint8_t i = 0; i < MOVES_ARRAYS; i++)
//     {
//         if (moves[MMtA(i, 0)].id != NULLPIECE)
//             return false;
//     }
//     return true;
// }

// bool battleAgainstRandom(bool areWeWhite)
// {
//     HAIveContext_t context;
//     Piece_t bestMove;
//     Piece_t *moves;
//     initHAIveContext(&context);
//
//     Hashmap_t hashtable;
//     initHashmap(&hashtable, 8192);
//
//     while (!isContextEnded(&context))
//     {
//         if (context.turn % 10 == 0)
//             printf("Turn %d\n", context.turn);
//         if ((areWeWhite && context.curColor == WHITE) || (!areWeWhite && context.curColor == BLACK))
//         {
//             negamax_heuristic_ab(&context, 0, 2, context.curColor == WHITE, &bestMove, mzingaHeuristic, -2, 2, std::chrono::high_resolution_clock::now(), 1000000, &hashtable);
//             addHAIveMove(&context, &bestMove);
//         } else
//         {
//             getMoves(&context, &moves);
//             uint_fast8_t chosenPiece;
//             uint16_t chosenMove;
//             if (context.idToPos[context.curColor == WHITE ? W_QUEEN : B_QUEEN].z == -1)
//             {
//                 chosenPiece = 14;
//             } else
//             {
//                 if (isPass(moves))
//                 {
//                     addHAIveMove(&context, &pass);
//                     free(moves);
//                     continue;
//                 }
//                 do
//                 {
//                     chosenPiece = rand() % MOVES_ARRAYS;
//                 } while (moves[MMtA(chosenPiece, 0)].id == NULLPIECE);
//             }
//             chosenMove = rand() % getMovesSize(&moves[MMtA(chosenPiece, 0)]);
//             addHAIveMove(&context, &moves[MMtA(chosenPiece, chosenMove)]);
//             free(moves);
//         }
//     }
//
//     const GameStatus_t gameStat = getGameStatus(&context);
//     cleanHAIveContext(&context);
//
//     if (areWeWhite && gameStat == WHITE_WON)
//         return true;
//     if (!areWeWhite && gameStat == BLACK_WON)
//         return true;
//     return false;
// }

// void testAgainstRandom()
// {
//     int played = 0;
//     int won = 0;
//     srand(time(NULL));
//
//     for (size_t i = 0; i < 100; i++)
//     {
//         if (battleAgainstRandom(i % 2 == 0))
//             won++;
//         played++;
//
//         printf("%d / %d\n", won, played);
//     }
// }


