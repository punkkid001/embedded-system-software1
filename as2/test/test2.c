#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_pir.h"

int main(void)
{
    struct ku_pir_data data;
    int fd = ku_pir_open();

    printf("fd: %d\n", fd);

    ku_pir_read(fd, data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);
    ku_pir_read(fd, &data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);
    ku_pir_read(fd, &data);
    printf("[READ] ts: %d / flag: %d\n", data.timestamp, data.rf_flag);

    ku_pir_print();

    ku_pir_close(fd);
    return 0;
}
