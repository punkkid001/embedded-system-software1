#include <linux/list.h>

#define KUIPC_MAXMSG 1024
#define KUIPC_MAXVOL 1024
#define IPC_CREAT 100
#define IPC_EXCL 200
#define IPC_NOWAIT 300
#define MSG_NOERROR 400

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3

#define KU_IOCTL_NUM 'z'
#define KU_CREAT _IOWR(KU_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_CLOSE _IOWR(KU_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define KU_CHECK _IOWR(KU_IOCTL_NUM, IOCTL_NUM3, unsigned long *)

#define DEV_NAME "ku_ipc_dev"

typedef struct msgbuf
{
    struct list_head list;
    long type;
    int id;
    int size;
    void *data;
} MSGBUF;

/*
 * <RETURN VALUE>
 * 0: success
 * -1: fail
*/
