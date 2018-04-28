#define KUIPC_MAXMSG 1024
#define KUIPC_MAXVOL 8192

#define KUPIR_MAX_MSG 1024
#define KUPIR_SENSOR 17

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define KU_IOCTL_NUM 'z'
#define KU_CREAT _IOWR(KU_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_CLOSE _IOWR(KU_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define KU_FLUSH _IOWR(KU_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define KU_EMPTY _IOWR(KU_IOCTL_NUM, IOCTL_NUM4, unsigned long *)

#define DEV_NAME "ku_pir_dev"

struct ku_pir_data
{
    long unsigned int timestamp;
    char rf_flag;
};
