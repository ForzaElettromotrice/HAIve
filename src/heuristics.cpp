//
// Created by filippo on 15/06/25.
//

#include "heuristics.hpp"

bool MetricsManager::loadFromFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.find("inline HeuristicMetrics") == std::string::npos)
            continue;

        size_t start = line.find("HeuristicMetrics") + std::string("HeuristicMetrics").length();
        size_t end = line.find('(');
        std::string name = line.substr(start, end - start);
        name.erase(remove_if(name.begin(), name.end(), ::isspace), name.end());

        std::vector<double> values;

        while (std::getline(file, line))
        {
            if (line.find("});") != std::string::npos) break;

            auto comment_pos = line.find("//");
            if (comment_pos != std::string::npos)
                line = line.substr(0, comment_pos);

            line.erase(std::remove(line.begin(), line.end(), ','), line.end());
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

            if (!line.empty())
            {
                try
                {
                    values.push_back(std::stod(line));
                } catch (...)
                {
                    std::cerr << "Warning: could not parse line: " << line << std::endl;
                }
            }
        }

        if (name == "queenMetrics") queenMetrics = HeuristicMetrics(values);
        else if (name == "spiderMetrics") spiderMetrics = HeuristicMetrics(values);
        else if (name == "beetleMetrics") beetleMetrics = HeuristicMetrics(values);
        else if (name == "grasshopperMetrics") grasshopperMetrics = HeuristicMetrics(values);
        else if (name == "antMetrics") antMetrics = HeuristicMetrics(values);
        else if (name == "pillbugMetrics") pillbugMetrics = HeuristicMetrics(values);
        else if (name == "mosquitoMetrics") mosquitoMetrics = HeuristicMetrics(values);
        else if (name == "ladybugMetrics") ladybugMetrics = HeuristicMetrics(values);
        else
        {
            std::cerr << "Warning: Unknown metric name \"" << name << "\"\n";
        }
    }

    return true;
}

bool MetricsManager::saveToFile(const std::string &filename) const
{
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    auto writeMetric = [&out](const std::string &name, const HeuristicMetrics &metric)
    {
        out << "inline HeuristicMetrics " << name << "({\n";
        for (size_t i = 0; i < metric.weights().size(); ++i)
        {
            out << "    " << metric.weights()[i];
            if (i != metric.weights().size() - 1)
                out << ",";
            out << "\n";
        }
        out << "});\n\n";
    };

    writeMetric("queenMetrics", queenMetrics);
    writeMetric("spiderMetrics", spiderMetrics);
    writeMetric("beetleMetrics", beetleMetrics);
    writeMetric("grasshopperMetrics", grasshopperMetrics);
    writeMetric("antMetrics", antMetrics);
    writeMetric("pillbugMetrics", pillbugMetrics);
    writeMetric("mosquitoMetrics", mosquitoMetrics);
    writeMetric("ladybugMetrics", ladybugMetrics);

    return true;
}

uint_fast8_t getMovesSize(const Piece_t *moves)
{
    uint8_t size = 0;
    while (moves[size].id != NULLPIECE)
        size++;
    return size;
}

