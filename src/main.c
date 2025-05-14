//
// Created by f3m on 28/03/25.
//

#include "main.h"


#ifdef WIN32
size_t getline(char **lineptr, size_t *n, FILE *stream)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL)
    {
        return (size_t) -1;
    }
    if (stream == NULL)
    {
        return (size_t) -1;
    }
    if (n == NULL)
    {
        return (size_t) -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF)
    {
        return (size_t) -1;
    }
    if (bufptr == NULL)
    {
        bufptr = malloc(128);
        if (bufptr == NULL)
        {
            return (size_t) -1;
        }
        size = 128;
    }
    p = bufptr;
    while (c != EOF)
    {
        if ((size_t) (p - bufptr) > size - 1)
        {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL)
            {
                return (size_t) -1;
            }
        }
        *p++ = (char) c;
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


void print_info(void)
{
    printf("id hAIve v1.0\n");
    printf("Mosquito;Ladybug;Pillbug\n");
}

/*
    0: correctly parsed
    1: error in parsing
*/
int parseGameTypeString(const char *str, Context_t *context)
{
    char base[] = "Base";
    for (size_t c = 0; c < 4; c++)
    {
        if (str[c] != base[c])
        {
            printf("err Parsing GameTypeString failed.\nok\n");
            return 1;
        }
    }
    if (str[4] == '\0')
        return 0;
    else if (str[4] != '+')
    {
        printf("err Parsing GameTypeString failed.\nok\n");
        return 1;
    } else
    {
        for (size_t c = 5; str[c] != '\0' && c <= 7; c++)
        {
            switch (str[c])
            {
                case 'M':
                    context->gameType.mosquito = true;
                    break;
                case 'L':
                    context->gameType.ladybug = true;
                    break;
                case 'P':
                    context->gameType.pillbug = true;
                    break;
                default:
                    printf("err Parsing GameTypeString failed.\nok\n");
                    return 1;
            }
        }
    }
    return 0;
}

void print_gamestring(const Context_t *context)
{
    const GameType_t gameType = context->gameType;
    const char *moves = context->moves;

    // GameType
    printf("Base");
    if (gameType.ladybug || gameType.pillbug || gameType.mosquito)
    {
        printf("+");
        if (gameType.mosquito) printf("M");
        if (gameType.ladybug) printf("L");
        if (gameType.pillbug) printf("P");
    }
    printf(";");

    // GameStatus
    switch (context->gameStatus)
    {
        case WHITE_WON: printf("WhiteWins;");
            break;
        case BLACK_WON: printf("BlackWins;");
            break;
        case DRAW: printf("Draw;");
            break;
        case IN_PROGRESS: printf("InProgress;");
            break;
        case NOT_STARTED: printf("NotStarted;");
            break;
        case NOT_INITIALIZED:
            break;
    }

    // TurnString
    switch (context->curColor)
    {
        case WHITE: printf("White[%d]", context->turn / 2 + 1);
            break;
        case BLACK: printf("Black[%d]", context->turn / 2);
            break;
        case NULLCOLOR: return;
    }
    if (moves[0] == '\0')
    {
        printf("\nok\n");
        return;
    }
    printf(";");

    // Moves
    printf("%s", moves);
    printf("\nok\n");
}

// -1: exit
// 0: nothing to do
// 1: newgame
// 2: play move
// 3: bestmove
// 4: pass
// 5: validmoves
int manage_command(char *buffer)
{
    char *command = strdup(buffer);
    command = strtok(command, " ");

    if (command == NULL)
    {
        // FIXME: è impossibile sia NULL
        return -1;
    }
    if (strcmp(command, "exit") == 0)
    {
        return -1;
    } else if (strcmp(command, "info") == 0)
    {
        print_info();
    } else if (strcmp(command, "options") == 0)
    {
        // FIXME: a che serve sto controllo se tanto non fa niente? COMPATIBILITA'
        // Doesn't print anything
    } else if (strcmp(command, "newgame") == 0)
    {
        return 1;
    } else if (strcmp(command, "play") == 0)
    {
        return 2;
    } else if (strcmp(command, "pass") == 0)
    {
        return 4;
    } else if (strcmp(command, "validmoves") == 0)
    {
        return 5;
    } else if (strcmp(command, "bestmove") == 0)
    {
        return 3;
    } else
    {
        printf("Unknown command: %s\n", command);
    }

    printf("ok\n");
    return 0;
}


