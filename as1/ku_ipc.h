#include <linux/list.h>

#define KUIPC_MAXMSG 1024
#define KUIPC_MAXVOL 1024
#define IPC_CREAT 100
#define IPC_EXCL 200
#define IPC_NOWAIT 300
#define MSG_NOERROR 400

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOTCL_START_NUM+2

#define KU_IOCTL_NUM 'z'
#define KU_READ _IOWR(KU_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_WRITE _IOWR(KU_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

#define DEV_NAME "ku_ipc_dev"

typedef struct msgbuf
{
    long type;
    char text[KUIPC_MAXMSG];
} MSGBUF;

typedef struct rcvmsg
{
    struct list_head list;
    long type;
    int size;
    struct msgbuf msg;
} RCVMSG;
