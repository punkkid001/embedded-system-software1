#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include "ku_ipc.h"

struct msgbuf
{
    long mtype;
    char mtext[64];
    int num;
};

int main(void)
{
    struct msgbuf msg1, msg2, msg3, msg4, msg5;
    struct msgbuf fin;

    int key = 3253, result;
    
    // first create
    result = ku_msgget(key, IPC_CREAT);
    printf("[msgget] result: %d - success(id)\n", result);
    // duplicate key test - return no error
    result = ku_msgget(key, IPC_CREAT);
    printf("[msgget] result: %d - noerror(id)\n", result);
    // duplicate key test - return error
    result = ku_msgget(key, IPC_EXCL);
    printf("[msgget] result: %d - error(-1)\n", result);

    // test
    msg1.mtype = 1;
    memset(msg1.mtext, '\0', 64);
    strcpy(msg1.mtext, "first");
    msg1.num = 10;

    msg2.mtype = 2;
    memset(msg2.mtext, '\0', 64);
    strcpy(msg2.mtext, "second");
    msg2.num = 20;

    msg3.mtype = 3;
    memset(msg3.mtext, '\0', 64);
    strcpy(msg3.mtext, "third");
    msg3.num = 30;

    msg4.mtype = 4;
    memset(msg4.mtext, '\0', 64);
    strcpy(msg4.mtext, "fourth");
    msg4.num = 40;

    msg5.mtype = 5;
    memset(msg5.mtext, '\0', 64);
    strcpy(msg5.mtext, "fifth");
    msg5.num = 50;

    // write
    result = ku_msgsnd(key, &msg5, sizeof(msg5.mtext), 0);
    printf("[msgsnd] result: %d\n", result);

    result = ku_msgsnd(key, &msg4, sizeof(msg4.mtext), 0);
    printf("[msgsnd] result: %d\n", result);

    result = ku_msgsnd(key, &msg3, sizeof(msg3.mtext), 0);
    printf("[msgsnd] result: %d\n", result);

    result = ku_msgsnd(key, &msg2, sizeof(msg2.mtext), 0);
    printf("[msgsnd] result: %d\n", result);
    
    result = ku_msgsnd(key, &msg1, sizeof(msg1.mtext), 0);
    printf("[msgsnd] result: %d\n", result);

    // read
    memset(fin.mtext, '\0', 64);
    result = ku_msgrcv(key, &fin, sizeof(fin.mtext), 0, 0);
    printf("[msgrcv] result: %d / type: %d\n", result, fin.mtype);
    printf("[msgrcv] data: %s\n", fin.mtext);

    memset(fin.mtext, '\0', 64);
    result = ku_msgrcv(key, &fin, sizeof(fin.mtext), 0, 0);
    printf("[msgrcv] result: %d / type: %d\n", result, fin.mtype);
    printf("[msgrcv] data: %s\n", fin.mtext);

    memset(fin.mtext, '\0', 64);
    result = ku_msgrcv(key, &fin, sizeof(fin), -5, MSG_NOERROR);
    printf("[msgrcv] result: %d / type: %d\n", result, fin.mtype);
    printf("[msgrcv] data: %s\n", fin.mtext);

    memset(fin.mtext, '\0', 64);
    result = ku_msgrcv(key, &fin, sizeof(fin), 0, MSG_NOERROR);
    printf("[msgrcv] result: %d / type: %d\n", result, fin.mtype);

    // close
    ku_msgclose(key);
    return 0;
}
