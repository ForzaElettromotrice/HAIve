//
// Created by f3m on 28/03/25.
//

#include "main.h"

#include <assert.h>
#include <enums.h>
#include <stdlib.h>
#include <string.h>

#include "xxhash.h"

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

void print_gamestring(const GameType game_type, const Colors_t white_turn, const int turn, char **moves, const enum GameStatus game_status)
{
    // GameType
    printf("Base");
    if (game_type.ladybug || game_type.pillbug || game_type.mosquito)
    {
        //FIXME: ma se tanto basta che sia presente uno per attivarli tutti, perché 3 booleani? Ne basta 1
        printf("+");
        if (game_type.mosquito) printf("M");
        if (game_type.ladybug) printf("L");
        if (game_type.pillbug) printf("P");
    }
    printf(";");

    // GameStatus
    switch (game_status)
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
    switch (white_turn)
    {
        case WHITE: printf("White[%d]", (turn / 2) + 1);
            break;
        case BLACK: printf("Black[%d]", turn / 2);
            break;
        case NULLCOLOR: return;
    }
    if (moves == NULL) {
        printf("\nok\n");
        return;
    }
    printf(";");

    // Moves
    for (int i = 0; moves[i] != NULL; i++)
    {
        printf("%s", moves[i]);
        if (moves[i + 1] != NULL)
        {
            printf(";");
        }
    }
    printf("ok\n");
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
        printf("Unknown command: %s", command);
    }

    printf("ok\n");
    return 0;
}

int main(void)
{
#ifdef Debug
    D_Print("Launched in Debug Mode!\n");
#endif

    size_t buf_size = 128;
    char *buffer = malloc(sizeof(char) * buf_size);

    bool white_turn = true;
    GameType game_type = {false, false, false};
    enum GameStatus game_status = NOT_STARTED;
    int turn = 1;
    char **moves = NULL;

    while (true)
    {
        const size_t read = getline(&buffer, &buf_size, stdin);
        if (read == -1) // EOF
            break;
        buffer[read] = '\0';
        if (read == 1) continue; // Empty line

        const int result = manage_command(buffer);
        if (result == -1) break;
        if (result <= 0) continue;
        if (result == 1)
        {
            // simply command 'newgame'
            // TODO: initialize empty board
            game_type = (GameType){false, false, false};
            white_turn = true;
            turn = 1;
            game_status = NOT_STARTED;
            // FIXME: mi spieghi sta roba a che serve? La condizione è sempre falsa
            if (moves != NULL)
            {
                for (int i = 0; moves[i] != NULL; i++)
                {
                    free(moves[i]);
                }
                free(moves);
            }
            moves = NULL;
            print_gamestring(game_type, white_turn, turn, moves, game_status);
        }
    }
    free(buffer);
    return 0;
}
