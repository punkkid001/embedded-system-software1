#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_ipc.h"

#define FALSE 0
#define TRUE 1

int ku_msgget(int key, int msgflg);
int ku_msgclose(int msqid);
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg);
int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg);

int ku_msgget(int key, int msgflg)
{
    int dev, msqid;
    
    dev= open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev == -1)
        return -1;
    msqid = ioctl(dev, KU_CHECK, key);

    if(msqid == -1)
    {
        switch(msgflg)
        {
            case IPC_CREAT:
                msqid = ioctl(dev, KU_GET, key);
                close(dev);
                return msqid;
            case IPC_EXCL:
                close(dev);
                return -1;
        }

        return -1;
    }

    else
    {
        msqid = ioctl(dev, KU_CREAT, key);
        close(dev);
        return msqid;
    }
}

int ku_msgclose(int msqid)
{
    int dev, result;

    dev = open("/dev/ku_ipc_dev", O_RDWR);
    //dev open error
    if(dev == -1)
        return -1;

    result = ioctl(dev, KU_CLOSE, msqid);
    if(result == 0)
        return 0;
    else
        return -1;
}

int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg)
{
    int dev = open("/dev/ku_ipc_dev", O_RDWR);
    write(dev, msgp, msgsz);
    return 0;
}

int ku_msgrcv(int msgid, void *msgp, int msgsz, long msgtyp, int msgflg)
{
    int dev = open("/dev/ku_ipc_dev", O_RDWR);
    read(dev, msgp, msgsz);
    return 0;
}
