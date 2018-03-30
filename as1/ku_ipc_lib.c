#include <unistd.h>
#include <fcntl.h>

#include "ku_ipc.h"

#define FALSE 0
#define TRUE 1

int ku_msgget(int key, int msgflg);
int ku_msgclose(int msqid);
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg);
int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg);
int ku_msgchk(int msqid);

int ku_msgget(int key, int msgflg)
{
    switch(msgflg)
    {
        case IPC_CREAT:
            return 0;
        case IPC_EXCL:
            return -1;
    }

    return -1;
}

int ku_msgclose(int msqid)
{
    return 0;
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

int ku_msqchk(int msqid)
{

    return TRUE;
}
