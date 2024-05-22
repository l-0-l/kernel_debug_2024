#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/sysrq.h>

extern unsigned long my_threshold;
extern unsigned long get_my_indicator(void);

static struct task_struct *monitor_thread;
static struct workqueue_struct *my_wq;

static int debug_level = 0;
static int trigger_action = 0;
static char message[100] = "Hello from the depths!";
static struct ctl_table_header *my_sysctl_header;

module_param(debug_level, int, 0644);
MODULE_PARM_DESC(debug_level, "Debug level");

static struct ctl_table sysctl_table[] = {
    {
        .procname     = "debug_level",
        .data         = &debug_level,
        .maxlen       = sizeof(int),
        .mode         = 0644,
        .proc_handler = proc_dointvec
    },
    {
        .procname     = "trigger_action",
        .data         = &trigger_action,
        .maxlen       = sizeof(int),
        .mode         = 0644,
        .proc_handler = proc_dointvec
    },
    {
        .procname     = "message",
        .data         = message,
        .maxlen       = sizeof(message),
        .mode         = 0644,
        .proc_handler = proc_dostring
    },
    { }
};

static void sysrq_handler(unsigned char key)
{
	pr_info("Leon's mighty 'y' custom SysRq key in action o_O\n");
}

static struct sysrq_key_op sysrq_key_op = {
	.handler = sysrq_handler,
	.help_msg = "Leon's mighty 'y'",
	.action_msg = "Leon's mighty 'y' can do anything!",
};

static void work_say_hello(struct work_struct *work)
{
    char *argv[] = {"/usr/bin/wall", message, NULL};
    static char *envp[] = {"HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};
    call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
    pr_info("[Second] module said hello\n");
}

static DECLARE_WORK(my_work, work_say_hello);

static int monitor_indicator_thread(void *data)
{
    unsigned long my_indicator;
    unsigned short done = 0;

    while(!kthread_should_stop())
    {
        my_indicator = get_my_indicator();
        if((my_indicator > my_threshold && !done) || trigger_action)
        {
            queue_work(my_wq, &my_work);
            if(!done) done = 1;
            if(trigger_action) trigger_action = 0;
        }
        if(READ_ONCE(debug_level))
            dynamic_pr_debug("[Second] my_indicator %lu\n", my_indicator);

        msleep(1000);
    }
    return 0;
}

static int __init second_init(void)
{
    my_wq = create_singlethread_workqueue("my_workqueue");
    if(!my_wq) return -ENOMEM;

    register_sysrq_key('y', &sysrq_key_op);

    my_sysctl_header = register_sysctl("second", sysctl_table);
    if(!my_sysctl_header) {
        printk(KERN_ERR "Failed to register sysctl table.\n");
        return -ENOMEM;
    }

    monitor_thread = kthread_run(monitor_indicator_thread, NULL, "monitor_indicator_thread");
    if(IS_ERR(monitor_thread))
    {
        destroy_workqueue(my_wq);
        return PTR_ERR(monitor_thread);
    }

    pr_info("[Second] module loaded\n");
    return 0;
}

static void __exit second_exit(void)
{
    if(monitor_thread) kthread_stop(monitor_thread);
    unregister_sysrq_key('y', &sysrq_key_op);
    if(my_sysctl_header) unregister_sysctl_table(my_sysctl_header);
    if(my_wq)
    {
        flush_workqueue(my_wq);
        destroy_workqueue(my_wq);
    }
    pr_info("[Second] module unloaded\n");
}

module_init(second_init);
module_exit(second_exit);
MODULE_LICENSE("GPL");
