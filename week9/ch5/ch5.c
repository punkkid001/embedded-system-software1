#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define PIR 17
#define LED1 5
#define LED2 26
#define LED3 16

MODULE_LICENSE("GPL");

struct my_data
{
    int led;
};

struct task_struct *test_task_a = NULL, *test_task_b = NULL;
static struct workqueue_struct *test_wq;

typedef struct
{
    struct work_struct my_work;
    struct my_data data;
}my_work_t;

my_work_t *work;

static int irq_num;

static void my_wq_func(struct work_struct *work)
{
    my_work_t *my_work = (my_work_t*)work;
    (my_work->data).led = gpio_get_value(LED1);
    
    gpio_set_value(LED1, !(my_work->data).led);

    kfree((void*)work);
}

static irqreturn_t simple_pir_isr(int irq, void *data)
{
    int ret;
    printk("ISR Start\n");

    if(test_wq)
    {
        work = (my_work_t*)kmalloc(sizeof(my_work_t), GFP_KERNEL);
        if(work)
        {
            INIT_WORK((struct work_struct*)work, my_wq_func);
            ret = queue_work(test_wq, (struct work_struct*)work);
        }
    }
    return IRQ_HANDLED;
}

int thread_func_a(void *data)
{
    int val;
    while(!kthread_should_stop())
    {
        val = gpio_get_value(LED2);
        gpio_set_value(LED2, !val);
        msleep(1000);
    }

    return 0;
}

int thread_func_b(void *data)
{
    int val;
    while(!kthread_should_stop())
    {
        val = gpio_get_value(LED3);
        gpio_set_value(LED3, !val);
        msleep(1500);
    }

    return 0;
}

static int __init kernthread_init(void)
{
    int ret;
    printk("init module\n");

    test_wq = create_workqueue("test workqueue");

    gpio_request_one(PIR, GPIOF_IN, "PIR");
    irq_num = gpio_to_irq(PIR);
    ret = request_irq(irq_num, simple_pir_isr, IRQF_TRIGGER_FALLING, "pir_irq", NULL);

    gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "led1");
    gpio_request_one(LED2, GPIOF_OUT_INIT_LOW, "led2");
    gpio_request_one(LED3, GPIOF_OUT_INIT_LOW, "led3");

    if(ret)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
        free_irq(irq_num, NULL);
    }

    test_task_a = kthread_create(thread_func_a, NULL, "my_thread_a");
    if(IS_ERR(test_task_a))
    {
        test_task_a = NULL;
        printk("test kernel thread ERROR\n");
    }

    wake_up_process(test_task_a);

    test_task_b = kthread_create(thread_func_b, NULL, "my_thread_b");
    if(IS_ERR(test_task_b))
    {
        test_task_b = NULL;
        printk("test kernel thread ERROR\n");
    }

    wake_up_process(test_task_b);

    return 0;
}

static void __exit kernthread_exit(void)
{
    printk("exit module\n");

    free_irq(irq_num, NULL);
    gpio_free(PIR);
    gpio_set_value(LED1, 0);
    gpio_set_value(LED2, 0);
    gpio_set_value(LED3, 0);

    gpio_free(LED1);
    gpio_free(LED2);
    gpio_free(LED3);

    if(test_task_a)
    {
        kthread_stop(test_task_a);
        printk("test kernel thread STOP");
    }
    if(test_task_b)
    {
        kthread_stop(test_task_b);
        printk("test kernel thread STOP");
    }

    flush_workqueue(test_wq);
    destroy_workqueue(test_wq);
}

module_init(kernthread_init);
module_exit(kernthread_exit);
