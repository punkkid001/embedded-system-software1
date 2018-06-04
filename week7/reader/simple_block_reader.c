#include <stdio.h>
#include <sys/fcntl.h>

int main(void)
{
    int fd, ret;

    fd = open("/dev/simple_block_dev", O_RDWR);
    ret = read(fd, NULL, 0);
    printf("success to consume: %d\n", ret);

    close(fd);
    return 0;
}
