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

struct thread_data {
	int cpu;
	struct task_struct *task;
};

static struct spinlock test_spinlock;

unsigned int test_threads = 1;
static int threads_num = 8;
module_param(threads_num, int, 0);

struct timeval ts_1;
struct timeval ts_2;
unsigned long success_count = 0;
bool b_ts2Change = false;

static atomic_t threads_left;
static atomic_t threads_come;
static DECLARE_COMPLETION(snap_start);
static DECLARE_COMPLETION(threads_done);

static int test_done = 0;
module_param(test_done, int, S_IRUGO|S_IWUSR);

static struct task_struct *monitor_task;

DEFINE_PER_CPU(struct thread_data, thread_datas);

struct task_struct *(*kthread_create_on_cpu_ptr)(int (*threadfn)(void *data),
		void *data, unsigned int cpu,
		const char *namefmt);

int (*sched_setscheduler_nocheck_ptr)(struct task_struct *p, int policy,
		const struct sched_param *param);

static int thread_fn(void *arg)
{
	if (atomic_dec_and_test(&threads_come)) {
		/* sync to snap to begin */
		complete(&snap_start);
	}
	printk("3");
	wait_for_completion(&snap_start);
	
	
	do_gettimeofday(&ts_1);
	while (1) {
		spin_lock(&test_spinlock);
		spin_unlock(&test_spinlock);
		success_count++;
		if(success_count>1000000)
			break;
	}
	
	printk("4");
	if(!b_ts2Change){
		b_ts2Change = true;
		do_gettimeofday(&ts_2);
		printk("cores: %d, start: %ld, end: %ld, count: %ld", test_threads, ts_1.tv_usec, ts_2.tv_usec, success_count);
		success_count=0;
	}
	
	printk("5");
	
	if (atomic_dec_and_test(&threads_left)) {
		printk("6");
		complete(&threads_done);
	}
	
	printk("7");
	do_exit(0);
	return 0;
}

static int monitor(void *unused)
{
	struct thread_data *td;
	int ret = 0, i, have_one_thread = 0;
	struct sched_param param = {.sched_priority = 1};

repeat:
	reinit_completion(&snap_start);
	reinit_completion(&threads_done);
	
	spin_lock_init(&test_spinlock);
	test_threads++;
	
	atomic_set(&threads_left, test_threads);
	atomic_set(&threads_come, test_threads);
	
	
	b_ts2Change = false;
	printk("1");
	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		td->task = kthread_create_on_node(thread_fn, td, cpu_to_node(i),
				"lockbench-%d", i);
		if (IS_ERR(td->task)) {
			ret = 1;
			atomic_set(&threads_left, i);
			goto error_out;
		}
		kthread_bind(td->task, i);

		td->cpu = i;

		ret = sched_setscheduler(monitor_task, SCHED_FIFO, &param);
		if (ret) {
			ret = 1;
			atomic_set(&threads_left, i);
			goto error_out;
		}
	}
	
	printk("2");

	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		wake_up_process(td->task);
	}

	/* print this test result */
	wait_for_completion(&threads_done);

	if (test_threads < threads_num)
		goto repeat;

error_out:
	if (ret) {
		printk("lockbench: test failed\n");
		for (i = 0; i < test_threads; i++) {
			td = &per_cpu(thread_datas, i);
			if (IS_ERR(td->task))
				break;
			have_one_thread = 1;
			wake_up_process(td->task);
		}
		test_done = -1;
	} else
		test_done = 1;
	module_put(THIS_MODULE);
	return 0;
}

static __init int lockbench_init(void)
{
	int ret;
	int monitor_cpu = threads_num;
	struct sched_param param = {.sched_priority = 2};

	if (threads_num > num_online_cpus())
		return -EINVAL;
	if (threads_num == num_online_cpus())
		monitor_cpu = threads_num - 1;

	monitor_task = kthread_create_on_node(monitor, NULL, cpu_to_node(monitor_cpu),
		"monitor-%d", threads_num);
	if (IS_ERR(monitor_task))
		return PTR_ERR(monitor_task);
	/* we use cpu [0, threads_num] to place monitor and works threads, FIFO priority = 1 */
	kthread_bind(monitor_task, monitor_cpu);

	ret = sched_setscheduler(monitor_task, SCHED_FIFO, &param);
	if (ret)
		return ret;

	__module_get(THIS_MODULE);
	wake_up_process(monitor_task);
	return 0;
}

static __exit void lockbench_exit(void)
{
}

module_init(lockbench_init);
module_exit(lockbench_exit);
MODULE_LICENSE("GPL");

