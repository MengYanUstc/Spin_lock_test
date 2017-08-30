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
};

static struct spinlock test_spinlock;

static int threads_num = 8;
module_param(threads_num, int, 0);

static int threads_work_num = 10000;
module_param(threads_work_num, int, 0);

struct timeval ts_1;
struct timeval ts_2;
unsigned long success_count = 0;
bool b_ts2Change = false;

static atomic_t threads_come;
static DECLARE_COMPLETION(snap_start);

static int test_done = 0;
module_param(test_done, int, S_IRUGO|S_IWUSR);

static struct task_struct *monitor_task;

DEFINE_PER_CPU(struct thread_data, thread_datas);

struct task_struct *(*kthread_create_on_cpu_ptr)(int (*threadfn)(void *data),
		void *data, unsigned int cpu,
		const char *namefmt);

int (*sched_setscheduler_nocheck_ptr)(struct task_struct *p, int policy,
		const struct sched_param *param);
		
int thread_counter = 0;

static int thread_fn(void *arg)
{
	/*
	struct thread_data *td = (struct thread_data *)arg;
	unsigned long i = 0;
	
	if (atomic_dec_and_test(&threads_come)) {
		
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
		printk("cores: , start: %ld, end: %ld, count: %ld", ts_1.tv_usec, ts_2.tv_usec, success_count);
		success_count=0;
	}
	
	printk("5");
	

	if (atomic_dec_and_test(td->threads_left))
		complete(td->threads_done);
	
	//printk("thread done: %d",thread_counter);
	thread_counter++;
	do_exit(0);
	return 0;*/
	
	struct thread_data *td = (struct thread_data *)arg;
	unsigned long i = 0;
	
	if (atomic_dec_and_test(&threads_come)) {
		
		complete(&snap_start);
	}
	wait_for_completion(&snap_start);

	while (1) {
		spin_lock(&test_spinlock);

		spin_unlock(&test_spinlock);

		if (++i == td->work_num)
			break;
	}

	if (atomic_dec_and_test(td->threads_left))
		complete(td->threads_done);
	do_exit(0);
	return 0;
	
}

static int monitor(void *unused)
{
	atomic_t threads_left;
	unsigned int test_threads = 0;
	DECLARE_COMPLETION_ONSTACK(threads_done);
	struct thread_data *td;
	int ret = 0, i, have_one_thread = 0;
	struct sched_param param = {.sched_priority = 1};

	unsigned long all_work_num = threads_num * threads_work_num;
	unsigned long test_work_num, fact_all_work;

repeat:

	thread_counter = 0;
	
	reinit_completion(&threads_done);
	reinit_completion(&snap_start);
	spin_lock_init(&test_spinlock);

	test_threads++;
	atomic_set(&threads_left, test_threads);
	atomic_set(&threads_come, test_threads);

	test_work_num = all_work_num / test_threads;
	fact_all_work = test_threads * test_work_num;

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

		ret = sched_setscheduler(monitor_task, SCHED_FIFO, &param);
		if (ret) {
			ret = 1;
			atomic_set(&threads_left, i + 1);
			goto error_out;
		}
	}

	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		wake_up_process(td->task);
	}

	/* wait for work done */
	wait_for_completion(&threads_done);
	printk("finish thread_num: %d", test_threads);
	/* print this test result */
	/*printk("lockbench: %d %lu %lu\n",
			test_threads, test_work_num, fact_all_work);*/

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

