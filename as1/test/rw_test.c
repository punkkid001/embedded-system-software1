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
    SNDMSG snd;
    RCVMSG rcv;
    struct msgbuf msg1, msg2, fin, fin2;
    int key = 8000, dev, result;
    
    dev = open("/dev/ku_ipc_dev", O_RDWR);

    result = ioctl(dev, KU_CHECK, key);
    printf("KU_CHECK: %d\n", result);

    result = ioctl(dev, KU_CREAT, key);

    // test
    msg1.mtype = 1;
    memset(msg1.mtext, '\0', 64);
    strcpy(msg1.mtext, "first");
    msg1.num = 10;

    msg2.mtype = 2;
    memset(msg2.mtext, '\0', 64);
    strcpy(msg2.mtext, "second");
    msg2.num = 20;

    // write
    snd.type = msg1.mtype;
    snd.id = key;
    snd.size = sizeof(msg1);
    snd.data = &msg1;
    result = write(dev, (char*)&snd, sizeof(snd));
    printf("write: %d\n", result);

    snd.type = msg2.mtype;
    snd.id = key;
    snd.size = sizeof(msg2);
    snd.data = &msg2;
    result = write(dev, (char*)&snd, sizeof(snd));
    printf("write2: %d\n", result);

    // read
    rcv.type = 0;
    rcv.id = key;
    rcv.size = sizeof(struct msgbuf);
    rcv.flag = 0;
    rcv.data = &fin;

    result = read(dev, (char*)&rcv, sizeof(rcv));
    printf("read: result %d\n", result);
    printf("read: type %d\n", rcv.type);
    printf("read: data - %s\n", ((struct msgbuf*)rcv.data)->mtext);

    rcv.type = 0;
    rcv.id = key;
    rcv.size = sizeof(struct msgbuf);
    rcv.flag = 0;
    rcv.data = &fin2;

    result = read(dev, (char*)&rcv, sizeof(rcv));
    printf("read2: result %d\n", result);
    printf("read2: type %d\n", rcv.type);
    printf("read2: data - %s\n", ((struct msgbuf*)rcv.data)->mtext);
    
    result = ioctl(dev, KU_CLOSE, key);

    close(dev);
    return 0;
}
