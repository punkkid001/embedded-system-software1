#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/ioctl.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_READ IOCTL_START_NUM+1
#define IOCTL_WRITE IOCTL_START_NUM+2

#define CH1_IOCTL_NUM 'z'
#define CH1_IOCTL_READ _IOR(CH1_IOCTL_NUM, IOCTL_READ, unsigned long *)
#define CH1_IOCTL_WRITE _IOW(CH1_IOCTL_NUM, IOCTL_WRITE, unsigned long *)

int main(void)
{
    int dev_mod = open("/dev/", O_RDWR);    // Only write
    int dev_mod2 = open("/dev/", O_RDWR);    // Only read

    unsigned long val = 0;

    ioctl(dev_mod2, CH1_IOCTL_READ, &val);    // Print global_value
    ioctl(dev_mod, CH1_IOCTL_WRITE, &val);    // Increase global_value
    ioctl(dev_mod2, CH1_IOCTL_READ, &val);

    close(dev_mod);
    close(dev_mod2);
    return 0;
}
