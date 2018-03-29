#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <asm/delay.h>

#include "simple_spin.h"

#define DEV_NAME "simple_spin_dev"

MODULE_LICENSE("GPL");

spinlock_t my_lock;
struct str_st *kern_buf;

void delay(int sec)
{
    int i, j;
    for(i=0; i<sec; i++)
        for(j=0; j<1000; j++)
            udelay(1000);
}

static int simple_spin_read(struct str_st *buf)
{
    int ret;

    delay(5);
    spin_lock(&my_lock);
    ret = copy_to_user(buf, kern_buf, sizeof(struct str_st));
    memset(kern_buf, '\0', sizeof(struct str_st));
    spin_unlock(&my_lock);

    return ret;
}

static int simple_spin_write(struct str_st *buf)
{
    int ret;

    spin_lock(&my_lock);
    ret = copy_from_user(kern_buf, buf, sizeof(struct str_st));
    spin_unlock(&my_lock);

    return ret;
}

static long simple_spin_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct str_st *user_buf;
    int ret;

    user_buf = (struct str_st*)arg;

    switch(cmd)
    {
        case SIMPLE_SPIN_READ:
            ret = simple_spin_read(user_buf);
            printk("simple_spin_read: %d\n", ret);
            break;
        case SIMPLE_SPIN_WRITE:
            ret = simple_spin_write(user_buf);
            printk("simple_spin_write: %d\n", ret);
            break;
    }

    return 0;
}

static int simple_spin_open(struct inode *inode, struct file *file)
{
    printk("simple_spin_open\n");
    return 0;
}

static int simple_spin_release(struct inode *inode, struct file *file)
{
    printk("simple_spin release\n");
    return 0;
}

struct file_operations simple_spin_fops = {
    .unlocked_ioctl = simple_spin_ioctl,
    .open = simple_spin_open,
    .release = simple_spin_release
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_spin_init(void)
{
    int ret;

    printk("Init Module\n");

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &simple_spin_fops);
    ret = cdev_add(cd_cdev, dev_num, 1);

    if(ret < 0)
    {
        printk("Fail to add character device\n");
        return -1;
    }

    kern_buf = (struct str_st*)kmalloc(sizeof(struct str_st), GFP_KERNEL);
    memset(kern_buf, '\0', sizeof(struct str_st));

    return 0;
}

static void __exit simple_spin_exit(void)
{
    printk("Exit Module\n");

    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    kfree(kern_buf);
}

module_init(simple_spin_init);
module_exit(simple_spin_exit);
