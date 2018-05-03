#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_pir.h"

int main(void)
{
    struct ku_pir_data data;
    int fd = ku_pir_open();

    printf("fd: %d\n", fd);

    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3001000, 1));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3002000, 0));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3003000, 1));

    ku_pir_print();

    ku_pir_flush(fd);

    ku_pir_print();

    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3001000, 1));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3002000, 0));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3003000, 1));

    ku_pir_print();

    data.timestamp = 0;
    data.rf_flag = 0;

    ku_pir_read(fd, data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);
    ku_pir_read(fd, &data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);
    ku_pir_read(fd, &data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);

    ku_pir_print();

    // Blocking test
    ku_pir_read(fd, &data);
    printf("[FINAL READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);

    ku_pir_close(fd);
    return 0;
}
