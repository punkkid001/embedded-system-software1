#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_pir.h"

int ku_pir_open();
int ku_pir_close(int fd);
void ku_pir_read(int fd, struct ku_pir_data *data);
void ku_pir_flush(int fd);
int ku_pir_insertData(int fd, long unsigned int ts, char rf_flag);

int ku_pir_open()
{
    int fd;
    
    fd = open("/dev/ku_pir_dev", O_RDWR);
    if(fd < 0)
        return -1;

    return fd;
}

int ku_pir_close(int fd)
{
    close(fd);
    if(fd < 0)
        return -1;
    return 0;
}

void ku_pir_read(int fd, struct ku_pir_data *data)
{
    // using blocking
    read(fd, data, sizeof(data));
}


void ku_pir_flush(int fd)
{
    ioctl(fd, KU_FLUSH, 0);
}

int ku_pir_insertData(int fd, long unsigned int ts, char rf_flag)
{
    int status;
    struct ku_pir_data data;
    data.timestamp = ts;
    data.rf_flag = rf_flag;

    status = write(fd, &data, sizeof(data));
    return status;
}
