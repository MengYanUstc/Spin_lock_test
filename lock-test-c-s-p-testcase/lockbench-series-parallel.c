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
	unsigned long work_num;
	struct completion *threads_done;
	atomic_t *threads_left;
	unsigned long lock_num;
	unsigned long lock_delay;
	unsigned long lock_hold;
	unsigned long unlock_hold;
};


static struct spinlock test_spinlock;

static int threads_num = 8;
module_param(threads_num, int, 0);

static int threads_work_num = 10000;
module_param(threads_work_num, int, 0);

static int test_done = 0;
module_param(test_done, int, S_IRUGO|S_IWUSR);

static struct task_struct *monitor_task;

static int lock_unlock_hold = 4000;

DEFINE_PER_CPU(struct thread_data, thread_datas);


struct task_struct *(*kthread_create_on_cpu_ptr)(int (*threadfn)(void *data),
		void *data, unsigned int cpu,
		const char *namefmt);

int (*sched_setscheduler_nocheck_ptr)(struct task_struct *p, int policy,
		const struct sched_param *param);

//static unsigned long unlock_time;

static int thread_fn(void *arg)
{
	struct thread_data *td = (struct thread_data *)arg;
	unsigned long i = 0;
	//unsigned long lock_num = 0;
	//unsigned long lock_delay = 0;
	unsigned long lock_hold = td->lock_hold;
	unsigned long unlock_hold = td->unlock_hold;

	while (1) {
		spin_lock(&test_spinlock);
/*
		if (unlock_time) {
			lock_num++;
			lock_delay += (ktime_to_ns(ktime_get()) - unlock_time);
		}
*/
		ndelay(lock_hold);
		//unlock_time = ktime_to_ns(ktime_get());
		spin_unlock(&test_spinlock);

		ndelay(unlock_hold);
		if (i++ == td->work_num)
			break;
	}
/*
	td->lock_num = lock_num;
	td->lock_delay = lock_delay;
*/
	if (atomic_dec_and_test(td->threads_left))
		complete(td->threads_done);
	do_exit(0);
	return 0;
}

static int monitor(void *unused)
{
	atomic_t threads_left;
	unsigned int test_threads = 1;
	DECLARE_COMPLETION_ONSTACK(threads_done);
	struct thread_data *td;
	int ret = 0, i, have_one_thread = 0;
	struct sched_param param = {.sched_priority = 1};

	unsigned long all_work_num = threads_num * threads_work_num;
	unsigned long test_work_num, fact_all_work;

	ktime_t test_begin_time, test_end_time, test_mid_time;
	unsigned long duration1, duration2;

	unsigned long proportion = 1;
	unsigned long lock_hold, unlock_hold;
//	unsigned long lock_num = 0, lock_delay = 0;

repeat:
	reinit_completion(&threads_done);
	spin_lock_init(&test_spinlock);

	lock_hold = lock_unlock_hold/(proportion + 1);
	unlock_hold = lock_unlock_hold - lock_hold;

	atomic_set(&threads_left, test_threads);

	test_work_num = all_work_num / test_threads;
	if (all_work_num % test_threads)
		fact_all_work = test_threads * test_work_num;
	else
		fact_all_work = all_work_num;

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
		td->work_num = test_work_num;
		td->threads_done = &threads_done;
		td->threads_left = &threads_left;
		td->lock_hold = lock_hold;
		td->unlock_hold = unlock_hold;

		ret = sched_setscheduler(monitor_task, SCHED_FIFO, &param);
		if (ret) {
			ret = 1;
			atomic_set(&threads_left, i + 1);
			goto error_out;
		}
	}

	test_begin_time = ktime_get();

	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		wake_up_process(td->task);
	}
	test_mid_time = ktime_get();

	/* wait for work done */
	wait_for_completion(&threads_done);
	test_end_time = ktime_get();

/*
	lock_num = lock_delay = 0;
	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		lock_num += td->lock_num;
		lock_delay += td->lock_delay;
	}
*/

	/* print this test result */
	duration1 = ktime_to_ns(ktime_sub(test_end_time, test_mid_time));
	duration2 = ktime_to_ns(ktime_sub(test_end_time, test_begin_time));
	printk("lockbench: %d %lu %lu %lu %lu %lu lock-unlock: %luns %luns %lu\n", // lock_delay: %lu %lu %lu\n",
			test_threads, test_work_num, fact_all_work, duration1, duration2,
			fact_all_work * 1000000000 / duration2,
			lock_hold, unlock_hold,
			proportion);
/*
			lock_num, lock_delay,
			lock_delay / lock_num);
*/
	proportion++;
	if (proportion == 21) {
		proportion = 1;
		test_threads++;

		if (test_threads <= threads_num)
			goto repeat;
		else
			goto error_out;
	}
	goto repeat;

error_out:
	if (ret) {
		printk("test failed\n");
		for (i = 0; i < test_threads; i++) {
			td = &per_cpu(thread_datas, i);
			if (IS_ERR(td->task))
				break;
			have_one_thread = 1;
			wake_up_process(td->task);
		}
		if (have_one_thread)
			wait_for_completion(&threads_done);
		test_done = -1;
	} else
		test_done = 1;
	module_put(THIS_MODULE);
	return 0;
}

static __init int lockbench_init(void)
{
	int ret;
	struct sched_param param = {.sched_priority = 1};

	monitor_task = kthread_create_on_node(monitor, NULL, cpu_to_node(threads_num),
		"monitor-%d", threads_num);
	if (IS_ERR(monitor_task))
		return PTR_ERR(monitor_task);
	/* we use cpu [0, threads_num] to place monitor and works threads, FIFO priority = 1 */
	kthread_bind(monitor_task, threads_num);

	ret = sched_setscheduler(monitor_task, SCHED_FIFO, &param);
	if (ret)
		return ret;

	__module_get(THIS_MODULE);
	wake_up_process(monitor_task);
	return 0;
}

static __exit void lockbench_exit(void)
{
/*	if (!test_done)
		return -EBUSY;
	return 0;
*/
}

module_init(lockbench_init);
module_exit(lockbench_exit);
MODULE_LICENSE("GPL");

