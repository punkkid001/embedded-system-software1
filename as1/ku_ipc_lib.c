#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ku_ipc.h"

struct msgbuf
{
    long mtype;
    char *mtext;
};

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
    close(dev);
    return status;
}

// ku_ipc_write
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg)
{
    SNDMSG snd;
    int dev, status;
    
    dev = open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev < 0)
        return -1;

    snd.id = msqid;
    snd.size = msgsz;
    snd.data = msgp;
    snd.type = ((struct msgbuf*)msgp)->mtype;

    status = write(dev, &snd, sizeof(snd));

    if(status < 0)
    {
        if(status == -3 && (msgflg & IPC_NOWAIT) != 0)
        {
            close(dev);
            return -1;
        }
        else if(status == -3 && (msgflg & IPC_NOWAIT) == 0)
            while(status != 0)
                status = write(dev, &snd, sizeof(snd));
        else
        {
            close(dev);
            return -1;
        }
    }

    close(dev);
    return 0;
}

// ku_ipc_read
int ku_msgrcv(int msgid, void *msgp, int msgsz, long msgtyp, int msgflg)
{
    RCVMSG rcv;
    int dev, status;

    dev = open("/dev/ku_ipc_dev", O_RDWR);
    // dev open error
    if(dev < 0)
        return -1;

    status = ioctl(dev, KU_EMPTY, msgid);
    if(status == 0 && (msgflg & IPC_NOWAIT) != 0)
    {
        close(dev);
        return -1;
    }
    else
        while(!(status == -1))
            status = ioctl(dev, KU_EMPTY, msgid);

    rcv.type = msgtyp;
    rcv.id = msgid;
    rcv.size = msgsz;
    rcv.flag = msgflg;
    rcv.data = msgp;

    status = read(dev, &rcv, sizeof(rcv));

    close(dev);
    return status;
}
