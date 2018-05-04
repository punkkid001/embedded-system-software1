#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include "ku_pir.h"

MODULE_LICENSE("GPL");

struct ku_pir_list
{
    struct list_head list;
    struct ku_pir_data data;
};

dev_t dev_num;
wait_queue_head_t wq;

int ku_pir_volume[KUPIR_MAX_DEV] = {0, }, fd_list[KUPIR_MAX_DEV] = {0, };
struct ku_pir_list ku_list[KUPIR_MAX_DEV];

static int irq_num;
static struct cdev *cd_cdev;


static int ku_pir_open(struct inode *inode, struct file *file)
{
    printk("[KU_PIR][INFO] Open\n");
    enable_irq(irq_num);

    return 0;
}

static int ku_pir_release(struct inode *inode, struct file *file)
{
    printk("[KU_PIR][INFO] Release\n");
    disable_irq(irq_num);

    return 0;
}

static int ku_pir_read(struct file *file, char *buf, size_t len, loff_t *lof)
{
    int fd = -1, ret = 0;
    struct ku_pir_capsule *user_data = (struct ku_pir_capsule *)buf;
    struct ku_pir_list *pos = NULL, *q = NULL;

    fd = user_data->fd - 1;

    // If queue is empty
    if(ku_pir_volume[fd] == 0)
        wait_event_interruptible(wq, ku_pir_volume[fd] > 0);

    // Read data
    rcu_read_lock();
    list_for_each_entry_rcu(pos, &ku_list[fd].list, list)
    {
        printk("[KU_PIR][Read][%d] ts: %lu / flag: %d\n", fd, pos->data.timestamp, pos->data.rf_flag);
        if(copy_to_user(user_data->data, &pos->data, sizeof(struct ku_pir_data)))
            ret = -1;
        break;
    }
    rcu_read_unlock();

    // Remove data
    if(ret != -1)
    {
        list_for_each_entry_safe(pos, q, &ku_list[fd].list, list)
        {
            list_del_rcu(&pos->list);
            kfree(pos);
            ku_pir_volume[fd]--;
            break;
        }
    }

    return ret;
}

static int ku_pir_write(struct file *file, const char *buf, size_t len, loff_t *lof)
{
    int fd = -1;
    struct ku_pir_capsule *user_data = NULL;
    struct ku_pir_list *item = NULL, *pos = NULL, *q = NULL;

    user_data = kmalloc(sizeof(struct ku_pir_capsule), GFP_KERNEL);
    if(copy_from_user(user_data, buf, len))
        return -1;
    fd = user_data->fd - 1;

    item = kmalloc(sizeof(struct ku_pir_list), GFP_ATOMIC);
    if(item == NULL)
        return -1;
    item->data.timestamp = user_data->data->timestamp;
    item->data.rf_flag = user_data->data->rf_flag;

    printk("[KU_PIR][Write] fd: %d\n", fd);

    // If queue is full
    if(ku_pir_volume[fd] == KUPIR_MAX_MSG)
    {
        list_for_each_entry_safe(pos, q, &ku_list[fd].list, list)
        {
            list_del_rcu(&pos->list);
            kfree(pos);
            ku_pir_volume[fd]--;
            break;
        }
    }

    list_add_tail_rcu(&item->list, &ku_list[fd].list);
    ku_pir_volume[fd]++;

    wake_up_interruptible(&wq);
    kfree(user_data);

    return 0;
}

