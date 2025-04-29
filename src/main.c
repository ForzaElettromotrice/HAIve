//
// Created by f3m on 28/03/25.
//

#include "main.h"

// Assumiamo che queste condizioni siano sempre vere altrimenti si sfancula tutto
static_assert(sizeof(Pieces_t) == 1);
static_assert(sizeof(Piece_t) == 4);

#ifdef WIN32
size_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
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


void print_info()
{
    printf("id hAIve v1.0\n");
    printf("Mosquito;Ladybug;Pillbug\n");
}

/*
    0: correctly parsed
    1: error in parsing
*/
int parseGameTypeString(const char* str, Context_t *context) {

    char base[] = "Base";
    for (size_t c = 0; c < 4; c++) {
        if (str[c] != base[c]) {
            printf("err Parsing GameTypeString failed.\nok\n");
            return 1;
        }
    }
    if (str[4] == '\0')
        return 0;
    else if (str[4] != '+') {
        printf("err Parsing GameTypeString failed.\nok\n");
        return 1;
    }
    else {
        for (size_t c = 5; str[c] != '\0' && c <= 7; c++) {
            switch (str[c]) {
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
    const char* moves = context->moves;

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
    if (moves[0] == '\0') {
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
int manage_command(char *buffer)
{
    
    char* command = strdup(buffer);
    command = strtok(command, " ");

    if (command == NULL)
    {
        // FIXME: è impossibile sia NULL
        return -1;
    }
    if (strcmp(command, "exit") == 0)
    {
        return -1;
    }
    else if (strcmp(command, "info") == 0)
    {
        print_info();
    } 
    else if (strcmp(command, "options") == 0)
    {
        // FIXME: a che serve sto controllo se tanto non fa niente?
        //Doesn't print anything
    } 
    else if (strcmp(command, "newgame") == 0)
    {
        // TODO: DO NEWGAME
        return 1;
    } 
    else if (strcmp(command, "play") == 0)
    {
        // TODO: Play move
        return 2;
    } 
    else if (strcmp(command, "pass") == 0)
    {
        // TODO: Play pass
    } 
    else if (strcmp(command, "validmoves") == 0)
    {
        // TODO: Print valid moves
    } 
    else if (strcmp(command, "bestmove") == 0)
    {
        return 3;
    } 
    else
    {
        printf("Unknown command: %s\n", command);
    }

    printf("ok\n");
    return 0;
}

void initContext(Context_t *context) {

    context->curColor = WHITE;
    context->turn = 1;
    context->moves = calloc(1024, sizeof(char));
    context->movesSize = 1024;
    context->board = malloc(BOARD_SIZE * sizeof(Pieces_t));
    for (size_t i = 0; i < BOARD_SIZE; i++) context->board[i] = NULLPIECE;
    context->idToPos = malloc(NUM_PIECES * sizeof(Position_t));
    for (size_t i = 0; i < NUM_PIECES; i++) context->idToPos[i].x = -1;
    context->lastMovedPiece = NULLPIECE;

}

void cleanContext(Context_t *context) {

    free(context->moves);
    free(context->board);
    free(context->idToPos);
    memset(context, 0, sizeof(Context_t));

}


int main(void)
{
#ifdef Debug
    D_Print("Launched in Debug Mode!\n");
#endif

    size_t buf_size = 128;
    char *buffer = malloc(sizeof(char) * buf_size);

    Context_t context = {};
    initContext(&context);

    while (true)
    {
        const size_t read = getline(&buffer, &buf_size, stdin);
        if (read == -1) // EOF
            break;
        buffer[read - 1] = '\0'; 
        if (read == 1) continue; // Empty line

        const int result = manage_command(buffer);
        if (result == -1) break;
        else if (result <= 0) continue;
        else if (result == 1)
        {   // newgame
            cleanContext(&context);
            initContext(&context);
            if (read == 8) {
                print_gamestring(&context);
                continue;
            }
            char *parameters = buffer + 8;

            if (strchr(parameters, ';') == NULL) {
                // Command is like 'Base+MLP'
                if (parseGameTypeString(parameters, &context) == 1) {
                    cleanContext(&context);
                    continue;
                }
            } else {
                // Command is GameString
                if (convertFromMZinga(parameters, &context) == 1) {
                    cleanContext(&context);
                    continue;
                }
            }
            print_gamestring(&context);
            
        }
        else if (result == 2) {
            // play command
            char* move = buffer + 5;
            context.gameStatus = IN_PROGRESS;
            playMove(&context, move);
            print_gamestring(&context);
            debugPrint(&context);
        }
        else if (result == 3) {
            // bestmove
            // TODO: Get best move out
        }
    }
    free(buffer);
    return 0;

}
