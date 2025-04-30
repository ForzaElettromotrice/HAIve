//
// Created by f3m on 02/04/25.
//
#include "utils.h"


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


// Dalla stringa, ritorna il valore del pezzo
Pieces_t getPiece(const char *piece, char white)
{
    if (strcmp(piece, "Q") == 0)
        return white ? W_QUEEN : B_QUEEN;
    if (strcmp(piece, "S1") == 0)
        return white ? W_SPIDER_1 : B_SPIDER_1;
    if (strcmp(piece, "S2") == 0)
        return white ? W_SPIDER_2 : B_SPIDER_2;
    if (strcmp(piece, "G1") == 0)
        return white ? W_GRASSHOPPER_1 : B_GRASSHOPPER_1;
    if (strcmp(piece, "G2") == 0)
        return white ? W_GRASSHOPPER_2 : B_GRASSHOPPER_2;
    if (strcmp(piece, "G3") == 0)
        return white ? W_GRASSHOPPER_3 : B_GRASSHOPPER_3;
    if (strcmp(piece, "A1") == 0)
        return white ? W_ANT_1 : B_ANT_1;
    if (strcmp(piece, "A2") == 0)
        return white ? W_ANT_2 : B_ANT_2;
    if (strcmp(piece, "A3") == 0)
        return white ? W_ANT_3 : B_ANT_3;
    if (strcmp(piece, "B1") == 0)
        return white ? W_BEETLE_1 : B_BEETLE_1;
    if (strcmp(piece, "B2") == 0)
        return white ? W_BEETLE_2 : B_BEETLE_2;
    if (strcmp(piece, "L") == 0)
        return white ? W_LADYBUG : B_LADYBUG;
    if (strcmp(piece, "M") == 0)
        return white ? W_MOSQUITO : B_MOSQUITO;
    if (strcmp(piece, "P") == 0)
        return white ? W_PILLBUG : B_PILLBUG;

    return NULLPIECE; // Default case for invalid input
}

void cleanContext(Context_t* context) {

    free(context->moves);
    free(context->board);
    free(context->idToPos);
    memset(context, 0, sizeof(Context_t));

}

// NOTE: The context MUST BE EDITED in order to add the move
void manageMove(Context_t* context, Piece_t* move) {

    // FIXME: 'pass' :(
    
    // Delete from old position
    Position_t oldPosition = context->idToPos[move->id];
    uint8_t z = oldPosition.z;
    uint8_t y = oldPosition.y;
    uint8_t x = oldPosition.x;
    context->board[MtA(z, y, x)] = NULLPIECE;
    // Add to new position
    Position_t newPosition = move->position;
    z = newPosition.z;
    y = newPosition.y;
    x = newPosition.x;
    context->board[MtA(z, y, x)] = move->id;
    context->idToPos[move->id] = newPosition;

    // Manage other stuff
    context->turn += 1;
    context->curColor *= -1;
    context->lastMovedPiece = move->id;
    char* moveString = deconvertMove(context, move->id);
    addMove(context, moveString);
    // TODO: Check if win/draw and change game status!
}

void playMove(Context_t *context, char *move)
{
    addMove(context, move);
    if (parseMove(context, move) > 0)
        return;
    context->turn += 1;
    context->curColor *= -1;
    // TODO: Check if win/draw and change game status!
}

void addMove(Context_t *context, char *move)
{
    size_t moveLen = strlen(move);
    size_t currentLen = strlen(context->moves);

    if (currentLen + moveLen + 1 >= context->movesSize)
    {
        size_t newSize = context->movesSize * 2 + moveLen + 1;
        char *newMoves = realloc(context->moves, newSize);
        context->moves = newMoves;
        context->movesSize = newSize;
    }

    if (currentLen > 0)
        strcat(context->moves, ";");
    strcat(context->moves, move);
}

