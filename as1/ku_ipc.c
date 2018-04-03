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

typedef struct kumsg
{
    struct list_head list;
    long type;
    int id;
    void *data;    // data type: MSGBUF
} KUMSG;

typedef struct queue
{
    struct list_head list;
    KUMSG msg_buf;    // messages in the queue
    int key;    // queue id(key)
    int size;    // capacity of the queue
    int count;
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
    KUMSG *tmp = NULL, *min = NULL;
    RCVMSG *user_msg = (RCVMSG*)buf;
    int count = 0, size = 0, flag = 0;

    spin_lock(&ku_lock);
    list_for_each_entry(temp, &msg_q.list, list)
    {
        if(temp->key == user_msg->id)
        {
            /*
            list_for_each_entry(tmp, &(temp->msg_buf).list, list)
            {
                if(count == 0)
                    min = tmp;

                if(user_msg->type == 0 && count == 0)
                {
                    if(user_msg->size < sizeof(tmp->data))
                    {
                        if(user_msg->flag == 0)
                            size = copy_to_user(user_msg->data, tmp->data, user_msg->size);
                        else
                            return -2;
                    }
                    else
                        size = copy_to_user(user_msg->data, tmp->data, sizeof(tmp->data));
                    spin_unlock(&ku_lock);
                    return size;
                }

                else if(user_msg->type > 0 && tmp->type == user_msg->type)
                {
                    if(user_msg->size < sizeof(tmp->data))
                    {
                        if(user_msg->flag == 0)
                            size = copy_to_user(user_msg->data, tmp->data, user_msg->size);
                        else
                            return -2;
                    }
                    else
                        size = copy_to_user(user_msg->data, tmp->data, sizeof(tmp->data));

                    spin_unlock(&ku_lock);
                    return size;
                }

                else if(user_msg->type < 0)
                {
                    long type = user_msg->type * (-1);
                    if(tmp->type <= type && min->type > tmp->type)
                        min = tmp;
                    flag = 1;
                }

                count++;
            }

            if(flag == 1)
            {
                if(user_msg->size < sizeof(tmp->data))
                {
                    if(user_msg->flag == 0)
                        size = copy_to_user(user_msg->data, tmp->data, user_msg->size);
                    else
                        return -2;
                }
                else
                    size = copy_to_user(user_msg->data, tmp->data, sizeof(tmp->data));

                spin_unlock(&ku_lock);
                return size;
            }
            */
            printk("[KU_IPC]Read - %d\n", temp->key);
            spin_unlock(&ku_lock);
            return 0;
        }
    }
    spin_unlock(&ku_lock);

    return -1;    // fail to read
}

// ku_msgsnd
static int ku_ipc_write(struct file *file, const char *buf, size_t len, loff_t *lof)
{
    QUEUE *temp = NULL;
    SNDMSG *user_msg = NULL;
    KUMSG *msg = NULL;
    int size = 0;

    if(len >= KUIPC_MAXMSG)
        return -2;    // oversize

    if(copy_from_user(user_msg, buf, len) > 0)
    {
        spin_lock(&ku_lock);
        list_for_each_entry(temp, &msg_q.list, list)
        {
            // find queue
            if(temp->key == user_msg->id)
            {
                /*
                size = temp->size + user_msg->size;
                // need to check
                if((size >= KUIPC_MAXVOL) || (temp->count >= KUIPC_MAXMSG))
                    return -3;    // lack of space

                msg = kmalloc(sizeof(KUMSG), GFP_KERNEL);
                msg->id = user_msg->id;
                msg->data = user_msg->data;
                msg->type = user_msg->type;
                list_add_tail(&msg->list, &(temp->msg_buf).list);

                temp->size = size;
                temp->count++;
                */

                msg = kmalloc(sizeof(KUMSG), GFP_KERNEL);
                msg->id = user_msg->id;
                msg->data = user_msg->data;
                msg->type = user_msg->type;
                list_add_tail(&msg->list, &(temp->msg_buf).list);

                printk("[KU_IPC]Write: %d\n", temp->key);
                break;
            }
        }
        spin_unlock(&ku_lock);

        printk("[KU_IPC]Write: queue id %ld\n", msg->id);
        return 0;    // success to write
    }

    return -1;    // fail to write
}

static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    QUEUE *temp = NULL;
    struct list_head *pos = NULL, *q = NULL;

    printk("[KU_IPC]Ioctl: arg %ld\n", arg);

    switch(cmd)
    {
        case KU_CHECK:
            if(list_empty(&msg_q.list))
                return 0;    // queue is empty

            spin_lock(&ku_lock);
            list_for_each_entry(temp, &msg_q.list, list)
            {
                //printk("[KU_IPC]Queue check: id %d\n", temp->key);
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
                temp->key = arg;
                temp->size = 0;
                temp->count = 0;

                INIT_LIST_HEAD(&(temp->msg_buf).list);
                (temp->msg_buf).data = kmalloc(KUIPC_MAXMSG, GFP_KERNEL);
                (temp->msg_buf).id = arg;
                (temp->msg_buf).type = -1;

                list_add_tail(&temp->list, &msg_q.list);
                ku_ipc_volume++;
                spin_unlock(&ku_lock);

                printk("[KU_IPC]Create queue: id %ld\n", arg);
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
                        KUMSG *msg;
                        struct list_head *ipos, *iq;

                        // remove list node (msg_buf)
                        list_for_each_safe(ipos, iq, &(temp->msg_buf).list)
                        {
                            msg = list_entry(ipos, KUMSG, list);
                            list_del(ipos);
                            kfree(msg->data);    // remove msg data
                            kfree(msg);    // remove msg node
                        }

                        list_del(pos);
                        kfree(temp);    // remove temp node (queue)
                        ku_ipc_volume--;
                        spin_unlock(&ku_lock);

                        printk("[KU_IPC]Close queue: id %d\n", (int)arg);
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
                    if(temp->count == 0)
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
    msg_q.key = -1;
    msg_q.size = 0;
    msg_q.count = 0;
    msg_q.msg_buf.data = kmalloc(KUIPC_MAXMSG, GFP_KERNEL);

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
