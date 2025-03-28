//
// Created by f3m on 28/03/25.
//

#include "main.h"

int main()
{
#ifdef Debug
    D_Print("Launched in Debug Mode!\n");
#endif

    printf("Hello, World!\n");

    return 0;
}
