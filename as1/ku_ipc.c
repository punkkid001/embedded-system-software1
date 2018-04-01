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
    MSGBUF msg_buf;
    int key;
    int size;
} QUEUE;

QUEUE msg_q;

static struct cdev *cd_cdev;
spinlock_t ku_lock;
dev_t dev_num;

int ku_ipc_volume = 0;

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
    QUEUE *temp = NULL;
    MSGBUF *user_buf = NULL;
    struct list_head *pos = NULL, *q = NULL;
    int size = 0;

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

        case KU_CHECK:
            // is queue empty
            if(list_empty(&msg_q.list))
                return -1;

            spin_lock(&ku_lock);
            list_for_each_entry(temp, &msg_q.list, list)
            {
                if(temp->key == (int)arg)
                {
                    spin_unlock(&ku_lock);
                    return 0;
                }
            }
            spin_unlock(&ku_lock);

            // duplicated keys are not allowed
            return -1;

        case KU_CREAT:
            spin_lock(&ku_lock);
            if(ku_ipc_volume <= KUIPC_MAXVOL)
            {
                temp = (QUEUE*)kmalloc(sizeof(QUEUE), GFP_KERNEL);
                temp->key = (int)arg;
                temp->size = 0;

                INIT_LIST_HEAD(&(temp->msg_buf).list);
                (temp->msg_buf).data = kmalloc(KUIPC_MAXMSG);

                list_add_tail(&temp->list, &msg_q.list);
                ku_ipc_volume++;
                spin_unlock(&ku_lock);

                printk("[KU_IPC]create queue - %d\n", (int)arg);
                return arg;
            }

            // queue volume is full
            else
                return -1;

        case KU_CLOSE:
            spin_lock(&ku_lock);

            if(ku_ipc_volume > 0)
            {
                // remove list node (queue)
                list_for_each_safe(pos, q, &msg_q.list)
                {
                    temp = list_entry(pos, QUEUE, list);

                    // success to remove queue
                    if(temp->key == arg)
                    {
                        MSGBUF *msg;
                        struct list_head *ipos, *iq;

                        // remove list node (msg_buf)
                        list_for_each_safe(ipos, iq, &(temp->msg_buf).list)
                        {
                            msg = list_entry(ipos, MSGBUF, list);
                            list_del(ipos);
                            kfree(msg->data);    // remove msg data
                            kfree(msg);    // remove msg node
                        }

                        list_del(pos);
                        kfree(temp);    // remove temp node (queue)
                        ku_ipc_volume--;
                        spin_unlock(&ku_lock);

                        printk("[KU_IPC]close queue - %d\n", (int)arg);
                        return 0;
                    }
                }
            }

            // fail to remove queue
            spin_unlock(&ku_lock);
            return -1;
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
    msg_q.buf.data = kmalloc(KUIPC_MAXMSG, GFP_KERNEL);

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
