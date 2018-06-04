#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

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

void simple_tasklet_func(unsigned long recv_data)
{
    struct my_data *temp_data;
    temp_data = (struct my_data*)recv_data;

    printk("today is %d/%d/%d\n", temp_data->year, temp_data->month, temp_data->day);
}

static int __init simple_tasklet_init(void)
{
    printk("init module\n");

    my_tasklet_data.year = 2018;
    my_tasklet_data.month = 5;
    my_tasklet_data.day = 16;

    tasklet_init(&my_tasklet, simple_tasklet_func, (unsigned long)&my_tasklet_data);
    tasklet_schedule(&my_tasklet);

    return 0;
}

static void __exit simple_tasklet_exit(void)
{
    tasklet_kill(&my_tasklet);
    printk("exit module\n");
}

module_init(simple_tasklet_init);
module_exit(simple_tasklet_exit);
