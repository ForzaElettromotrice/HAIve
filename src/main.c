//
// Created by f3m on 28/03/25.
//

#include "main.h"

#include <assert.h>
#include <enums.h>

#include "xxhash.h"

// Assumiamo che queste condizioni siano sempre vere altrimenti si sfancula tutto
static_assert(sizeof(Pieces_t) == 1);
static_assert(sizeof(Piece_t) == 4);


int main()
{
#ifdef Debug
    D_Print("Launched in Debug Mode!\n");

#endif

    printf("Hello, World!\n");
    printf("%llu\n", XXH3_64bits("ciao", 5));

    return 0;
}
