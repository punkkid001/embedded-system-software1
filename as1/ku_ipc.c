#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#include <asm/delay.h>
#include <asm/uaccess.h>

#include "ku_ipc.h"

MODULE_LICENSE("GPL");

typedef struct queue
{
    struct list_head list;
    MSGBUF buf;
    int key;
} QUEUE;

MSGBUF msg_buf;
QUEUE msg_q[MAXVOL];

static struct cdev *cd_cdev;
spinlock_t ku_lock;
dev_t dev_num;

void delay(int sec)
{
    int i, j;
    for(i=0; i<sec; i++)
        for(j=0; j<1000; j++)
            udelay(1000);
}

static int ku_ipc_read(MSGBUF *msg)
{
    int size;

    delay(1);
    spin_lock(&ku_lock);
    size = copy_to_user(msg, msg_buf, sizeof(MSGBUF));
    memset(msg_buf, '\0', sizeof(MSGBUF));
    spin_unlock(&ku_lock);

    return size;
}

static int ku_ipc_write(MSGBUF *msg)
{
    int size;

    spin_lock(&ku_lock);
    size = copy_from_user(msg_buf, msg, sizeof(MSGBUF));
    spin_unlock(&ku_lock);

    return size;
}

static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    MSGBUF *user_buf;
    int size;

    user_buf = (MSGBUF*)arg;
    switch(cmd)
    {
        case KU_READ:
            size = ku_ipc_read(user_buf);
            printk("[KU_IPC]read - %d\n", size);
            break;
        case KU_WRITE:
            size = ku_ipc_write(user_buf);
            printk("[KU_IPC]write - %d\n", size);
            break;
    }

    return 0;
}

static int ku_ipc_open(struct inode *inode, struct file *file)
{
    printk("[KU_IPC]Open\n");
    return 0;
}

static int ku_ipc_release(struct inode *inode, struct file *file)
{
    printk("[KU_IPC]Release\n");
    return 0;
}

struct file_operations ku_ipc_fops = {
    .unlocked_ioctl = ku_ipc_ioctl,
    .open = ku_ipc_open,
    .release = ku_ipc_release,
    .read = ku_ipc_read,
    .write = ku_ipc_write
};

static int __init ku_ipc_init(void)
{
    printk("[KU_IPC]Init\n");
    INIT_LIST_HEAD(&msg_q.list);

    cd_cdev = cdev_alloc();
    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cdev_init(cd_cdev, &ku_ipc_fops);
    cdev_add(cd_cdev, dev_num, 1);
    
    return 0;
}

static void __exit ku_ipc_exit(void)
{
    printk("[KU_IPC]Exit\n");
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
