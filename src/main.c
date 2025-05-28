//
// Created by f3m on 28/05/25.
//


#include <string.h>
#include <utils.h>
#include <logger.h>

int main()
{
#ifdef Debug
    logD(stdout, "Launched in Debug Mode!\n");
#endif

    //TODO: (PARALLELISMO) fai partire il thread che genera l'albero

    size_t buf_size = 128;
    char *buffer = malloc(sizeof(char) * buf_size);


    Context_t context = {};
    initContext(&context);

    while (true)
    {
#ifdef Debug
        printf("> ");
#endif
        const size_t read = getline(&buffer, &buf_size, stdin);
        if (read == -1) // EOF
            break;
        if (read == 1) //Empty line
            continue;

        buffer[read - 1] = '\0';

        const Command_t command = parseCommand(buffer);

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
                strtok(buffer, " ");
                char *move = strtok(nullptr, " ");
                doMove(&context, move);
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
