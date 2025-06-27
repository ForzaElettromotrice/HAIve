//
// Created by f3m on 28/05/25.
//

#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <logger.h>
#include <enums.h>
#include <xxhash.h>

const Piece_t pass = {NULLPIECE, {0, 0, 0}};

uint64_t hashPiece(const Position_t *pos, const Pieces_t *board)
{
    const int_fast8_t z = pos->z;
    const int_fast8_t y = pos->y;
    const int_fast8_t x = pos->x;

    int_fast8_t max = -2;
    uint_fast8_t idx = 0;
    for (uint_fast8_t i = 0; i < 6; i++)
    {
        const int_fast8_t newY = (int_fast8_t) (y + directions[i][0]);
        const int_fast8_t newX = (int_fast8_t) (x + directions[i][1]);

        const Pieces_t neighbor = board[MtA(z, newY, newX)];
        if (neighbor == NULLPIECE)
            continue;

        if (neighbor > max)
        {
            max = neighbor;
            idx = i;
        }
    }

    for (uint_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (y + directions[(idx - 1) % 6][0]);
        const int_fast8_t newX = (int_fast8_t) (x + directions[(idx - 1) % 6][1]);

        if (board[MtA(z, newY, newX)] != max)
            break;

        idx = (idx - 1) % 6;
    }


    uint64_t hash = 0;
    for (uint_fast8_t i = 0; i < 6; ++i, idx = (idx + 1) % 6)
    {
        const int_fast8_t newY = (int_fast8_t) (y + directions[idx][0]);
        const int_fast8_t newX = (int_fast8_t) (x + directions[idx][1]);

        int8_t value = 0;
        switch (board[MtA(z, newY, newX)])
        {
            case B_QUEEN:
                value = 0;
                break;
            case B_PILLBUG:
                value = 1;
                break;
            case B_LADYBUG:
                value = 2;
                break;
            case B_MOSQUITO:
                value = 3;
                break;
            case B_ANT_1:
            case B_ANT_2:
            case B_ANT_3:
                value = 4;
                break;
            case B_GRASSHOPPER_1:
            case B_GRASSHOPPER_2:
            case B_GRASSHOPPER_3:
                value = 5;
                break;
            case B_BEETLE_1:
            case B_BEETLE_2:
                value = 6;
                break;
            case B_SPIDER_1:
            case B_SPIDER_2:
                value = 7;
                break;
            case W_QUEEN:
                value = 8;
                break;
            case W_PILLBUG:
                value = 9;
                break;
            case W_LADYBUG:
                value = 10;
                break;
            case W_MOSQUITO:
                value = 11;
                break;
            case W_ANT_1:
            case W_ANT_2:
            case W_ANT_3:
                value = 12;
                break;
            case W_GRASSHOPPER_1:
            case W_GRASSHOPPER_2:
            case W_GRASSHOPPER_3:
                value = 13;
                break;
            case W_BEETLE_1:
            case W_BEETLE_2:
                value = 14;
                break;
            case W_SPIDER_1:
            case W_SPIDER_2:
                value = 15;
                break;
            case NULLPIECE:
                value = 16;
                break;
        }

        hash = (hash << 8) + value;
    }
    return hash;
}
uint64_t hashAll(const Pieces_t *board, const Position_t *positions)
{
    uint64_t toHash[28];

    for (uint_fast8_t i = 0; i < 28; ++i)
        toHash[i] = hashPiece(&positions[i], board);

    return XXH3_64bits(toHash, 28 * sizeof(uint64_t));
}


