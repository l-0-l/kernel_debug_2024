#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include "common.h"

#define NSEC_IN_SEC (1000000000)

static struct task_struct *target_task = NULL;
static struct task_struct *update_thread;
static spinlock_t my_lock;
static unsigned long my_indicator;

extern const unsigned long my_threshold;
/****/ const unsigned long my_threshold = 30;
extern unsigned long get_my_indicator(void);
/****/ unsigned long get_my_indicator(void);
extern void set_my_indicator(unsigned long);
/****/ void set_my_indicator(unsigned long);

// Forward declaration
static int first_open(struct inode *, struct file *);
static int first_release(struct inode *, struct file *);
static ssize_t first_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);

EXPORT_SYMBOL(my_threshold);
EXPORT_SYMBOL(get_my_indicator);
EXPORT_SYMBOL(set_my_indicator);

noinline unsigned long get_my_indicator(void)
{
    unsigned long flags, retval;

    spin_lock_irqsave(&my_lock, flags);
    retval = my_indicator;
    spin_unlock_irqrestore(&my_lock, flags);

    msleep(50);

    return retval;
}

noinline void set_my_indicator(unsigned long value)
{
    unsigned long flags;

    trace_printk("[First] inside %s with value %lu\n", __func__, value);
    spin_lock_irqsave(&my_lock, flags);
    my_indicator = value;
    spin_unlock_irqrestore(&my_lock, flags);
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if(cmd == IOCTL_CHANGE_PID)
    {
        spin_lock(&my_lock);
        target_task = pid_task(find_vpid(arg), PIDTYPE_PID);
        spin_unlock(&my_lock);
    }
    if(cmd == IOCTL_CAUSE_OOPS)
    {
        WARN_ONCE(1, "[First] manually generated oops, time = %lu", (long unsigned)ktime_get_ns());
    }
    return 0;
}

static unsigned long get_start_time(void)
{
    unsigned long retval;

    spin_lock(&my_lock);
    retval = target_task->start_time;
    spin_unlock(&my_lock);

    return retval;
}

static struct file_operations fops =
    {
        .unlocked_ioctl = my_ioctl,
        .owner = THIS_MODULE,
        .open = first_open,
        .release = first_release,
        .read = first_read,
};

static struct miscdevice first_misc_device =
    {
        .minor = MISC_DYNAMIC_MINOR,
        .name = DEVICE_NAME,
        .fops = &fops,
};

static int first_open(struct inode *inode, struct file *file)
{
    dev_dbg(first_misc_device.this_device, "opened\n");
    return 0;
}

static int first_release(struct inode *inode, struct file *file)
{
    dev_dbg(first_misc_device.this_device, "closed\n");
    return 0;
}

static ssize_t first_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char message[] = "Nothing to see here\n";
    size_t len = strlen(message);

    if (*ppos >= len) return 0;
    if (count > len - *ppos) count = len - *ppos;
    if (copy_to_user(buf, message + *ppos, count)) return -EFAULT;

    dev_dbg(first_misc_device.this_device, "printed stuff\n");
    *ppos += count;
    return count;
}

static int update_indicator_thread(void *data)
{
    unsigned long int ktime_ns;
    unsigned long int stime_ns;

    while(!kthread_should_stop())
    {
        if(target_task)
        {
            ktime_ns = ktime_get_ns();
            stime_ns = get_start_time();
            set_my_indicator((ktime_ns - stime_ns) / NSEC_IN_SEC);
        }
        dynamic_pr_debug("[First] my_indicator %lu\n", my_indicator);

        msleep(1000);
    }
    return 0;
}

static int __init first_init(void)
{
    target_task = current;
    spin_lock_init(&my_lock);
    if(misc_register(&first_misc_device))
        return -ENODEV;
    update_thread = kthread_run(update_indicator_thread, NULL, "update_indicator_thread");
    if(IS_ERR(update_thread))
        return PTR_ERR(update_thread);
    pr_info("[First] module loaded\n");
    return 0;
}

static void __exit first_exit(void)
{
    if(update_thread)
        kthread_stop(update_thread);
    misc_deregister(&first_misc_device);
    pr_info("[First] module unloaded\n");
}

module_init(first_init);
module_exit(first_exit);
MODULE_LICENSE("GPL");