static long ku_pir_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int i = 0, fd = -1;
    struct ku_pir_list *pos = NULL, *q = NULL;

    printk("[KU_PIR][IOCTL] arg: %ld\n", arg);

    switch(cmd)
    {
        case KU_FLUSH:
            fd = arg - 1;

            list_for_each_entry_safe(pos, q, &ku_list[fd].list, list)
            {
                list_del_rcu(&pos->list);
                kfree(pos);
            }

            INIT_LIST_HEAD(&ku_list[fd].list);
            ku_pir_volume[fd] = 0;

            return 1;

        case KU_OPEN:
            // Find empty id
            for(i=0; i<KUPIR_MAX_DEV; i++)
            {
                if(fd_list[i] == 0)
                {
                    fd_list[i] = 1;
                    fd = i + 1;
                    break;
                }
            }

            if(fd != -1)
            {
                INIT_LIST_HEAD(&ku_list[fd-1].list);
                ku_pir_volume[fd-1] = 0;
            }

            return fd;

        case KU_CLOSE:
            fd = arg - 1;
            fd_list[fd] = 0;

            list_for_each_entry_safe(pos, q, &ku_list[fd].list, list)
            {
                list_del_rcu(&pos->list);
                kfree(pos);
            }

            ku_pir_volume[fd] = 0;

            return 1;

        case KU_PRINT:
            for(fd=0; fd<KUPIR_MAX_DEV; fd++)
            {
                if(fd_list[fd] != 0)
                {
                    list_for_each_entry(pos, &ku_list[fd].list, list)
                    {
                        printk("[KU_PRINT][%d] ts: %lu / flag: %d\n", fd, pos->data.timestamp, pos->data.rf_flag);
                    }
                }
            }

            break;

        default:
            return -1;
    }

    return 0;
}

static irqreturn_t ku_pir_isr(int irq, void *dev)
{
    int fd;
    struct ku_pir_list *item = NULL, *pos = NULL, *q = NULL;

    item = kmalloc(sizeof(struct ku_pir_list), GFP_ATOMIC);
    if(item == NULL)
        return IRQ_NONE;
    item->data.timestamp = jiffies;

    if(gpio_get_value(KUPIR_SENSOR) == 0)
        item->data.rf_flag = 1;
    else if(gpio_get_value(KUPIR_SENSOR) == 1)
        item->data.rf_flag = 0;
    else
        return IRQ_NONE;

    printk("[KU_PIR][Detect] ts: %lu / flag: %d\n", item->data.timestamp, item->data.rf_flag);

    // Push to queue(linked list)
    for(fd=0; fd<KUPIR_MAX_DEV; fd++)
    {
        if(fd_list[fd] == 1)
        {
            // If queue is full
            if(ku_pir_volume[fd] == KUPIR_MAX_MSG)
            {
                list_for_each_entry_safe(pos, q, &ku_list[fd].list, list)
                {
                    list_del_rcu(&pos->list);
                    kfree(pos);
                    ku_pir_volume[fd]--;
                    break;
                }
            }

            list_add_tail_rcu(&item->list, &ku_list[fd].list);
            ku_pir_volume[fd]++;
        }
    }

    wake_up_interruptible(&wq);

    return IRQ_HANDLED;
}

struct file_operations ku_pir_fops =
{
    .open = ku_pir_open,
    .release = ku_pir_release,
    .read = ku_pir_read,
    .write = ku_pir_write,
    .unlocked_ioctl = ku_pir_ioctl
};

static int __init ku_pir_init(void)
{
    int fd, ret;
    printk("[KU_PIR][INFO] Init\n");

    // Init character device
    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &ku_pir_fops);
    cdev_add(cd_cdev, dev_num, 1);

    // Init GPIO
    gpio_request_one(KUPIR_SENSOR, GPIOF_IN, "ku_pir_sensor");
    irq_num = gpio_to_irq(KUPIR_SENSOR);
    ret = request_irq(irq_num, ku_pir_isr, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "sensor_irq", NULL);
    if(ret)
    {
        printk(KERN_ERR "[KU_PIR][ERROR] Unable to request IRQ: %d\n", ret);
        free_irq(irq_num, NULL);
    }
    else
        disable_irq(irq_num);

    // Init waitqueue
    init_waitqueue_head(&wq);
    // Init list
    for(fd=0; fd<KUPIR_MAX_DEV; fd++)
        INIT_LIST_HEAD(&ku_list[fd].list);

    return 0;
}

static void __exit ku_pir_exit(void)
{
    int fd;
    struct ku_pir_list *pos = NULL, *q = NULL;

    printk("[KU_PIR][INFO] Exit\n");

    // Remove list
    for(fd=0; fd<KUPIR_MAX_DEV; fd++)
    {
        list_for_each_entry_safe(pos, q, &(ku_list[fd].list), list)
        {
            list_del_rcu(&pos->list);
            kfree(pos);
        }
    }
 
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    free_irq(irq_num, NULL);
    gpio_free(KUPIR_SENSOR);
}

module_init(ku_pir_init);
module_exit(ku_pir_exit);