/*
    0: if parsing was successful
    1: if an error occurred
*/
int parseMove(Context_t *context, char *move)
{
    char white_piece;
    Pieces_t piece;
    Position_t *id_to_pos = context->idToPos;
    Pieces_t *pieces = context->board;

    if (strcmp(move, "pass") == 0)
        return 0;

    else if (context->turn <= 1) {
        // Primo pezzo
        if (move[0] != 'w')
            return EXIT_FAILURE;
        white_piece = 1;
        move++;
        piece = getPiece(move, white_piece);
        if (piece == NULLPIECE)
            return EXIT_FAILURE;
        id_to_pos[piece].x = 0;
        id_to_pos[piece].y = 0;
        id_to_pos[piece].z = 0;
        pieces[MtA(0, 0, 0)] = piece;
        context->lastMovedPiece = piece;

        return EXIT_SUCCESS;
    }

    char *space_pos = strchr(move, ' ');
    if (space_pos != NULL) *space_pos = '\0';
    if (move[0] == 'w')
        white_piece = 1;
    else
        white_piece = 0;
    move++;
    piece = getPiece(move, white_piece);
    
    if (piece == NULLPIECE)
        return EXIT_FAILURE;

    Pieces_t other_piece;
    uint8_t direction[2] = {0, 0};
    move += strlen(move);
    move++;
    if (move[0] == '-')
    {
        direction[0] = directions[LEFT][0];
        direction[1] = directions[LEFT][1];
        white_piece = move[1] == 'w' ? 1 : 0;
        move++;
    } else if (move[0] == '/')
    {
        direction[0] = directions[LEFT_DOWN][0];
        direction[1] = directions[LEFT_DOWN][1];
        white_piece = move[1] == 'w' ? 1 : 0;
        move++;
    } else if (move[0] == '\\')
    {
        direction[0] = directions[LEFT_UP][0];
        direction[1] = directions[LEFT_UP][1];
        white_piece = move[1] == 'w' ? 1 : 0;
        move++;
    } else if (move[0] == 'w')
        white_piece = 1;
    else
        white_piece = 0;
    move++;

    if (direction[0] == 0 && direction[1] == 0)
    {
        char last_char = move[strlen(move) - 1];
        if (last_char == '-')
        {
            direction[0] = directions[1][0];
            direction[1] = directions[1][1];
        } else if (last_char == '\\')
        {
            direction[0] = directions[2][0];
            direction[1] = directions[2][1];
        } else
        {
            direction[0] = directions[RIGHT_UP][0];
            direction[1] = directions[RIGHT_UP][1];
        }
        move[strlen(move) - 1] = '\0';
    }
    other_piece = getPiece(move, white_piece);
    if (other_piece == NULLPIECE)
        return EXIT_FAILURE;
    if (id_to_pos[piece].x != -1)
    {
        pieces[
            MtA(id_to_pos[piece].z, id_to_pos[piece].y, id_to_pos[piece].x)
        ] = NULLPIECE;
    }
    int8_t z, y, x;
    z = id_to_pos[other_piece].z + (direction[0] == 0 && direction[1] == 0) ? 1 : 0;
    y = id_to_pos[other_piece].y + direction[1];
    x = id_to_pos[other_piece].x + direction[0];
    id_to_pos[piece].z = z;
    id_to_pos[piece].y = y;
    id_to_pos[piece].x = x;
    // printf("DEBUG: Placed piece %d at x: %d, y: %d, z: %d\n", piece, id_to_pos[piece].x, id_to_pos[piece].y, id_to_pos[piece].z);
    pieces[MtA(z, y, x)] = piece;
    context->lastMovedPiece = piece;

    return EXIT_SUCCESS;
}