// void test()
// {
//     Context_t context;
//     initContext(&context);
//
//     context.board[MtA(0, 0, -2)] = W_QUEEN;
//     context.board[MtA(0, 0, 4)] = B_QUEEN;
//     context.board[MtA(0, 0, 0)] = W_BEETLE_1;
//     context.board[MtA(0, 0, 2)] = B_BEETLE_1;
//
//     context.idToPos[W_QUEEN] = (Position_t){0, 0, -2};
//     context.idToPos[B_QUEEN] = (Position_t){0, 0, 4};
//     context.idToPos[W_BEETLE_1] = (Position_t){0, 0, 0};
//     context.idToPos[B_BEETLE_1] = (Position_t){0, 0, 2};
//
//     context.curColor = WHITE;
//
//     Piece_t piece = {W_ANT_1, context.idToPos[W_ANT_1]};
//
//     uint_fast8_t mSize = 0;
//     Piece_t *moves;
//
//     bool visited[28] = {};
//     for (uint_fast8_t i = 0; i < 28; ++i)
//     {
//         if (context.idToPos[i].z == -1)
//             visited[i] = true;
//     }
//
//     getMoves(&context, &moves, &mSize);
//
//
//     for (int i = 0; i < mSize; ++i)
//     {
//         printf("ID: %d, Move: (%d,%d,%d)\n", moves[i].id, moves[i].position.z, moves[i].position.y, moves[i].position.x);
//     }
//
//
//     cleanContext(&context);
// }


int main(void)
{
#ifdef Debug
    D_Print("Launched in Debug Mode!\n");
#endif

    setvbuf(stdout, NULL, _IONBF, 0);
    size_t buf_size = 128;
    char *buffer = malloc(sizeof(char) * buf_size);

    Context_t context = {};
    initContext(&context);

    while (true)
    {
        printf("> ");
        size_t read = getline(&buffer, &buf_size, stdin);
        if (read == -1) // EOF
            break;
        while (buffer[0] == ' ')
        {
            buffer++;
            read--;
        }
        buffer[read - 1] = '\0';
        if (read == 1) continue; // Empty line
        const int result = manage_command(buffer);

        //TODO: cambiare questi result con uno switch
        if (result == -1) break;
        if (result <= 0) continue;
        if (result == 1)
        {
            // newgame
            cleanContext(&context);
            initContext(&context);
            context.gameStatus = NOT_STARTED;
            if (read == 8)
            {
                print_gamestring(&context);
                continue;
            }
            char *parameters = buffer + 8;

            if (strchr(parameters, ';') == NULL)
            {
                // Command is like 'Base+MLP'
                if (parseGameTypeString(parameters, &context) == 1)
                {
                    cleanContext(&context);
                    continue;
                }
            } else
            {
                // Command is GameString
                if (convertFromMZinga(parameters, &context) == 1)
                {
                    cleanContext(&context);
                    continue;
                }
            }
            print_gamestring(&context);
        } else if (result == 2)
        {
            // play command
            if (context.gameStatus == NOT_INITIALIZED)
            {
                printf("err Game not yet started \n");
                continue;
            }
            char *move = buffer + 5;
            playMove(&context, move);
            print_gamestring(&context);
            debugPrint(&context);
        } else if (result == 3)
        {
            // bestmove
            // TODO: Get best move out
        } else if (result == 4)
        {
            playMove(&context, "pass");
            print_gamestring(&context);
            debugPrint(&context);
            debugBoardPrint(&context);
        } else
        {
            Piece_t *moves;
            uint_fast8_t mSize = 0;
            getMoves(&context, &moves, &mSize);
            for (uint_fast8_t i = 0; i < mSize; i++)
            {
                char *move = deconvertMove(&context, &moves[i]);
                printf("%s", move);
                if (i != mSize - 1) printf(";");
                free(move);
            }
            printf("\n");
        }
    }
    free(buffer);
    return 0;
}
