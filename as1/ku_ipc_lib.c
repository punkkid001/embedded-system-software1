#include <stdio.h>

#define IPC_CREAT
#define IPC_EXCL

struct msgbuf
{
    long type;
    char text[1];
};

int ku_msgget(int key, int msgflg);
int ku_msgclose(int msqid);
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg);
int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg);

int main(void)
{
    return 0;
}

int ku_msgget(int key, int msgflg)
{
    if(msgflg == IPC_CREAT)
        return 0;
    else if(msgflg == IPC_EXCL)
        return -1;
    return -1;
}

int ku_msgclose(int msqid)
{
    return 0;
}

int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg)
{
    return 0;
}

int ku_msgrcv(int msgid, void *msgp, int msgsz, long msgtyp, int msgflg)
{
    return 0;
}
