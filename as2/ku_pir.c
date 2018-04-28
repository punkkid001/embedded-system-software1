#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include "ku_pir.h"

MODULE_LICENSE("GPL");

struct ku_pir_list {
    struct list_head list;
    struct ku_pir_data data;
};

dev_t dev_num;
int ku_pir_volume;
struct ku_pir_list ku_list;

static int irq_num;
wait_queue_head_t wq;
static struct cdev *cd_cdev;


static int ku_pir_open(struct inode *inode, struct file *file)
{
    printk("[KU_PIR] Open\n");
    return 0;
}

static int ku_pir_release(struct inode *inode, struct file *file)
{
    printk("[KU_PIR] Release\n");
    return 0;
}

static int ku_pir_read(struct file *file, char *buf, size_t len, loff_t *loff)
{
    printk("[KU_PIR] Read: len %d\n", len);
    return 0;
}

static int ku_pir_write(struct file *file, char *buf, size_t len, loff_t *loff)
{
    printk("[KU_PIR] Write: len %d\n", len);
    return 0;
}

static long ku_pir_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("[KU_PIR] Ioctl: arg %ld\n", arg);

    switch(cmd)
    {
        case KU_FLUSH:
            break;
    }

    return 0;
}

static irqreturn_t ku_pir_isr(int irq, void *dev)
{
    struct ku_pir_list *item = NULL, *tmp = NULL;
    struct list_head *pos = NULL, *q = NULL;

    item = kmalloc(sizeof(struct ku_pir_list), GFP_ATOMIC);
    item->data.timestamp = jiffies;
    item->data.rf_flag = 10;

    printk("[KU_PIR] Detect\n");

    if(gpio_get_value(KUPIR_SENSOR) == 0)
        item->data.rf_flag = 1;
    else if(gpio_get_value(KUPIR_SENSOR) == 1)
        item->data.rf_flag = 0;

    // NEED TO CHECK
    if(ku_pir_volume == KUPIR_MAX_MSG)
    {
        list_for_each_safe(pos, q, &ku_list.list)
        {
            tmp = list_entry(pos, struct ku_pir_list, list);
            list_del(pos);
            kfree(tmp);
        }
    }
    
    // NEED TO CHECK
    list_add_tail(&ku_list.list, &(item->list));

    ku_pir_volume++;

    //wake_up(&wq);
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
    int ret;
    printk("[KU_PIR] Init\n");

    // Init character device
    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &ku_pir_fops);
    cdev_add(cd_cdev, dev_num, 1);

    // Init GPIO
    gpio_request_one(KUPIR_SENSOR, GPIOF_IN, "ku_pir_sensor");
    irq_num = gpio_to_irq(KUPIR_SENSOR);
    ret = request_irq(irq_num, ku_pir_isr, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "sensor", NULL);
    if(ret)
    {
        printk(KERN_ERR, "[KU_PIR] ERROR: unable to request IRQ %d\n", ret);
        free_irq(irq_num, NULL);
    }
    else
        disable_irq(irq_num);

    // Init waitqueue
    init_waitqueue_head(&wq);
    // Init list
    INIT_LIST_HEAD(&ku_list);
    ku_pir_volume = 0;

    return 0;
}

static void __exit ku_pir_exit(void)
{
    struct ku_pir_list *tmp = NULL;
    struct list_head *pos = NULL, *q = NULL;

    printk("[KU_PIR] Exit\n");

    // Remove list
    /*
    list_for_each_entry_safe(pos, q, &(ku_list.list), list)
    {
        list_del_rcu(&(pos->list));
        kfree(pos);
    }
    */

    // NEED TO CHECK
    list_for_each_safe(pos, q, &ku_list.list)
    {
        tmp = list_entry(pos, struct ku_pir_list, list);
        list_del(pos);
        kfree(tmp);
    }
    
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num);
    free_irq(irq_num, NULL);
    gpio_free(KUPIR_SENSOR);
}

module_init(ku_pir_init);
module_exit(ku_pir_exit);
