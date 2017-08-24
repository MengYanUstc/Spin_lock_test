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
#include <linux/delay.h>
#include <asm/delay.h>

#include "lockbench_trace.h"

struct thread_data {
	struct task_struct *task;
	unsigned long work_num;
	unsigned long s_time;
	unsigned long p_time;
	int out_lock;
};

static int threads_num = 62;
module_param(threads_num, int, 0);

static int threads_work_num = 5000;
module_param(threads_work_num, int, 0);

static int test_done = 0;
module_param(test_done, int, S_IRUGO|S_IWUSR);

static int c_time = 33;
module_param(c_time, int, 0);

static int s_tests = 4;
module_param(s_tests, int, S_IRUGO|S_IWUSR);

static struct spinlock test_spinlock;

static unsigned int test_threads = 0;

static atomic_t threads_come;
static atomic_t threads_left;
static DECLARE_COMPLETION(threads_done);

static int snap_over;
static int snap_exit;
static DECLARE_COMPLETION(snap_start);
static DECLARE_COMPLETION(snap_done);

DEFINE_PER_CPU(struct thread_data, thread_datas);

struct task_struct *(*kthread_create_on_cpu_ptr)(int (*threadfn)(void *data),
		void *data, unsigned int cpu,
		const char *namefmt);
int (*sched_setscheduler_nocheck_ptr)(struct task_struct *p, int policy,
		const struct sched_param *param);

static int thread_fn(void *arg)
{
	struct thread_data *td = (struct thread_data *)arg;
	unsigned long i = 0;
	//unsigned long outlock_num;

	td->out_lock = 1;
	if (atomic_dec_and_test(&threads_come)) {
		/* sync to snap to begin */
		snap_over = 0;
		complete(&snap_start);
	}

	while (1) {
		td->out_lock = 0;
		//atomic_dec(&out_lock);
		spin_lock(&test_spinlock);

		ndelay(td->s_time);

		spin_unlock(&test_spinlock);
		td->out_lock = 1;
		//outlock_num = atomic_inc_return(&out_lock);

		ndelay(td->p_time);

		if (++i == td->work_num)
			break;
	}

	if (atomic_dec_and_test(&threads_left)) {
		/* sync to snap to over */
		snap_over = 1;
		wait_for_completion(&snap_done);
		reinit_completion(&snap_done);

		complete(&threads_done);
	}
	do_exit(0);
	return 0;
}

static unsigned long all_num = 0;
static unsigned long all_times = 0;

static int snap(void *unused)
{
	int i;
	unsigned int num;

	while (1) {
		/* all threads begin */
		wait_for_completion(&snap_start);
		reinit_completion(&snap_start);

		all_num = all_times = 0;

		while (!READ_ONCE(snap_over)) {
			num = 0;
			for (i = 0; i < test_threads; i++)
				if (per_cpu(thread_datas, i).out_lock)
					num++;
			all_num += num;
			all_times++;

			mdelay(500);
		}

		/* tell master to go on */
		complete(&snap_done);
		if (READ_ONCE(snap_exit))
			break;
	}
	do_exit(0);
	return 0;
}


static int s_tests_v[] = {20, 10, 5, 2, 1};

static int monitor(void *unused)
{
	struct thread_data *td;
	int ret = 0, i, have_one_thread = 0;
	struct sched_param param = {.sched_priority = 1};

	unsigned long all_work_num = threads_num * threads_work_num;
	unsigned long test_work_num, fact_all_work;

	unsigned long s_time = c_time * s_tests_v[s_tests];
	int p_to_s = 1;
	unsigned long p_time = s_time * 1000 / p_to_s;

	printk("monitor test start\n");
repeat:
	spin_lock_init(&test_spinlock);
	test_threads++;
	atomic_set(&threads_left, test_threads);
	atomic_set(&threads_come, test_threads);

	test_work_num = all_work_num / test_threads;
	/* all threads are of the same work_num */
	test_work_num = threads_work_num;
	fact_all_work = test_threads * test_work_num;

	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		td->task = kthread_create_on_node(thread_fn, td, cpu_to_node(i),
				"lockbench-%d", i);
		if (IS_ERR(td->task)) {
			ret = 1;
			atomic_set(&threads_left, i);
			atomic_set(&threads_come, i);
			goto error_out;
		}
		kthread_bind(td->task, i);

		td->work_num = test_work_num;
		td->s_time = s_time;
		td->p_time = p_time;

		ret = sched_setscheduler(td->task, SCHED_FIFO, &param);
		if (ret) {
			ret = 1;
			atomic_set(&threads_left, i + 1);
			atomic_set(&threads_come, i + 1);
			goto error_out;
		}
	}

	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		wake_up_process(td->task);
	}

	/* wait for work done */
	wait_for_completion(&threads_done);
	reinit_completion(&threads_done);

	/* print this test result */
	printk("lockbench: %d %lu %lu s_tests: %d p_to_s: %d avg_cpu: %ld %ld %ld\n",
			test_threads, test_work_num, fact_all_work,
			s_tests, p_to_s,
			all_num, all_times, all_num/all_times);

	if (test_threads < threads_num)
		goto repeat;

	/* from 1 to 31 */
	if (p_to_s < 31) {
		test_threads = 0;
		p_to_s++;
		p_time = s_time * 1000 / p_to_s;
		goto repeat;
	}

	/* we only test one s_tests once, b/c dmesg buffer small */
#if 0
	/* from 0.05 to 1 */
	if (s_tests < 4) {
		test_threads = 0;
		s_tests++;
		s_time = c_time * s_tests_v[s_tests];
		p_to_s = 1;
		p_time = s_time * 1000 / p_to_s;
		goto repeat;
	}
#endif

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
		if (have_one_thread) {
			/* let snap task exit */
			snap_exit = 1;

			wait_for_completion(&threads_done);
			reinit_completion(&threads_done);
		} else {
			snap_over = snap_exit = 1;
			complete(&snap_start);
			wait_for_completion(&snap_done);
		}
		test_done = -1;
	} else
		test_done = 1;

	module_put(THIS_MODULE);
	printk("monitor test end\n");
	do_exit(0);
	return 0;
}

static __init int lockbench_init(void)
{
static struct task_struct *monitor_task;
static struct task_struct *snap_task;
static int monitor_cpu;
static int snap_cpu;

	int ret;
	struct sched_param param = {.sched_priority = 2};

	if (threads_num > num_online_cpus() - 2)
		return -EINVAL;
	monitor_cpu = threads_num;
	snap_cpu = threads_num + 1;

	monitor_task = kthread_create_on_node(monitor, NULL, cpu_to_node(monitor_cpu),
		"monitor-%d", monitor_cpu);
	if (IS_ERR(monitor_task))
		return PTR_ERR(monitor_task);

	snap_task = kthread_create_on_node(snap, NULL, cpu_to_node(snap_cpu),
		"snap-%d", snap_cpu);
	if (IS_ERR(snap_task))
		return PTR_ERR(snap_task);

	kthread_bind(monitor_task, monitor_cpu);
	kthread_bind(snap_task, snap_cpu);

	ret = sched_setscheduler(monitor_task, SCHED_FIFO, &param);
	if (ret)
		return ret;

	ret = sched_setscheduler(snap_task, SCHED_FIFO, &param);
	if (ret)
		return ret;

	__module_get(THIS_MODULE);
	wake_up_process(monitor_task);
	wake_up_process(snap_task);
	return 0;
}

static __exit void lockbench_exit(void)
{
}

module_init(lockbench_init);
module_exit(lockbench_exit);
MODULE_LICENSE("GPL");