void initContext(Context_t* context) {

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

/*
    0: if parsing was succesful
    1: if an error occurred
*/
int convertFromMZinga(char *mzinga_string, Context_t *context)
{
    // Get GameTypeString
    char *token = strtok(mzinga_string, ";");
    if (token == NULL)
    {
        printf("err Initial parsing of GameString failed.\nok\n");
        return 1;
    }

    if (strcmp(token, "Base+M") == 0)
        context->gameType.mosquito = true;
    else if (strcmp(token, "Base+L") == 0)
        context->gameType.ladybug = 1;
    else if (strcmp(token, "Base+P") == 0)
        context->gameType.pillbug = 1;
    else if (strcmp(token, "Base+ML") == 0)
        context->gameType.mosquito = context->gameType.ladybug = 1;
    else if (strcmp(token, "Base+MP") == 0)
        context->gameType.mosquito = context->gameType.pillbug = 1;
    else if (strcmp(token, "Base+LP") == 0)
        context->gameType.ladybug = context->gameType.pillbug = 1;
    else if (strcmp(token, "Base+MLP") == 0)
        context->gameType.mosquito = context->gameType.ladybug = context->gameType.pillbug = 1;
    else
    {
        printf("err Parsing of GameTypeString failed.\nok\n");
        return 1;
    }

    Pieces_t *pieces = context->board;

    // Get GameStateString
    token = strtok(NULL, ";");
    if (strcmp(token, "NotStarted") == 0)
        return 0;
    else if (strcmp(token, "InProgress") == 0)
        context->gameStatus = IN_PROGRESS;
    else if (strcmp(token, "Draw") == 0)
        context->gameStatus = DRAW;
    else if (strcmp(token, "WhiteWins") == 0)
        context->gameStatus = WHITE_WON;
    else if (strcmp(token, "BlackWins") == 0)
        context->gameStatus = BLACK_WON;
    else
    {
        printf("err Parsing of GameStatusString failed.\nok\n");
        return 1;
    }

    // Get TurnString: Black[n] or White[n]
    token = strtok(NULL, ";");
    if (strncmp(token, "Black", 5) == 0)
        context->curColor = BLACK;
    else if (strncmp(token, "White", 5) == 0)
        context->curColor = WHITE;
    else
    {
        printf("err Parsing of TurnString failed.\nok\n");
        return 1;
    }

    token += 6;
    token[strlen(token) - 1] = '\0';
    context->turn = (int) strtol(token, NULL, 10);
    // The raw turn number must be edited
    context->turn *= 2;
    if (context->curColor == WHITE)
        context->turn -= 1;

    char white_piece;
    Pieces_t piece;
    Position_t *id_to_pos = context->idToPos;

    // Il primo pezzo si gestisce fuori dal while
    token = strtok(NULL, ";");
    addMove(context, token);
    if (token[0] == 'b')
        white_piece = 0;
    else
        white_piece = 1;

    token++;
    piece = getPiece(token, white_piece);
    id_to_pos[piece].x = 0;
    id_to_pos[piece].y = 0;
    id_to_pos[piece].z = 0;
    pieces[MtA(0, 0, 0)] = piece;

    while ((token = strtok(NULL, ";")) != NULL)
    {
        addMove(context, token);
    }
    // debugPrint(context);
    return 0;
}

void appendPiece(const Pieces_t pieceMoved, char *move)
{
    if (pieceMoved == B_QUEEN) {
        strcat(move, "Q");
    } else if (pieceMoved == B_SPIDER_1) {
        strcat(move, "S1");
    } else if (pieceMoved == B_SPIDER_2) {
        strcat(move, "S2");
    } else if (pieceMoved == B_GRASSHOPPER_1) {
        strcat(move, "G1");
    } else if (pieceMoved == B_GRASSHOPPER_2) {
        strcat(move, "G2");
    } else if (pieceMoved == B_GRASSHOPPER_3) {
        strcat(move, "G3");
    } else if (pieceMoved == B_ANT_1) {
        strcat(move, "A1");
    } else if (pieceMoved == B_ANT_2) {
        strcat(move, "A2");
    } else if (pieceMoved == B_ANT_3) {
        strcat(move, "A3");
    } else if (pieceMoved == B_BEETLE_1) {
        strcat(move, "B1");
    } else if (pieceMoved == B_BEETLE_2) {
        strcat(move, "B2");
    } else if (pieceMoved == B_LADYBUG) {
        strcat(move, "L");
    } else if (pieceMoved == B_MOSQUITO) {
        strcat(move, "M");
    } else if (pieceMoved == B_PILLBUG) {
        strcat(move, "P");
    }
}

/*
    From our system, to a MZinga-compliant one;
        il contesto è già MODIFICATO dalla mossa da deconvertire.
*/
char *deconvertMove(Context_t *context, Pieces_t pieceMoved)
{
    char *move = calloc(sizeof(char), 50);
    move[0] = pieceMoved <= 13 ? 'b' : 'w';
    pieceMoved = pieceMoved % 14;

    appendPiece(pieceMoved, move);
    move = strcat(move, " ");

    int8_t index = -1;
    char x = context->idToPos[pieceMoved].x;
    char y = context->idToPos[pieceMoved].y;
    char z = context->idToPos[pieceMoved].z;
    Pieces_t *board = context->board;
    for (int8_t i = 0; i < 6; i++)
    {
        if (board[MtA(z, y + directions[i][0], x + directions[i][1])] != NULLPIECE)
        {
            index = i;
            break;
        }
    }
    // If not found, then the piece was put on top of another one
    if (index == -1)
    {
        appendPiece(board[MtA(z - 1, y, x)], move);
    } else
    {
        if (index == RIGHT)
        {
            appendPiece(board[MtA(z, y + directions[index][0], x + directions[index][1])], move);
            move = strcat(move, "-");
        } else if (index == RIGHT_UP)
        {
            appendPiece(board[MtA(z, y + directions[index][0], x + directions[index][1])], move);
            move = strcat(move, "/");
        } else if (index == RIGHT_DOWN)
        {
            appendPiece(board[MtA(z, y + directions[index][0], x + directions[index][1])], move);
            move = strcat(move, "\\");
        } else if (index == LEFT)
        {
            move = strcat(move, "-");
            appendPiece(board[MtA(z, y + directions[index][0], x + directions[index][1])], move);
        } else if (index == LEFT_UP)
        {
            move = strcat(move, "\\");
            appendPiece(board[MtA(z, y + directions[index][0], x + directions[index][1])], move);
        } else if (index == LEFT_DOWN)
        {
            move = strcat(move, "/");
            appendPiece(board[MtA(z, y + directions[index][0], x + directions[index][1])], move);
        }
    }
    
    move = realloc(move, strlen(move) + 1);
    return move;
}

void debugPrint(Context_t *context)
{
    printf("--- DEBUG PRINT ---\n");
    for (int z = 0; z < 5; ++z)
    {
        for (int y = -14; y < 14; ++y)
        {
            for (int x = -28; x < 28; ++x)
            {
                Pieces_t piece = context->board[MtA(z, y, x)];
                if (piece != NULLPIECE)
                {
                    printf("Piece: %d at x: %d, y: %d, z: %d\n", piece, x + 28, y + 14, z);
                }
            }
        }
    }
    printf("--- END DEBUG PRINT ---\n");
}
