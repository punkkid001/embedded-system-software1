#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

static struct proc_dir_entry *simple_proc = NULL;
static char buf[4000];

static int my_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len;
    len = sprintf(page, "[Message] %s\n", (char *)data);
    *eof = 1;

    return len;
}

static int my_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char *kernel_data;

    kernel_data = (char *)data;
    copy_from_user(kernel_data, buffer, count);
    kernel_data[count] = '\0';

    return count;
}

int __init simple_init(void)
{
    simple_proc = create_proc_entry("simple", 0, NULL);
    if(simple_proc == NULL)
        return -ENOMEM;

    simple_proc->data = buf;
    simple_proc->read_proc = my_read;
    simple_proc->write_prcc = my_write;
    simple_proc->owner = THIS_MOUDLE;

    return 0;
}

void __exit simple_exit(void)
{
    remove_proc_entry("simple", NULL);
}

module_init(simple_init);
module_exit(simple_exit);
