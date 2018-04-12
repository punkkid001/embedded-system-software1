#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/cdev.h>

#define DEV_NAME "ch3_dev"
#define SENSOR1 17
#define LED1 6

MODULE_LICENSE("GPL");

int is_detect = 0;
int light_status = 0;
static int irq_num;
static struct timer_list my_timer;

static void my_timer_func(unsigned long data)
{
    printk("timer: %ld\n", data);

    if(is_detect == 1)
    {
        gpio_set_value(LED1, 1);
        light_status = 1;
        is_detect = 0;
    }
    else
    {
        gpio_set_value(LED1, 0);
        light_status = 0;
    }

    my_timer.data = data + 1;
    my_timer.expires = jiffies + (2*HZ);

    add_timer(&my_timer);
}

static int dev_open(struct inode *node, struct file *file)
{
    printk("dev open\n");
    enable_irq(irq_num);
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    printk("dev close\n");
    disable_irq(irq_num);
    return 0;
}

struct file_operations dev_fops =
{
    .open = dev_open,
    .release = dev_release,
};

static irqreturn_t dev_isr(int irq, void* dev_id)
{
    printk("hello world\n");
    is_detect = 1;
    if(light_status == 1)
        gpio_set_value(LED1, 0);
    return IRQ_HANDLED;
}

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init dev_init(void)
{
    int ret;

    printk("init module\n");

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &dev_fops);
    cdev_add(cd_cdev, dev_num, 1);

    gpio_request_one(SENSOR1, GPIOF_IN, "sensor1");
    gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "led1");

    init_timer(&my_timer);
    my_timer.function = my_timer_func;
    my_timer.data = 1L;
    my_timer.expires = jiffies + (2*HZ);
    add_timer(&my_timer);
    
    irq_num = gpio_to_irq(SENSOR1);
    ret = request_irq(irq_num, dev_isr, IRQF_TRIGGER_FALLING, "sensor_irq", NULL);
    if(ret)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
        free_irq(irq_num, NULL);
    }
    else
        disable_irq(irq_num);

    return 0;
}

static void __exit dev_exit(void)
{
    printk("exit module\n");
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);

    free_irq(irq_num, NULL);
    del_timer(&my_timer);

    gpio_set_value(LED1, 0);
    gpio_free(SENSOR1);
    gpio_free(LED1);
}

module_init(dev_init);
module_exit(dev_exit);
