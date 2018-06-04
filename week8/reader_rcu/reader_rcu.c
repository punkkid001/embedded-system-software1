#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_READ _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_READ_RCU _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

int main(void)
{
    int dev;
    dev = open("/dev/simple_rcu_dev", O_RDWR);
    ioctl(dev, IOCTL_READ_RCU, 0);

    close(dev);
    return 0;
}
