//
// Created by filip on 20/05/2025.
//

#include "trainings.h"

bool isContextEnded(Context_t* context) {
    const GameStatus_t gameStatus = context->gameStatus;
    return gameStatus == WHITE_WON || gameStatus == BLACK_WON || gameStatus == DRAW;
}

void getRandomState(Context_t* toFill, const uint8_t minMoves, const uint8_t maxMoves) {

    srand(time(NULL));
    const uint8_t movesToPlay = (rand() % (maxMoves - minMoves)) + minMoves;
    Piece_t* moves; uint_fast8_t mSize;
    for (uint8_t i = 0; i < movesToPlay && !isContextEnded(toFill); i++) {
        getMoves(toFill, &moves, &mSize);
        mSize = rand() % mSize;
        manageMove(toFill, &moves[mSize]);
    }
    printf("Random game ended!\n");

}

void setDataFileName(std::string& filename, const size_t i) {
    std::ostringstream oss;
    oss << "play_" << i << ".dat";
    filename = oss.str();
}

void LearnFromHeuristicTrainer::train(const bool toLoad) {

    auto model = HiveCNN(getFilename());
    if (toLoad && std::filesystem::exists(getFilename())){
        model->load_model();
    }

    float avg_error = FLT_MAX;
    while (avg_error > 0.01) {  // We want a hundredth precision
        Context_t context;
        torch::Tensor x;

        printf("--- Train Phase ---\n");
        for (uint_fast8_t i = 0; i < 20; i++) {
            initContext(&context);
            context.gameStatus = NOT_STARTED;

            getRandomState(&context, 1, 20);
            Processor::getTensor(&context, x);

            const torch::Tensor y_gt = torch::tensor(heuristic(&context));
            const torch::Tensor y_pred = model->forward(x);
            torch::Tensor loss = torch::abs(y_pred - y_gt);
            printf("Epoch %d/20 - Loss: %lf\n", i + 1, loss.item<float>());

            loss.backward();

            cleanContext(&context);
        }

        float totError = 0;
        printf("------------------\n");
        printf("--- Test Phase ---\n");
        for (uint_fast8_t i = 0; i < 10; i++) {
            initContext(&context);
            getRandomState(&context, 1, 60);
            Processor::getTensor(&context, x);

            const float y_gt = heuristic(&context);
            const float y_pred = model->forward(x).item().toFloat();

            printf("Epoch %d/10 - Error: %lf\n", i + 1, abs(y_pred - y_gt));

            totError += abs(y_pred - y_gt);
            cleanContext(&context);
        }
        printf("------------------\n");
        avg_error = totError / 10;
    }

}


void SelfPlayTrainer::train(const bool toLoad) {

    auto ourModel = HiveCNN(getFilename());
    auto enemyModel = HiveCNN(getFilename());
    if (toLoad && std::filesystem::exists(getFilename())){
        ourModel->load_model();
        enemyModel->load_model();
    }
    Context_t context;

    for (uint16_t i = 0; i < 100; i++) {
        std::string dataFile;
        printf("--- Starting game %d /100 ---", i + 1);
        initContext(&context); contextNewGameMLP(&context);
        setDataFileName(dataFile, i);
        auto processor = Processor(dataFile);
        processor.newGame();
        const bool areWeWhite = i % 2 == 0;

        while(context.gameStatus != DRAW && context.gameStatus != WHITE_WON && context.gameStatus != BLACK_WON) {

            Piece_t bestMove;
            if (context.curColor == WHITE)
                negamax_net(&context, 0, 1, true, &bestMove, areWeWhite ? ourModel : enemyModel);
            else
                negamax_net(&context, 0, 1, false, &bestMove, areWeWhite ? enemyModel : ourModel);
            manageMove(&context, &bestMove);
            if (context.gameStatus != DRAW && context.gameStatus != WHITE_WON && context.gameStatus != BLACK_WON)
                processor.playMove(&context);

        }

        processor.endGame(&context, resultOf(&context));

    }

    // Training

}

double MzingaHeuristicTrainer::heuristic(Context_t* context) {

    double result = 0;
    bool whiteTurn = context->curColor == WHITE;
    uint_fast8_t mSize = 0;
    Piece_t* moves;
    getMoves(context, &moves, &mSize);

    for (uint8_t i = B_QUEEN; i < NUM_PIECES; i++) {
        const Position_t piecePos = context->idToPos[i];
        if (piecePos.z == -1)
            continue;
        HeuristicMetrics pieceMetric = getMetrics(static_cast<Pieces_t>(i));
        if (!whiteTurn)
            pieceMetric = pieceMetric.black();

        result += pieceMetric.inPlayWeight();

        if (piecePos.z < 5 && context->board[MtA(piecePos.z + 1, piecePos.y, piecePos.x)] != NULLPIECE)
            result += pieceMetric.isCoveredWeight();

        for (uint8_t j = 0; j < 6; j++) {
            const int_fast8_t newY = directions[j][0];
            const int_fast8_t newX = directions[j][1];

            const Pieces_t neighbor = context->board[MtA(piecePos.z, newY, newX)];
            if (neighbor != NULLPIECE) {
                if (isBlack(neighbor) && isBlack(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else if (isWhite(neighbor) && isWhite(i))
                    result += pieceMetric.friendlyNeighborWeight();
                else
                    result += pieceMetric.enemyNeighborWeight();
            }
        }

        // How to check if isPinned?

        const Pieces_t enemyQueen = isBlack(i) ? W_QUEEN : B_QUEEN;
        if (context->idToPos[enemyQueen].z == -1)
            result += (pieceMetric.quietMoveWeight() * mSize);

        for (size_t j = 0; j < mSize; j++) {
            if (moves[mSize].id == i) {

                const Position_t newPos = moves[mSize].position;
                const Position_t enemyQueenPos = context->idToPos[enemyQueen];
                if (abs(newPos.y - enemyQueenPos.y) + abs(newPos.x - enemyQueenPos.x) > 2) {
                    result += pieceMetric.quietMoveWeight();
                    continue;
                }
                if (abs(piecePos.y - enemyQueenPos.y) + abs(piecePos.x - enemyQueenPos.x) > 2)
                    result += pieceMetric.noisyMoveWeight();
                else
                    result += pieceMetric.quietMoveWeight();

            }
        }

    }

    return result;

}
