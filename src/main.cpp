//
// Created by f3m on 28/05/25.
//


#include <cstring>
#include <thread>
#include <chrono>
#include <utils.h>
#include <logger.h>
#include <trainings.hpp>
#include <tree.hpp>


int main()
{
#ifdef Debug
    logD(stdout, "Launched in Debug Mode!\n");
#endif

    bool firstTime = true;
    initTree();

    size_t buf_size = 128;
    auto buffer = static_cast<char *>(malloc(sizeof(char) * buf_size));

    MzingaContext_t mzingaContext;
    initMzingaContext(&mzingaContext);

    setvbuf(stdout, nullptr, _IOLBF, 0);

    printInfo();

    while (true)
    {
#ifdef Debug
        printf("> ");
#endif

        const int64_t read = getline(&buffer, &buf_size, stdin);
        if (read == -1) // EOF
            break;
        if (read == 1) //Empty line
            continue;

        buffer[read - 1] = '\0';

        switch (parseCommand(buffer))
        {
            case INFO:
                printInfo();
                break;
            case NEWGAME:
                resetMzingaContext(&mzingaContext);
                printGameString(&mzingaContext);
                if (!firstTime)
                {
                    cleanTree();
                    initTree();
                }
                firstTime = false;
                break;
            case PLAY:
            {
                char *move = buffer + 5;
                addMazingaMove(&mzingaContext, move);
                printGameString(&mzingaContext);
                adversaryMove(move);
                break;
            }
            case BESTMOVE:
            {
                // std::this_thread::sleep_for(std::chrono::duration<double>(4.7));
                const Node_t *bestChild = getBestChild();
                printMove(&bestChild->context, &bestChild->move);
                break;
            }
            case INVALID:
                printf("err Unknown command\n");
        }
    }


    free(buffer);
    cleanMzingaContext(&mzingaContext);
    cleanTree();
    return EXIT_SUCCESS;
}
