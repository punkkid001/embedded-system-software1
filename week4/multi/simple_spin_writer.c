#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "simple_spin.h"

int main(void)
{
    int dev;
    struct str_st user_str = {
        "Reader: Hi, writer!", 0
    };

    user_str.len = strlen(user_str.str);

    printf("Writer: Hello Reader!\n");

    dev = open("/dev/simple_spin_dev", O_RDWR);
    ioctl(dev, SIMPLE_SPIN_WRITE, &user_str);

    close(dev);
    return 0;
}
