#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <limits>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <functional>
extern "C" {
#include "enums.h"
#include "moves.h"
#include "utils.h"
}
#include "heuristics_minimal.hpp"
#include "preprocessor_minimal.hpp"
extern "C" {
#include "CUtils/logger.h"
}

// Forward declarations for missing functions
int games = 100; // Default number of games
double evaluate(const MetricsManager& manager);
double compare(const MetricsManager& manager1, const MetricsManager& manager2); // Fixed signature

// Helper function to create a modified MetricsManager
MetricsManager createModifiedMetricsManager(const MetricsManager& original, 
                                           int pieceType, int metricIndex, double newValue) {
    // Save original to temporary file
    std::string tempFile = "temp_metrics_" + std::to_string(std::time(nullptr)) + ".txt";
    
    // Since the saveToFile method in heuristics.cpp has a bug (references metric.weights instead of metric.values_),
    // we'll implement our own save functionality here
    std::ofstream out(tempFile);
    if (!out.is_open()) {
        std::cerr << "Error: Could not create temporary file for metrics modification\n";
        return original;
    }

    std::vector<std::string> pieceNames = {
        "queenMetrics", "spiderMetrics", "beetleMetrics", "grasshopperMetrics",
        "antMetrics", "pillbugMetrics", "mosquitoMetrics", "ladybugMetrics"
    };
    
    std::vector<Pieces_t> pieces = {
        W_QUEEN, W_SPIDER_1, W_BEETLE_1, W_GRASSHOPPER_1,
        W_ANT_1, W_PILLBUG, W_MOSQUITO, W_LADYBUG
    };

    // Write all metrics, modifying the specified one
    for (size_t i = 0; i < pieceNames.size(); ++i) {
        HeuristicMetrics metrics = original.getMetrics(pieces[i]);
        
        out << "inline HeuristicMetrics " << pieceNames[i] << "({\n";
        
        // Get all 7 values
        std::array<double, 7> values = {
            metrics.inPlayWeight(),
            metrics.isPinnedWeight(),
            metrics.isCoveredWeight(),
            metrics.noisyMoveWeight(),
            metrics.quietMoveWeight(),
            metrics.friendlyNeighborWeight(),
            metrics.enemyNeighborWeight()
        };
        
        // Modify the specific value if this is the target piece type
        if (static_cast<int>(i) == pieceType) {
            values[metricIndex] = newValue;
        }
        
        // Write values
        for (size_t j = 0; j < values.size(); ++j) {
            out << "    " << values[j];
            if (j != values.size() - 1) {
                out << ",";
            }
            out << "\n";
        }
        out << "});\n\n";
    }
    out.close();
    
    // Create new MetricsManager from modified file
    MetricsManager modified;
    if (!modified.loadFromFile(tempFile)) {
        std::cerr << "Error: Could not load modified metrics from temporary file\n";
        std::remove(tempFile.c_str());
        return original;
    }
    
    // Clean up temporary file
    std::remove(tempFile.c_str());
    
    return modified;
}

MetricsManager perturb(const MetricsManager& current, std::mt19937& rng, double temperature = 1.0) {
    MetricsManager neighbor = current;
    
    // There are 8 piece types, each with 7 metric values = 56 total parameters
    std::uniform_int_distribution<> pieceTypeDist(0, 7); // 8 piece types
    std::uniform_int_distribution<> metricIndexDist(0, 6); // 7 metrics per type
    std::normal_distribution<> noise(0.0, temperature);

    // Perturb 1 or 2 random metrics
    int numPerturb = 1 + (rng() % 2);
    for (int i = 0; i < numPerturb; ++i) {
        int pieceType = pieceTypeDist(rng);
        int metricIndex = metricIndexDist(rng);
        
        // Get the appropriate piece to read current value
        Pieces_t piece;
        switch (pieceType) {
            case 0: piece = W_QUEEN; break;
            case 1: piece = W_SPIDER_1; break;
            case 2: piece = W_BEETLE_1; break;
            case 3: piece = W_GRASSHOPPER_1; break;
            case 4: piece = W_ANT_1; break;
            case 5: piece = W_PILLBUG; break;
            case 6: piece = W_MOSQUITO; break;
            case 7: piece = W_LADYBUG; break;
            default: piece = W_QUEEN; break;
        }
        
        // Get current metrics for this piece type
        HeuristicMetrics currentMetrics = neighbor.getMetrics(piece);
        
        // Get current value and add noise
        double currentValue;
        switch (metricIndex) {
            case 0: currentValue = currentMetrics.inPlayWeight(); break;
            case 1: currentValue = currentMetrics.isPinnedWeight(); break;
            case 2: currentValue = currentMetrics.isCoveredWeight(); break;
            case 3: currentValue = currentMetrics.noisyMoveWeight(); break;
            case 4: currentValue = currentMetrics.quietMoveWeight(); break;
            case 5: currentValue = currentMetrics.friendlyNeighborWeight(); break;
            case 6: currentValue = currentMetrics.enemyNeighborWeight(); break;
            default: currentValue = 0.0; break;
        }
        
        double newValue = currentValue + noise(rng);
        newValue = std::clamp(newValue, -1.0, 1.0); // Adjust bounds as needed
        
        // Create modified MetricsManager
        neighbor = createModifiedMetricsManager(neighbor, pieceType, metricIndex, newValue);
    }

    return neighbor;
}