void initContext(Context_t *context)
{
    context->board = malloc(BOARD_SIZE * sizeof(Pieces_t));
    context->moves = calloc(1024, sizeof(char));
    context->idToPos = malloc(NUM_PIECES * sizeof(Position_t));
    context->movesSize = 1024;

    resetContext(context);
}
void resetContext(Context_t *context)
{
    memset(context->board, 0xff, BOARD_SIZE * sizeof(Pieces_t));
    memset(context->idToPos, 0xff, NUM_PIECES * sizeof(Position_t));
    memset(context->moves, 0x00, context->movesSize);

    context->turn = 1;
    context->curColor = WHITE;
    context->gameStatus = NOT_STARTED;
    context->lastMovedPiece = NULLPIECE;
}
void copyContext(const Context_t *src, Context_t *dst)
{
    memcpy(dst, src, sizeof(Context_t));

    dst->moves = malloc(dst->movesSize * sizeof(char));
    dst->board = malloc(BOARD_SIZE * sizeof(Pieces_t));
    dst->idToPos = malloc(NUM_PIECES * sizeof(Position_t));

    memcpy(dst->moves, src->moves, dst->movesSize * sizeof(char));
    memcpy(dst->board, src->board, BOARD_SIZE * sizeof(Pieces_t));
    memcpy(dst->idToPos, src->idToPos, NUM_PIECES * sizeof(Position_t));
}
void cleanContext(const Context_t *context)
{
    free(context->moves);
    free(context->board);
    free(context->idToPos);
}


Command_t parseCommand(const char *command)
{
    if (strncmp(command, "play", 4) == 0)
        return PLAY;
    if (strncmp(command, "newgame", 7) == 0)
        return NEWGAME;
    if (strncmp(command, "bestmove", 8) == 0)
        return BESTMOVE;
    if (strncmp(command, "info", 4) == 0)
        return INFO;

    return INVALID;
}
Pieces_t parsePiece(const char *piece)
{
    const Colors_t color = piece[0] == 'w' ? WHITE : BLACK;

    piece++;

    if (strncmp(piece, "Q", 1) == 0)
        return color == WHITE ? W_QUEEN : B_QUEEN;
    if (strncmp(piece, "S1", 2) == 0)
        return color == WHITE ? W_SPIDER_1 : B_SPIDER_1;
    if (strncmp(piece, "S2", 2) == 0)
        return color == WHITE ? W_SPIDER_2 : B_SPIDER_2;
    if (strncmp(piece, "G1", 2) == 0)
        return color == WHITE ? W_GRASSHOPPER_1 : B_GRASSHOPPER_1;
    if (strncmp(piece, "G2", 2) == 0)
        return color == WHITE ? W_GRASSHOPPER_2 : B_GRASSHOPPER_2;
    if (strncmp(piece, "G3", 2) == 0)
        return color == WHITE ? W_GRASSHOPPER_3 : B_GRASSHOPPER_3;
    if (strncmp(piece, "A1", 2) == 0)
        return color == WHITE ? W_ANT_1 : B_ANT_1;
    if (strncmp(piece, "A2", 2) == 0)
        return color == WHITE ? W_ANT_2 : B_ANT_2;
    if (strncmp(piece, "A3", 2) == 0)
        return color == WHITE ? W_ANT_3 : B_ANT_3;
    if (strncmp(piece, "B1", 2) == 0)
        return color == WHITE ? W_BEETLE_1 : B_BEETLE_1;
    if (strncmp(piece, "B2", 2) == 0)
        return color == WHITE ? W_BEETLE_2 : B_BEETLE_2;
    if (strncmp(piece, "L", 1) == 0)
        return color == WHITE ? W_LADYBUG : B_LADYBUG;
    if (strncmp(piece, "M", 1) == 0)
        return color == WHITE ? W_MOSQUITO : B_MOSQUITO;
    if (strncmp(piece, "P", 1) == 0)
        return color == WHITE ? W_PILLBUG : B_PILLBUG;

    return NULLPIECE; // Default case for invalid input
}
Piece_t parseMove(const Position_t *idToPos, char *move)
{
    if (strcmp(move, "pass") == 0)
        return (Piece_t){-1, {-1, -1, -1}};

    const char *firstStr = strtok(move, " ");
    const char *secondStr = strtok(NULL, " ");

    const Pieces_t first = parsePiece(firstStr);

    if (secondStr == NULL)
        return (Piece_t){first, {0, 0, 0}};


    int8_t direction;
    Pieces_t second;
    if (secondStr[0] == 'w' || secondStr[0] == 'b')
    {
        second = parsePiece(secondStr);
        switch (secondStr[strlen(secondStr - 1)])
        {
            case '/':
                direction = RIGHT_UP;
                break;
            case '-':
                direction = RIGHT;
                break;
            case '\\':
                direction = RIGHT_DOWN;
                break;
            default:
                direction = -1;
        }
    } else
    {
        second = parsePiece(secondStr + 1);
        switch (secondStr[0])
        {
            case '\\':
                direction = LEFT_UP;
                break;
            case '-':
                direction = LEFT;
                break;
            case '/':
                direction = LEFT_DOWN;
                break;
            default:
                direction = -2;
                logE(stderr, "Non dovremmo mai entrare qua...\n");
                break;
        }
    }
    Position_t secondPos = idToPos[second];
    if (direction == -1)
    {
        secondPos.z++;
        return (Piece_t){first, secondPos};
    }

    secondPos.y = (int8_t) (secondPos.y + directions[direction][0]);
    secondPos.x = (int8_t) (secondPos.x + directions[direction][1]);
    return (Piece_t){first, secondPos};
}


