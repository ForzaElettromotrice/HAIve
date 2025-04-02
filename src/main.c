//
// Created by f3m on 28/03/25.
//

#include "main.h"
#include "xxhash.h"

int main()
{
#ifdef Debug
    D_Print("Launched in Debug Mode!\n");
#endif

    printf("Hello, World!\n");
    printf("%ld\n", XXH3_64bits("ciao", 5));

    return 0;
}
