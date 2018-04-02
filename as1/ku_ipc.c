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

typedef struct msgbuf
{
    struct list_head list;
    long type;    // message type
    int id;    // queue id(key)
    int size;    // data size
    int flag;    // message flag
    void *data;
} MSGBUF;

typedef struct queue
{
    struct list_head list;
    MSGBUF msg_buf;    // messages in the queue
    int key;    // queue id(key)
    int size;    // capacity of the queue
} QUEUE;

QUEUE msg_q;    // root queue

static struct cdev *cd_cdev;
spinlock_t ku_lock;
dev_t dev_num;

int ku_ipc_volume = 0;

// ku_msgrcv
static int ku_ipc_read(struct file *file, char *buf, size_t len, loff_t *lof)
{
    QUEUE *temp = NULL;
    MSGBUF *msg = (MSGBUF*)buf, *tmp = NULL;
    int size = 0;
    //struct list_head *pos = NULL, *q = NULL;

    spin_lock(&ku_lock);
    list_for_each_entry(temp, &msg_q.list, list)
    {
        if(temp->key == msg->id)
        {
            list_for_each_entry(tmp, &(temp->msg_buf.list), list)
            {
                if(tmp->type == msg->type && tmp->size <= len)
                {
                    size = copy_to_user(msg->data, tmp->data, len);
                    spin_unlock(&ku_lock);
                    return size;
                }

                if(msg->flag == 0)
                {
                    size = copy_to_user(msg->data, tmp->data, sizeof(tmp->data));
                    spin_unlock(&ku_lock);
                    return size;
                }

                return -2;    // lack of space
            }

            break;
        }
    }

    spin_unlock(&ku_lock);
    return -1;    // fail to read
}

// ku_msgsnd
static int ku_ipc_write(struct file *file, const char *buf, size_t len, loff_t *lof)
{
    QUEUE *temp = NULL;
    MSGBUF *msg = NULL;
    int size = 0;

    if(len >= KUIPC_MAXMSG)
        return -2;    // oversize

    msg = (MSGBUF*)kmalloc(sizeof(MSGBUF), GFP_KERNEL);
    // msg = kmalloc(len, GFP_KERNEL); : is it okay?
    if(copy_from_user((void*)msg, (void*)buf, len) > 0)
    {
        spin_lock(&ku_lock);
        list_for_each_entry(temp, &msg_q.list, list)
        {
            if(temp->key == msg->id)
            {
                size = temp->size + msg->size;
                if(size >= KUIPC_MAXVOL)
                    return -3;    // lack of space

                //msg->type = *((long*)(msg->data));
                list_add_tail(&msg->list, &(temp->msg_buf).list);
                temp->size += msg->size;
                break;
            }
        }
        spin_unlock(&ku_lock);
        printk("[KU_IPC]write to queue id - %d\n", msg->id);
        return 0;    // success to write
    }

    return -1;    // fail to write
}

static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    QUEUE *temp = NULL;
    MSGBUF *user_buf = NULL;
    struct list_head *pos = NULL, *q = NULL;

    user_buf = (MSGBUF*)arg;
    switch(cmd)
    {
        case KU_CHECK:
            if(!list_empty(&msg_q.list))
                return -1;    // queue is not empty

            spin_lock(&ku_lock);
            list_for_each_entry(temp, &msg_q.list, list)
            {
                // duplicated keys are not allowed
                if(temp->key == (int)arg)
                {
                    spin_unlock(&ku_lock);
                    return -1;
                }
            }
            spin_unlock(&ku_lock);

            return 0;

        case KU_CREAT:
            if(ku_ipc_volume <= KUIPC_MAXVOL)
            {
                spin_lock(&ku_lock);
                temp = kmalloc(sizeof(QUEUE), GFP_KERNEL);
                temp->key = (int)arg;
                temp->size = 0;

                INIT_LIST_HEAD(&(temp->msg_buf).list);
                (temp->msg_buf).data = kmalloc(KUIPC_MAXMSG, GFP_KERNEL);

                /*
                   (temp->msg_buf).id = (int)arg;
                   (temp->msg_buf).size = 0;
                   (temp->msg_buf).flag = 0;
                   (temp->msg_buf).data = NULL;
                 */

                list_add_tail(&temp->list, &msg_q.list);
                ku_ipc_volume++;
                spin_unlock(&ku_lock);

                printk("[KU_IPC]create queue - %d\n", (int)arg);
                return arg;
            }

            // queue volume is full
            return -1;

        case KU_CLOSE:
            if(ku_ipc_volume > 0)
            {
                spin_lock(&ku_lock);
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
                spin_unlock(&ku_lock);

                // fail to remove queue (there is no matching key)
                return -1;
            }

            // fail to remove queue (queue is empty)
            return -1;

        case KU_EMPTY:
            spin_lock(&ku_lock);
            list_for_each_entry(temp, &msg_q.list, list)
            {
                if(temp->key == arg)
                {
                    if(list_empty(&temp->list))
                    {
                        spin_unlock(&ku_lock);
                        return 0;    // queue is empty
                    }
                    break;
                }
            }
            spin_unlock(&ku_lock);

            return -1;    // queue is not empty
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
    .read = ku_ipc_read,
    .write = ku_ipc_write,
    .unlocked_ioctl = ku_ipc_ioctl,
    .open = ku_ipc_open,
    .release = ku_ipc_release
};

static int __init ku_ipc_init(void)
{
    printk("[KU_IPC]Init\n");

    INIT_LIST_HEAD(&msg_q.list);
    msg_q.key = 0;
    msg_q.size = 0;
    //msg_q.buf.data = kmalloc(KUIPC_MAXMSG, GFP_KERNEL);

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