bool isSurrounded(const Context_t *context, const Pieces_t id)
{
    const int8_t z = context->idToPos[id].z;
    if (z == -1)
        return 0;
    const int8_t y = context->idToPos[id].y;
    const int8_t x = context->idToPos[id].x;

    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        if (context->board[MtA(z, newY, newX)] == NULLPIECE)
            return false;
    }
    return true;
}
GameStatus_t checkGameStatus(const Context_t *context)
{
    // TODO: Check draw

    if (isSurrounded(context, B_QUEEN))
        return WHITE_WON;
    if (isSurrounded(context, W_QUEEN))
        return BLACK_WON;

    return IN_PROGRESS;
}
int_fast8_t howManyAround(const Context_t *context, const Pieces_t id, bool friendly)
{
    char nearAround = 0;
    const int8_t z = context->idToPos[id].z;
    if (z == -1)
        return 0;
    const int8_t y = context->idToPos[id].y;
    const int8_t x = context->idToPos[id].x;
    for (int_fast8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);
        Pieces_t neighbor = context->board[MtA(z, newY, newX)];

        if (neighbor == NULLPIECE)
            continue;

        if ((neighbor % 14 == id % 14) && friendly)
            nearAround++;
        else if ((neighbor % 14 != id % 14) && !friendly)
            nearAround++;
    }
    return nearAround;
}

void addMazingaMove(Context_t *context, const char *move)
{
    const size_t moveLen = strlen(move);
    const size_t currentLen = strlen(context->moves);

    if (currentLen + moveLen + 1 >= context->movesSize)
    {
        const size_t newSize = context->movesSize * 2 + moveLen + 1;
        char *newMoves = realloc(context->moves, newSize);
        context->moves = newMoves;
        context->movesSize = newSize;
    }

    strcat(context->moves, ";");
    strcat(context->moves, move);
}
void addOurMove(Context_t *context, const Piece_t *move)
{
    context->turn++;
    context->curColor *= -1;
    if (move->id == -1)
        return;

    const Position_t oldPos = context->idToPos[move->id];
    if (oldPos.z != -1)
        context->board[MtA(oldPos.z, oldPos.y, oldPos.x)] = NULLPIECE;


    const int8_t z = move->position.z;
    const int8_t y = move->position.y;
    const int8_t x = move->position.x;
    context->board[MtA(z, y, x)] = move->id;

    context->idToPos[move->id] = move->position;

    context->lastMovedPiece = move->id;
    context->gameStatus = checkGameStatus(context);
}
void doMove(Context_t *context, char *move)
{
    addMazingaMove(context, move);

    //TODO: salva l'hash della board
    const Piece_t *piece = parseMove(context->idToPos, move);
    addOurMove(context, piece);
}


