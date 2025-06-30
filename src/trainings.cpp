//
// Created by filip on 20/05/2025.
//

#include "trainings.h"
#include <random>
#include <cfloat>
#include "minmanager.h"

namespace fs = std::filesystem;

void getRandomState(Context_t *toFill, const uint8_t minMoves, const uint8_t maxMoves) {
    const uint8_t movesToPlay = (rand() % (maxMoves - minMoves)) + minMoves;
    Piece_t *moves[15];
    for (uint8_t i = 0; i < movesToPlay && !isContextEnded(toFill); i++) {
        getMoves(toFill, moves);

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
        addOurMove(toFill, &moves[chosenPiece][chosenMove]);
    }
}

void setDataFileName(std::string &filename, const size_t i) {
    std::ostringstream oss;
    oss << "play_" << i << ".dat";
    filename = oss.str();
}

void LearnFromHeuristicTrainer::train(const bool toLoad) {
    auto model = HiveCNN(getFilename());
    if (toLoad && std::filesystem::exists(getFilename())) {
        model->load_model();
    }

    size_t trainCycles = 50, testCycles = 25, i, epochs = 0, maxEpochs = 10;

    srand(time(nullptr));
    std::shared_ptr<torch::optim::Optimizer> optimizer =
            std::make_shared<torch::optim::Adam>(model->parameters(), torch::optim::AdamOptions(1e-3));


    Context_t context;
    initContext(&context);

    auto avg_error = DBL_MAX;
    while (avg_error > 0.01 && epochs < maxEpochs) {
        // We want a hundredth precision

        for (i = 0; i < trainCycles; i++) {
            resetContext(&context);
            context.gameStatus = NOT_STARTED;

            getRandomState(&context, 1, 100);

            const float hOut = heuristic(&context);
            torch::Tensor y_gt = torch::tensor({hOut}, torch::TensorOptions().dtype(torch::kFloat));
            const torch::Tensor y_pred = model->forward(&context);
            torch::Tensor loss = torch::abs(y_pred - y_gt);

            optimizer->zero_grad();
            loss.backward();
            optimizer->step();
        }

        double totError = 0;
        for (i = 0; i < testCycles; i++) {
            resetContext(&context);
            getRandomState(&context, 1, 60);

            const double hOutput = heuristic(&context);
            torch::Tensor y_gt = torch::tensor({hOutput}, torch::TensorOptions().dtype(torch::kFloat));
            torch::Tensor y_pred = model->forward(&context);

            totError += torch::abs(y_pred - y_gt).item<double>();
        }
        avg_error = totError / testCycles;

        printf("Avg. Error: %lf\n", avg_error);
        epochs++;
    }

    model->save_model(optimizer);
}

void FromFilesTrainer::train(const bool toLoad) {
    auto model = HiveCNNEnhanced(getFilename());
    const std::shared_ptr<torch::optim::Optimizer> optimizer =
            std::make_shared<torch::optim::AdamW>(model->parameters(), torch::optim::AdamWOptions(1e-3));
    torch::nn::SmoothL1Loss smoothL1;

    if (toLoad && std::filesystem::exists(getFilename())) {
        model->load_model(optimizer);
    }

    const torch::Device device(torch::cuda::is_available() ? torch::kCUDA : torch::kCPU);
    model->to(device);

    MinManager minManager = MinManager();

    float total_loss = 0.0f;
    int steps = 0, numEntry = 0, signs = 0;

    std::vector<fs::directory_entry> entries;
    for (const auto &entry: fs::directory_iterator(dirName_)) {
        entries.push_back(entry);
    }

    std::mt19937 g(420);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::shuffle(entries.begin(), entries.end(), g);

    const auto num_selected = static_cast<size_t>(entries.size() * 0.01l);
    std::vector selected(entries.begin(), entries.begin() + num_selected);

    for (size_t epoch = 0; epoch < 15; epoch++) {
        for (const auto &entry: selected) {
            if (entry.is_regular_file()) {
                numEntry++;
                minManager.load(entry.path().string());
                float result = minManager.result();
                torch::Tensor yT = torch::tensor({result}, torch::TensorOptions().dtype(torch::kFloat)).to(
                    torch::cuda::is_available() ? torch::kCUDA : torch::kCPU);
                yT = yT.reshape({1, 1});
                while (!minManager.isEnded()) {
                    const Context_t *context = minManager.getNext();
                    if (dis(g) < 0.05 || context->turn <= 6)
                        continue;
                    torch::Tensor yP = model->forward(context);
                    torch::Tensor loss = smoothL1(yP, yT);
                    auto signMismatch = torch::where((yP * yT) < 0, torch::ones_like(loss), torch::zeros_like(loss)).mean();
                    loss += signMismatch;
                    signs += signMismatch.item<float>();

                    optimizer->zero_grad();
                    loss.backward();
                    optimizer->step();

                    total_loss += loss.item<float>();
                    steps++;
                }

                if (numEntry % 200 == 0)
                    std::cout << "Processed " << numEntry << " files out of " << selected.size() << std::endl;

            }
        }

        std::cout << "Epoch " << epoch + 1 << " | Loss: " << total_loss / steps << " | Wrong signs: " << signs << "/" << steps << std::endl;
        total_loss = 0.0f;
        steps = 0;
        numEntry = 0;
        signs = 0;
        model->save_partial(optimizer, epoch + 1);
    }


}

void SelfPlayTrainer::train(const bool toLoad) {
    auto ourModel = HiveCNNEnhanced(getFilename());
    auto enemyModel = HiveCNNEnhanced(getFilename());
    if (toLoad && std::filesystem::exists(getFilename())) {
        ourModel->load_model();
        enemyModel->load_model();
    }
    Context_t context;

    for (uint16_t i = 0; i < 100; i++) {
        std::string dataFile;
        printf("--- Starting game %d /100 ---", i + 1);
        initContext(&context);
        resetContext(&context);
        setDataFileName(dataFile, i);
        auto processor = Processor(dataFile);
        processor.newGame();
        const bool areWeWhite = i % 2 == 0;

        while (context.gameStatus != DRAW && context.gameStatus != WHITE_WON && context.gameStatus != BLACK_WON &&
               context.turn <= 100) {
            Piece_t bestMove;
            if (context.curColor == WHITE)
                negamax_net(&context, 0, 1, true, &bestMove, areWeWhite ? *ourModel : *enemyModel);
            else
                negamax_net(&context, 0, 1, false, &bestMove, areWeWhite ? *enemyModel : *ourModel);
            addOurMove(&context, &bestMove);
            if (context.gameStatus != DRAW && context.gameStatus != WHITE_WON && context.gameStatus != BLACK_WON)
                processor.playMove(&context);
        }

        processor.endGame(&context, resultOf(&context));
    }

    // Training
}

double MzingaHeuristicTrainer::heuristic(Context_t *context) {
    return mzingaHeuristic(context);
}
