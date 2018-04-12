#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

static struct timer_list my_timer;

static void my_timer_func(unsigned long data)
{
        printk("simple timer: %ld\n", data);
            my_timer.data = data + 1;
                my_timer.expires = jiffies + (2*HZ);

                    add_timer(&my_timer);
}

static int __init simple_timer_init(void)
{
        printk("hi timer\n");

            init_timer(&my_timer);

                my_timer.function = my_timer_func;
                    my_timer.data = 1L;
                        my_timer.expires = jiffies + (2*HZ);
                            add_timer(&my_timer);

                                return 0;
}

static void __exit simple_timer_exit(void)
{
        printk("bye timer\n");
            del_timer(&my_timer);
}

module_init(simple_timer_init);
module_exit(simple_timer_exit);
