#include "main.h"

#include <string.h>

int main(void)
{
#if Debug
    printf("Started in Debug mode!\n");
#endif

    if (initGame())
        return EXIT_FAILURE;
    const int out = mainLoop();
    cleanGame();
    return out;
}

#ifdef WIN32
size_t getline(char **lineptr, size_t *n, FILE *stream)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL)
    {
        return -1;
    }
    if (stream == NULL)
    {
        return -1;
    }
    if (n == NULL)
    {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF)
    {
        return -1;
    }
    if (bufptr == NULL)
    {
        bufptr = malloc(128);
        if (bufptr == NULL)
        {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while (c != EOF)
    {
        if ((p - bufptr) > (size - 1))
        {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL)
            {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n')
        {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}
#endif

int mainLoop()
{
    size_t lineSize = 64;
    char *line = malloc(64 * sizeof(char));

    //TODO: printare come scrivere una mossa
    printBoardStatus();

    bool add;
    uint8_t idx;
    size_t bytesRead;
    while ((int) (bytesRead = getline(&line, &lineSize, stdin)) != -1)
    {
        int8_t move[6];
        if (bytesRead == 1)
            continue;
        if (bytesRead == 2 && strcmp(line, "q\n") == 0)
            break;
        line[bytesRead - 1] = '\0';

        if (!isEncodingValid(line, move, &add))
        {
            E_Print("Encoding not valid!\n");
            continue;
        }
        if (!isMoveValid(move, &idx, add))
        {
            E_Print("Move is not valid!\n");
            continue;
        }

        if (add)
        {
            addPiece(move[3], move[0], move[1], move[2]);
        } else
        {
            movePiece(idx, move[3], move[4], move[5]);
        }

        //TODO: controllare se si ha vinto
        printBoardStatus();
    }
    free(line);

    return 0;
}