double mzingaHeuristic(HAIveContext_t *context)
{
    if (context->gameStatus == WHITE_WON)
        return static_cast<double>(Result::RESULT_WHITE_WON);
    if (context->gameStatus == BLACK_WON)
        return static_cast<double>(Result::RESULT_BLACK_WON);

    double result = 0;
    const bool whiteTurn = context->curColor == WHITE;
    Piece_t moves[MOVES_SIZE];
    if (context->curColor == BLACK)
        getMoves(context, moves);
    else
    {
        context->curColor = static_cast<Colors_t>(context->curColor * -1);
        getMoves(context, moves);
    }

    MetricsManager metricManager;

    //FIXME: non funziona più così devi iterare sulle mosse e via
    for (uint8_t i = B_QUEEN; i < NUM_PIECES; i++)
    {
        //FIXME: ora le devi ricalcolare semplicemente quando termini il for
        if (i == W_QUEEN)
        {
            context->curColor = static_cast<Colors_t>(context->curColor * -1);
            getMoves(context, moves);
        }
        const auto [z, y, x] = context->idToPos[i];
        if (z == -1)
            continue;
        // const uint16_t mSize = getMovesSize(&moves[MMtA(i % 14, 0)]);
        HeuristicMetrics pieceMetric = metricManager.getMetrics(static_cast<Pieces_t>(i));
        if ((whiteTurn && isBlack(i)) || (!whiteTurn && isWhite(i)))
            pieceMetric = pieceMetric.enemy();

        result += pieceMetric.inPlayWeight();

        if (z < 5 && context->board[MtA(z + 1, y, x)] != NULLPIECE)
            result += pieceMetric.isCoveredWeight();

        for (const auto direction: directions)
        {
            const int_fast8_t newY = direction[0] + y;
            const int_fast8_t newX = direction[1] + x;

            const Pieces_t neighbor = context->board[MtA(z, newY, newX)];
            if (neighbor == NULLPIECE)
                continue;

            if (isBlack(neighbor) && isBlack(i))
                result += pieceMetric.friendlyNeighborWeight();
            else if (isWhite(neighbor) && isWhite(i))
                result += pieceMetric.friendlyNeighborWeight();
            else
                result += pieceMetric.enemyNeighborWeight();
        }

        if (howManyAround(context, static_cast<Pieces_t>(i), true) + howManyAround(context, static_cast<Pieces_t>(i), false) > 2)
            result += pieceMetric.isPinnedWeight();

        const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
        if (context->idToPos[enemyQueen].z == -1)
        {
            //FIXME: perché ti serve la size delle move? ora non ce l'hai piu, volendo sono semplicemente calcolabili (tranne per il PILLBUG)
            // result += pieceMetric.quietMoveWeight() * mSize;
            continue;
        }

        //FIXME: come ho scritto all'inizio, ora si iterna normalmente
        // for (size_t j = 0; moves[MMtA(i % 14, j)].id != NULLPIECE; j++)
        // {
        //     const Position_t newPos = moves[MMtA(i % 14, j)].position;
        //     const Position_t enemyQueenPos = context->idToPos[enemyQueen];
        //     if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2)
        //     {
        //         result += pieceMetric.quietMoveWeight();
        //         continue;
        //     }
        //     if (abs(y - enemyQueenPos.y) + abs(x - enemyQueenPos.x) > 2)
        //         result += pieceMetric.noisyMoveWeight();
        //     else
        //         result += pieceMetric.quietMoveWeight();
        // }
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

/*
    Fills x with some useful statistics about the context.
*/
//FIXME: sta funzione non viene mai chiamata
// void setHeuristicParams(const HAIveContext_t *context, torch::Tensor &x)
// {
//     // n_neigh_friendly + enemy  // moves (quiet/noisy)
//     const int64_t size = 2 * NUM_PIECES + 2 * NUM_PIECES;
//     TORCH_CHECK(x.dim() == 1 && x.size(0) >= size, "Tensor x has incorrect shape");
//
//     // For other tests: maybe adding positional+identitary embeddings of pieces.
//
//     Piece_t *moves;
//     getMoves(context, &moves);
//
//     for (uint_fast8_t i = B_QUEEN; i < NUM_PIECES; i++)
//     {
//         x[i] = howManyAround(context, static_cast<Pieces_t>(i), true);
//         x[i + NUM_PIECES] = howManyAround(context, static_cast<Pieces_t>(i), false);
//
//         const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
//         uint16_t noisyMoves = 0, quietMoves = 0;
//         if (context->idToPos[enemyQueen].z == -1)
//         {
//             for (uint_fast16_t j = 0; moves[MMtA(i % 14, j)].id != NULLPIECE; j++)
//             {
//                 quietMoves++;
//             }
//         } else
//         {
//             for (size_t j = 0; moves[MMtA(i % 14, j)].id != NULLPIECE; j++)
//             {
//                 const Position_t piecePos = context->idToPos[i];
//                 const Position_t newPos = moves[MMtA(i % 14, j)].position;
//                 const Position_t enemyQueenPos = context->idToPos[enemyQueen];
//
//                 if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2)
//                 {
//                     quietMoves++;
//                     continue;
//                 }
//                 if (abs(piecePos.y - enemyQueenPos.y) + abs(piecePos.x - enemyQueenPos.x) > 2)
//                     noisyMoves++;
//                 else
//                     quietMoves++;
//             }
//         }
//         x[i + 2 * NUM_PIECES] = quietMoves;
//         x[i + 3 * NUM_PIECES] = noisyMoves;
//     }
// }
