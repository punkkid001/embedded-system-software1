#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>

#define LED1 4
#define LED2 5
#define LED3 6

MODULE_LICENSE("GPL");

unsigned int tick_tock = 0;
static struct timer_list ch2_timer;

static void ch2_timer_func(unsigned long data)
{
    printk("ch2_timer: %ld\n", data);
    tick_tock++;

    if(tick_tock % 3 == 0)
    {
        gpio_set_value(LED1, 1);
        gpio_set_value(LED2, 0);
        gpio_set_value(LED3, 0);
    }

    else if(tick_tock % 3 == 1)
    {
        gpio_set_value(LED1, 0);
        gpio_set_value(LED2, 1);
        gpio_set_value(LED3, 0);
    }

    else
    {
        gpio_set_value(LED1, 0);
        gpio_set_value(LED2, 0);
        gpio_set_value(LED3, 1);
    }

    ch2_timer.data = data + 1;
    ch2_timer.expires = jiffies + (1 * HZ);
    add_timer(&ch2_timer);
}

static int __init ch2_timer_init(void)
{
    printk("[CH2]Init\n");

    gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
    gpio_request_one(LED2, GPIOF_OUT_INIT_LOW, "LED2");
    gpio_request_one(LED3, GPIOF_OUT_INIT_LOW, "LED3");

    init_timer(&ch2_timer);
    ch2_timer.function = ch2_timer_func;
    ch2_timer.data = 1L;
    ch2_timer.expires = jiffies + (1 * HZ);
    add_timer(&ch2_timer);

    return 0;
}

static void __exit ch2_timer_exit(void)
{
    printk("[CH2]Exit\n");

    gpio_set_value(LED1, 0);
    gpio_set_value(LED2, 0);
    gpio_set_value(LED3, 0);

    gpio_free(LED1);
    gpio_free(LED2);
    gpio_free(LED3);

    del_timer(&ch2_timer);
}

module_init(ch2_timer_init);
module_exit(ch2_timer_exit);