// Remove the broken compare function that expects std::vector<double>
// The correct compare function is now in simulated_annealing_implementations.cpp

MetricsManager simulated_annealing(
    const MetricsManager& initial_weights,
    int max_iterations = 1000,
    double initial_temp = 1.0,
    double cooling_rate = 0.99
) {
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    MetricsManager current = initial_weights;
    double current_score = evaluate(current);

    MetricsManager best = current;
    double best_score = current_score;

    double T = initial_temp;

    for (int iter = 0; iter < max_iterations; ++iter) {
        MetricsManager neighbor = perturb(current, rng, T); // Pass temperature
        double neighbor_score = evaluate(neighbor);

        // FIXED: Use score difference instead of compare function
        double delta = neighbor_score - current_score;
        
        // Acceptance criteria: accept if better, or with probability exp(delta/T) if worse
        bool accept = (delta > 0) || (uniform(rng) < std::exp(delta / T));
        
        if (accept) {
            current = neighbor;
            current_score = neighbor_score;
            
            // Update best if this is better
            if (neighbor_score > best_score) {
                best = neighbor;
                best_score = neighbor_score;
            }
        }

        T *= cooling_rate;

        // Optional logging
        if (iter % 100 == 0 || iter == max_iterations - 1) {
            std::cout << "Iter " << iter << " | Best Score: " << best_score 
                      << " | Current Score: " << current_score << " | T: " << T << "\n";
        }
    }

    return best;
}

float negamax_heuristic_dup(HAIveContext_t *context, const int depth, const int maxDepth, const bool isWhiteTurn, Piece_t *bestMove, const std::function<float(HAIveContext_t *, MetricsManager&)> &heuristicFunc, MetricsManager& metricManagerWhite, MetricsManager& metricManagerBlack)
{
    std::cout << "[DEBUG] negamax_heuristic_dup called: depth=" << depth << ", maxDepth=" << maxDepth << ", isWhiteTurn=" << isWhiteTurn << std::endl;
    
    if (depth >= maxDepth)
    {
        std::cout << "[DEBUG] Reached max depth, calling heuristic function..." << std::endl;
        MetricsManager& metricManager = isWhiteTurn ? metricManagerWhite : metricManagerBlack;
        float result = heuristicFunc(context, metricManager);
        std::cout << "[DEBUG] Heuristic function returned: " << result << std::endl;
        return result;
    }

    const GameStatus_t gameStat = getGameStatus(context);
    std::cout << "[DEBUG] Game status: " << gameStat << std::endl;
    if (gameStat == WHITE_WON)
        return isWhiteTurn ? 1 : -1;
    if (gameStat == BLACK_WON)
        return isWhiteTurn ? -1 : 1;

    // Trova i figli
    std::cout << "[DEBUG] About to call getMoves..." << std::endl;
    Piece_t *moves;
    getMoves(context, &moves);
    std::cout << "[DEBUG] getMoves completed successfully" << std::endl;
    float maxVal = -2, tmp;
    Piece_t curBestMove = {NULLPIECE, {-1, -1, -1}}; // Initialize to null move

    const uint_fast8_t start = context->curColor == WHITE ? W_QUEEN : B_QUEEN;
    const uint_fast8_t end = start + 14;
    bool moved = false;

    HAIveContext_t newContext;
    for (uint_fast8_t piece = 0; piece < MOVES_ARRAYS; piece++)
    {
        for (uint16_t i = 0; moves[MMtA(piece, i)].id != NULLPIECE; i++)
        {
            if (i >= 120)
            {
                logE(stderr, "Too many moves!\n");
            }
            copyHAIveContext(context, &newContext);

            moved = true;
            addHAIveMove(&newContext, &moves[MMtA(piece, i)]);
            if ((tmp = negamax_heuristic_dup(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, metricManagerWhite, metricManagerBlack)) > maxVal)
            {
                maxVal = tmp;
                curBestMove = moves[MMtA(piece, i)];
            }
            cleanHAIveContext(&newContext);
        }
    }

    if (!moved)
    {
        copyHAIveContext(context, &newContext);

        addHAIveMove(&newContext, &pass);
        maxVal = negamax_heuristic_dup(&newContext, depth + 1, maxDepth, !isWhiteTurn, bestMove, heuristicFunc, metricManagerWhite, metricManagerBlack);
        cleanHAIveContext(&newContext);
    }

    free(moves);

    if (depth == 0)
    {
        *bestMove = curBestMove;
        return 0;
    }
    return -maxVal;
}

