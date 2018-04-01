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
    MSGBUF *msg;
    int dev, status;
    
    dev = open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev < 0)
        return -1;

    msg->type = 0;    // is it okay?
    msg->id = msqid;
    msg->size = msgsz;
    msg->flag = msgflg;
    msg->data = msgp;

    // status = write(dev, (char*)msg, sizeof(msg));
    status = write(dev, msg, sizeof(msg));

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
    MSGBUF *msg = NULL;
    void *result = NULL;
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

    result = malloc(msgsz);

    msg->type = msgtyp;
    msg->id = msgid;
    msg->size = msgsz;
    msg->flag = msgflg;
    msg->data = result;
    // msg->data = msgp;

    // status = read(dev, (char*)msg, sizeof(msg));
    status = read(dev, msg, sizeof(msg));

    // oversize (sizeof(msgq.data) > msgsz)
    if(status == -2)
    {
        if(msgflg & IPC_NOERROR == 0)
            return -1;
        else
        {
            msg->flag = 0;
            status = read(dev, msg, sizeof(msg));
        }
    }

    return status;
}
