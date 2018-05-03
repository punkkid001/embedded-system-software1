#include <stdio.h>
#include <sys/fcntl.h>

#include "ku_pir.h"

int main(void)
{
    printf("Insert data status: %d\n", ku_pir_insertData(1, 3001000, 1));
    return 0;
}