double mzingaHeuristicVariable(HAIveContext_t *context, MetricsManager& metricManager)
{
    if (context->gameStatus == WHITE_WON)
        return static_cast<double>(Result::RESULT_WHITE_WON);
    if (context->gameStatus == BLACK_WON)
        return static_cast<double>(Result::RESULT_BLACK_WON);

    double result = 0;
    bool whiteTurn = context->curColor == WHITE;
    Piece_t *moves;
    if (context->curColor == BLACK)
        getMoves(context, &moves);
    else
    {
        context->curColor = static_cast<Colors_t>(context->curColor * -1);
        getMoves(context, &moves);
    }

    for (uint8_t i = B_QUEEN; i < NUM_PIECES; i++)
    {
        if (i == W_QUEEN)
        {
            context->curColor = static_cast<Colors_t>(context->curColor * -1);
            free(moves);
            getMoves(context, &moves);
        }
        const Position_t piecePos = context->idToPos[i];
        if (piecePos.z == -1)
            continue;
        const uint16_t mSize = getMovesSize(&moves[MMtA(i % 14, 0)]);
        HeuristicMetrics pieceMetric = metricManager.getMetrics(static_cast<Pieces_t>(i));
        if ((whiteTurn && isBlack(i)) || (!whiteTurn && isWhite(i)))
            pieceMetric = pieceMetric.enemy();

        result += pieceMetric.inPlayWeight();

        if (piecePos.z < 5 && context->board[MtA(piecePos.z + 1, piecePos.y, piecePos.x)] != NULLPIECE)
            result += pieceMetric.isCoveredWeight();

        for (uint8_t j = 0; j < 6; j++)
        {
            const int_fast8_t newY = directions[j][0] + piecePos.y;
            const int_fast8_t newX = directions[j][1] + piecePos.x;

            const Pieces_t neighbor = context->board[MtA(piecePos.z, newY, newX)];
            if (neighbor != NULLPIECE)
            {
                if (isBlack(neighbor) && isBlack(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else if (isWhite(neighbor) && isWhite(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else
                    result += pieceMetric.enemyNeighborWeight();
            }
        }

        if (howManyAround(context, static_cast<Pieces_t>(i), true) + howManyAround(context, static_cast<Pieces_t>(i), false) > 2)
            result += pieceMetric.isPinnedWeight();

        const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
        if (context->idToPos[enemyQueen].z == -1)
        {
            result += (pieceMetric.quietMoveWeight() * mSize);
            continue;
        }

        for (size_t j = 0; moves[MMtA(i % 14, j)].id != NULLPIECE; j++)
        {
            const Position_t newPos = moves[MMtA(i % 14, j)].position;
            const Position_t enemyQueenPos = context->idToPos[enemyQueen];
            if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2)
            {
                result += pieceMetric.quietMoveWeight();
                continue;
            }
            if (abs(piecePos.y - enemyQueenPos.y) + abs(piecePos.x - enemyQueenPos.x) > 2)
                result += pieceMetric.noisyMoveWeight();
            else
                result += pieceMetric.quietMoveWeight();
        }
    }

    context->curColor = whiteTurn ? WHITE : BLACK;
    free(moves);

    const Pieces_t enemyQueen = whiteTurn ? B_QUEEN : W_QUEEN;

    result /= 5200000;
    // myVal
    if (context->idToPos[enemyQueen].z != -1)
    {
        // result += (0.17 * (howManyAround(context, enemyQueen, false) + howManyAround(context, enemyQueen, true)));
        int howMany = howManyAround(context, enemyQueen, false) + howManyAround(context, enemyQueen, true);
        result += (howMany * howMany * 0.032);
        result -= (howMany * 0.0286);
        result += 0.0179;
    }
    return result;
}