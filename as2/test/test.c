#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_pir.h"

int main(void)
{
    struct ku_pir_data data;
    int dev = open("/dev/ku_pir_dev", O_RDWR);
    int fd = ku_pir_open();
    int result;

    printf("fd: %d\n", fd);

    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3001000, 1));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3002000, 0));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3003000, 1));

    ku_pir_flush(fd);

    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3001000, 1));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3002000, 0));
    printf("Insert data status: %d\n", ku_pir_insertData(fd, 3003000, 1));

    ku_pir_read(fd, data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);
    ku_pir_read(fd, &data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);
    ku_pir_read(fd, &data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);

    /*
    ku_pir_read(fd, &data);
    printf("[FINAL READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);
    */

    ku_pir_close(fd);
    return 0;
}
