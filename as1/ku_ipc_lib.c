#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_ipc.h"

int ku_msgget(int key, int msgflg);
int ku_msgclose(int msqid);
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg);
int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg);

int ku_msgget(int key, int msgflg)
{
    int dev, msqid;
    
    dev= open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev < 0)
        return -1;
    msqid = ioctl(dev, KU_CHECK, key);

    if(msqid == -1)
    {
        switch(msgflg)
        {
            case IPC_CREAT:
                msqid = ioctl(dev, KU_CHECK, key);
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
    int dev, status;

    dev = open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev < 0)
        return -1;

    status = ioctl(dev, KU_CLOSE, msqid);
    return status;
}

// ku_ipc_write
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg)
{
    SNDMSG *snd_msg;
    int dev, status;
    
    dev = open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev < 0)
        return -1;

    snd_msg = malloc(sizeof(SNDMSG));
    snd_msg->id = msqid;
    snd_msg->size = msgsz;
    snd_msg->data = msgp;
    snd_msg->type = ((struct msgbuf*)msgp)->type;

    status = write(dev, snd_msg, sizeof(snd_msg));

    if(status < 0)
    {
        if(status == -3 && (msgflg & IPC_NOWAIT) != 0)
            return -1;
        else if(status == -3 && (msgflg & IPC_NOWAIT) == 0)
            while(!(status == 0))
                status = write(dev, msgp, msgsz);
        else
            return -1;
    }

    return 0;
}

// ku_ipc_read
int ku_msgrcv(int msgid, void *msgp, int msgsz, long msgtyp, int msgflg)
{
    RCVMSG *rcv_msg = NULL;
    int dev, status;

    dev = open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev < 0)
        return -1;

    status = ioctl(dev, KU_EMPTY, msgid);
    if(status == 0 && (msgflg & IPC_NOWAIT) != 0)
        return -1;
    else
        while(!(status == -1))
            status = ioctl(dev, KU_EMPTY, msgid);

    rcv_msg->type = msgtyp;
    rcv_msg->id = msgid;
    rcv_msg->size = msgsz;
    rcv_msg->flag = msgflg;
    rcv_msg->data = msgp;

    status = read(dev, rcv_msg, sizeof(rcv_msg));

    // oversize (sizeof(msgq.data) > msgsz)
    if(status == -2)
    {
        if(msgflg & MSG_NOERROR == 0)
            return -1;
        else
        {
            rcv_msg->flag = 0;
            status = read(dev, rcv_msg, sizeof(rcv_msg));
        }
    }

    return status;
}
