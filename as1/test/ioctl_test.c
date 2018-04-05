#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include "ku_ipc.h"

struct msgbuf
{
    long mtype;
    char mtext[64];
    int num;
};

int main(void)
{
    int key = 8000, dev, result;
    
    dev = open("/dev/ku_ipc_dev", O_RDWR);

    result = ioctl(dev, KU_CHECK, key);
    printf("KU_CHECK: %d\n", result);

    result = ioctl(dev, KU_CREAT, key);
    result = ioctl(dev, KU_CLOSE, key);

    close(dev);
    return 0;
}
