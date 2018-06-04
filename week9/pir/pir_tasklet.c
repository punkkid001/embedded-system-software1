#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define PIR 17

MODULE_LICENSE("GPL");

struct my_data
{
    int year;
    int month;
    int day;
};

struct my_data my_tasklet_data;
struct tasklet_struct my_tasklet;
//DECLARE_TASKLET(my_tasklet, simple_tasklet_func, (unsigned_long)&my_tasklet_data);

static int irq_num;

void pir_tasklet_func(unsigned long recv_data)
{
    struct my_data *temp_data;
    temp_data = (struct my_data*)recv_data;

    printk("today is %d/%d/%d\n", temp_data->year, temp_data->month, temp_data->day);
}

static irqreturn_t pir_tasklet_isr(int irq, void *data)
{
    printk("ISR start\n");
    tasklet_schedule(&my_tasklet);

    return IRQ_HANDLED;
}

static int __init simple_tasklet_init(void)
{
    int ret;

    printk("init module\n");

    my_tasklet_data.year = 2018;
    my_tasklet_data.month = 5;
    my_tasklet_data.day = 16;

    tasklet_init(&my_tasklet, pir_tasklet_func, (unsigned long)&my_tasklet_data);
    tasklet_schedule(&my_tasklet);

    gpio_request_one(PIR, GPIOF_IN, "PIR");
    irq_num = gpio_to_irq(PIR);
    ret = request_irq(irq_num, pir_tasklet_isr, IRQF_TRIGGER_FALLING, "pir_irq", NULL);
    if(ret)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
        free_irq(irq_num, NULL);
    }

    return 0;
}

static void __exit simple_tasklet_exit(void)
{
    tasklet_kill(&my_tasklet);
    printk("exit module\n");

    free_irq(irq_num, NULL);
    gpio_free(PIR);
}

module_init(simple_tasklet_init);
module_exit(simple_tasklet_exit);
