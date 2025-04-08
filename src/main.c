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

void print_info(void){
    printf("id hAIve v1.0\n");
    printf("Mosquito;Ladybug;Pillbug\n");
}

void print_gamestring(GameType game_type, bool white_turn, int turn, char** moves, enum GameStatus game_status){
    // GameType
    printf("Base");
    if (game_type.ladybug || game_type.pillbug || game_type.mosquito) {
        printf("+");
        if (game_type.mosquito) printf("M");
        if (game_type.ladybug) printf("L");
        if (game_type.pillbug) printf("P");
    } printf(";");

    // GameStatus
    switch (game_status) {
        case WHITE_WON: printf("WhiteWins;"); break;
        case BLACK_WON: printf("BlackWins;"); break;
        case DRAW:      printf("Draw;"); break;
        case IN_PROGRESS: printf("InProgress;"); break;
        case NOT_STARTED: printf("NotStarted;"); break;
    }

    // TurnString
    if (white_turn) {
        printf("White[%d]", (turn / 2) + 1);
    } else {
        printf("Black[%d]", turn / 2);
    }
    if (moves == NULL) return;
    printf(";");

    // Moves
    for (int i = 0; moves[i] != NULL; i++) {
        printf("%s", moves[i]);
        if (moves[i + 1] != NULL) {
            printf(";");
        }
    }

}

// -1: exit
// 0: nothing to do
// 1: newgame
// 2: play move
// 3: bestmove
int manage_command(char* command){
    if (command == NULL) {
        return -1;
    }

    if (strcmp(command, "exit") == 0) {
        return -1;
    } else if (strcmp(command, "info") == 0) {
        print_info();
    } else if (strcmp(command, "options") == 0) { 
        //Doesn't print anything
    } else if (strcmp(command, "newgame") == 0) {
        // TODO: DO NEWGAME
        return 1;
    } else if (strcmp(command, "play") == 0){
        // TODO: Play move
        return 2;
    } else if (strcmp(command, "pass") == 0){
        // TODO: Play pass
    } else if (strcmp(command, "validmoves") == 0){
        // TODO: Print valid moves
    } else if (strcmp(command, "bestmove") == 0){
        return 3;
    }
    else {
        printf("err Unknown command: %s\n", command);
    }

    printf("ok\n");
    return 0;
}

int main(void)
{
#ifdef Debug
    D_Print("Launched in Debug Mode!\n");

#endif

    /*
        printf("Hello, World!\n");
        printf("%llu\n", XXH3_64bits("ciao", 5));
    */

    uint32_t buf_size = 100, ch, read;
    char* buffer = malloc(sizeof(char) * buf_size);

    bool white_turn = true;
    GameType game_type = {false, false, false};
    enum GameStatus game_status = NOT_STARTED;
    int turn = 1;
    char** moves = NULL;

    while(true){
        read = 0;
        while((ch = getchar()) != '\n' && ch != EOF && ch != ' ') {
            if (read + 1 > buf_size) {
                buf_size *= 2;
                buffer = realloc(buffer, sizeof(char) * buf_size);
            } buffer[read++] = (char) ch;
        }
        buffer[read] = '\0';
        if (read == 0) continue; // Empty line

        int result = manage_command(buffer);
        if (result == -1)   break;
        if (result <= 0)    continue;
        if (result == 1){
            if(ch == '\n'){ // simply command 'newgame'
                // TODO: initialize empty board
                game_type = (GameType){false, false, false};
                white_turn = true; turn = 1;
                game_status = NOT_STARTED;
                if(moves != NULL){
                    for(int i = 0; moves[i] != NULL; i++){
                        free(moves[i]);
                    } free(moves);
                } moves = NULL;
                print_gamestring(game_type, white_turn, turn, moves, game_status);
            }
        }
    }

    return 0;
}
