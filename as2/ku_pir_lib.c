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
    int dev, fd;
    
    dev = open("/dev/ku_pir_dev", O_RDWR);
    if(dev < 0)
        return -1;

    fd = ioctl(dev, KU_OPEN, 0);

    close(dev);
    return fd;
}

int ku_pir_close(int fd)
{
    int dev;

    dev = open("/dev/ku_pir_dev", O_RDWR);
    if(dev < 0)
        return -1;

    ioctl(dev, KU_CLOSE, fd);

    close(dev);
    return 0;
}

void ku_pir_read(int fd, struct ku_pir_data *data)
{
    int dev;
    struct ku_pir_capsule capsule;

    dev = open("/dev/ku_pir_dev", O_RDWR);
    if(dev < 0)
        return;

    capsule.fd = fd;
    capsule.data = data;

    // using blocking
    read(dev, &capsule, sizeof(capsule));

    close(dev);
}


void ku_pir_flush(int fd)
{
    int dev;
    
    dev = open("/dev/ku_pir_dev", O_RDWR);
    if(dev < 0)
        return;

    ioctl(dev, KU_FLUSH, fd);

    close(dev);
}

int ku_pir_insertData(int fd, long unsigned int ts, char rf_flag)
{
    int dev, status;
    struct ku_pir_data data;
    struct ku_pir_capsule capsule;

    data.timestamp = ts;
    data.rf_flag = rf_flag;
    capsule.fd = fd;
    capsule.data = &data;

    dev = open("/dev/ku_pir_dev", O_RDWR);
    if(dev < 0)
        return -1;

    status = write(dev, (char*)&capsule, sizeof(capsule));

    close(dev);
    return status;
}
