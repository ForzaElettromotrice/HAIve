//
// Created by f3m on 28/05/25.
//

extern "C" {
#include <string.h>
#include <utils.h>
#include <logger.h>
}

#include <trainings.h>

int main()
{
#ifdef Debug
    logD(stdout, "Launched in Debug Mode!\n");
#endif

    trainSelfPlay(false);
    return 0;

    //TODO: (PARALLELISMO) fai partire il thread che genera l'albero


    size_t buf_size = 128;
    auto buffer = static_cast<char *>(malloc(sizeof(char) * buf_size));

    Context_t context = {};
    initContext(&context);

    while (true)
    {
#ifdef Debug
        printf("> ");
#endif

        const int32_t read = getline(&buffer, &buf_size, stdin);
        if (read == -1) // EOF
            break;
        if (read == 1) //Empty line
            continue;

        buffer[read - 1] = '\0';

        const Command_t command = parseCommand(buffer);
        char *move;

        switch (command)
        {
            case INFO:
                printInfo();
                break;
            case NEWGAME:
                resetContext(&context);
                printGameString(&context);
                //TODO:(PARALLELISMO) fare si che il processo che gestisce l'albero si resetti
                break;
            case PLAY:
                //TODO: ignora il play se è la nostra mossa
                move = buffer + 5;
                doMove(&context, move);
                printGameString(&context);
                break;
            case BESTMOVE:
                bestMove(&context);
                break;
            case INVALID:
                printf("err Unknown command\n");
        }
    }

    //TODO: (PARALLELISMO) chiudi il thread che fa partire l'albero
    cleanContext(&context);
    free(buffer);
    return EXIT_SUCCESS;
}
