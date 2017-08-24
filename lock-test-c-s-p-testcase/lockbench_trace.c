#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/percpu.h>
#include <linux/kallsyms.h>
#include <asm/delay.h>

#define CREATE_TRACE_POINTS
#include "lockbench_trace.h"

EXPORT_TRACEPOINT_SYMBOL(lock);
EXPORT_TRACEPOINT_SYMBOL(unlock);

static int lockbench_init(void)
{
	return 0;
}

static void lockbench_exit(void)
{
}

module_init(lockbench_init);
module_exit(lockbench_exit);
MODULE_LICENSE("GPL");