void parsePieceToMazinga(const Pieces_t piece, char *dst)
{
    switch (piece)
    {
        case B_QUEEN:
            strcat(dst, "Q");
            break;
        case B_SPIDER_1:
            strcat(dst, "S1");
            break;
        case B_SPIDER_2:
            strcat(dst, "S2");
            break;
        case B_GRASSHOPPER_1:
            strcat(dst, "G1");
            break;
        case B_GRASSHOPPER_2:
            strcat(dst, "G2");
            break;
        case B_GRASSHOPPER_3:
            strcat(dst, "G3");
            break;
        case B_ANT_1:
            strcat(dst, "A1");
            break;
        case B_ANT_2:
            strcat(dst, "A2");
            break;
        case B_ANT_3:
            strcat(dst, "A3");
            break;
        case B_BEETLE_1:
            strcat(dst, "B1");
            break;
        case B_BEETLE_2:
            strcat(dst, "B2");
            break;
        case B_LADYBUG:
            strcat(dst, "L");
            break;
        case B_MOSQUITO:
            strcat(dst, "M");
            break;
        case B_PILLBUG:
            strcat(dst, "P");
            break;
        default:
            break;
    }
}


void printInfo()
{
    printf("id hAIve v1.1\n");
    printf("Mosquito;Ladybug;Pillbug\n");
}
void printGameString(const Context_t *context)
{
    // GameType
    printf("Base+MLP;");

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
        case NULLCOLOR:
            logE(stderr, "Questo non dovrebbe succedere...\n");
            return;
    }
    if (context->moves[0] == '\0')
    {
        printf("\nok\n");
        return;
    }

    // Moves
    printf("%s", context->moves);
    printf("\nok\n");
}


void printMove(const Context_t *context, const Piece_t move)
{
    const int8_t z = context->idToPos[move.id].z;
    const int8_t y = context->idToPos[move.id].y;
    const int8_t x = context->idToPos[move.id].x;

    char out[20];
    sprintf(out, "%s", move.id < 14 ? "b" : "w");
    parsePieceToMazinga(move.id % 14, out);
    strcat(out, " ");

    bool hover = true;
    for (int8_t i = 0; i < 6; ++i)
    {
        const int_fast8_t newY = (int_fast8_t) (directions[i][0] + y);
        const int_fast8_t newX = (int_fast8_t) (directions[i][1] + x);

        const Pieces_t neighbor = context->board[MtA(z, newY, newX)];
        if (neighbor == NULLPIECE)
            continue;


        switch (i)
        {
            case RIGHT_UP:
                strcat(out, "/");
                parsePieceToMazinga(neighbor, out);
                break;
            case RIGHT:
                strcat(out, "-");
                parsePieceToMazinga(neighbor, out);
                break;
            case RIGHT_DOWN:
                strcat(out, "\\");
                parsePieceToMazinga(neighbor, out);
                break;
            case LEFT_DOWN:
                parsePieceToMazinga(neighbor, out);
                strcat(out, "/");
                break;
            case LEFT:
                parsePieceToMazinga(neighbor, out);
                strcat(out, "-");
                break;
            case LEFT_UP:
                parsePieceToMazinga(neighbor, out);
                strcat(out, "\\");
                break;
            default:
                logE(stderr, "Non dovremmo mai capitare qui...\n");
                break;
        }
        hover = false;
        break;
    }

    if (hover)
        parsePieceToMazinga(context->board[MtA(z-1, y, x)], out);
}

void bestMove(const Context_t *context)
{
    //TODO: prendi il figlio con valore maggiore dall'albero
    const Piece_t child = {0, {0, 0, 0}};
    printMove(context, child);
}

GameStatus_t getGameStatus(const Context_t *context) {
    return context->gameStatus;
}

bool isContextEnded(const Context_t* context) {
    const GameStatus_t gameStatus = context->gameStatus;
    return gameStatus == WHITE_WON || gameStatus == BLACK_WON || gameStatus == DRAW;
}