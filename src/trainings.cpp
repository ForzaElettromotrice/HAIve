//
// Created by filip on 20/05/2025.
//

#include "trainings.h"

#include <cfloat>

void getRandomState(Context_t* toFill, const uint8_t minMoves, const uint8_t maxMoves) {

    const uint8_t movesToPlay = (rand() % (maxMoves - minMoves)) + minMoves;
    Piece_t** moves;
    for (uint8_t i = 0; i < movesToPlay && !isContextEnded(toFill); i++) {
        getMoves(toFill, &moves);

        // DEBUG
        uint16_t mSize = 0;
        for (uint_fast8_t j = 0; j < MOVES_ARRAYS; j++) {
            mSize += getMovesSize(moves[j]);
        }
        if (mSize == 0) {
            printf("ERROR!!!");
            return;
        }

        uint8_t chosenPiece;
        do {
            chosenPiece = rand() % MOVES_ARRAYS;
        } while (moves[chosenPiece][0].id == NULLPIECE);
        uint16_t chosenMove = rand() % getMovesSize(moves[chosenPiece]);
        addOurMove(toFill, moves[chosenPiece][chosenMove]);
    }

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

    size_t trainCycles = 50, testCycles = 25, i, epochs = 0, maxEpochs = 10;

    srand(time(nullptr));
    std::shared_ptr<torch::optim::Optimizer> optimizer =
        std::make_shared<torch::optim::Adam>(model->parameters(), torch::optim::AdamOptions(1e-3));


    Context_t context;
    initContext(&context);
    torch::Tensor x;

    auto avg_error = DBL_MAX;
    while (avg_error > 0.01 && epochs < maxEpochs) {  // We want a hundredth precision

        for (i = 0; i < trainCycles; i++) {
            resetContext(&context);
            context.gameStatus = NOT_STARTED;

            getRandomState(&context, 1, 100);
            Processor::boardToTensor(context.idToPos, x);
            x = x.set_requires_grad(true);

            const torch::Tensor y_gt = torch::tensor(heuristic(&context));
            const torch::Tensor y_pred = model->forward(x);
            torch::Tensor loss = torch::abs(y_pred - y_gt);

            optimizer->zero_grad();
            loss.backward();
            optimizer->step();
        }

        double totError = 0;
        for (i = 0; i < testCycles; i++) {
            resetContext(&context);
            getRandomState(&context, 1, 60);
            Processor::boardToTensor(context.idToPos, x);

            const double y_gt = heuristic(&context);
            const double y_pred = model->forward(x).item().toFloat();

            totError += abs(y_pred - y_gt);
        }
        avg_error = totError / testCycles;

        printf("Avg. Error: %lf\n", avg_error);
        epochs++;

    }

    model->save_model(optimizer);

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
        initContext(&context); resetContext(&context);
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
            addOurMove(&context, bestMove);
            if (context.gameStatus != DRAW && context.gameStatus != WHITE_WON && context.gameStatus != BLACK_WON)
                processor.playMove(&context);

        }

        processor.endGame(&context, resultOf(&context));

    }

    // Training

}

double MzingaHeuristicTrainer::heuristic(Context_t* context) {

    return mzingaHeuristic(context);

}
