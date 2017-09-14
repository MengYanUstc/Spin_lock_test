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

struct thread_data {
	int cpu;
	struct task_struct *task;
	long long max_count;
	long long counter;

};

static struct spinlock test_spinlock;

unsigned int test_threads = 0;
static int threads_num = 47;
module_param(threads_num, int, 0);

ktime_t kt_begin_time, kt_end_time;
long long lu_begin_time, lu_end_time;
long long lu_begin_count, lu_end_count;

//static atomic_t b_ts2Change;

//struct percpu_counter counter_spinlock;
long long max_spinlock_count = 0;


static atomic_t threads_left;
static atomic_t threads_come;
//static DECLARE_COMPLETION(snap_start);
static DECLARE_COMPLETION(threads_done);
//bool b_waitFlag = true;


static int test_done = 0;
module_param(test_done, int, S_IRUGO|S_IWUSR);

static struct task_struct *monitor_task;

DEFINE_PER_CPU(struct thread_data, thread_datas);
//DEFINE_PER_CPU(int, percpuCount);

struct task_struct *(*kthread_create_on_cpu_ptr)(int (*threadfn)(void *data),
		void *data, unsigned int cpu,
		const char *namefmt);

int (*sched_setscheduler_nocheck_ptr)(struct task_struct *p, int policy,
		const struct sched_param *param);

static int thread_fn(void *arg)
{
	struct thread_data *td = (struct thread_data *)arg;
	//printk(get_cpu_var(percpuCount));
	long long *localCounter = &(td->counter);
	long long *localMax = &(td->max_count);
	while (1) {
		spin_lock(&test_spinlock);
		(*localCounter)++;
		if((*localCounter)>(*localMax))
		{
			spin_unlock(&test_spinlock);
			break;
		}
		spin_unlock(&test_spinlock);
	}
	
	if (atomic_dec_and_test(&threads_left)) {
		//printk("6");
		complete(&threads_done);
	}
	
	//printk("7");
	do_exit(0);
	return 0;
}


/*static int snap(void *unused)
{
	int i;
	unsigned int num;
	
	
	wait_for_completion(&snap_start);
	do_gettimeofday(&ts_1);
	
	printk("start snap");
	
	
	
	while (1) {
		
		if(b_threadComplete){
			if(test_threads==threads_num){
				printk("stop snap");
				break;
			}
			
			printk("change thread num");
			wait_for_completion(&snap_start);
			reinit_completion(&snap_start);
			do_gettimeofday(&ts_1);
		}
		
		complete(&snap_done);
		if (READ_ONCE(snap_exit))
			break;
	}
	do_exit(0);
	return 0;
}*/


static int monitor(void *unused)
{
	
	struct thread_data *td;
	int ret = 0, i, have_one_thread = 0;
	struct sched_param param = {.sched_priority = 1};
	printk("start monitor");
	max_spinlock_count = 1<<22;
	printk("max_count: %lld", max_spinlock_count);
	
repeat:
	//reinit_completion(&snap_start);
	reinit_completion(&threads_done);
	//b_waitFlag = true;
	
	
	spin_lock_init(&test_spinlock);
	test_threads++;
	//success_count = 0;
	
	lu_begin_count = 0;
	lu_end_count = 0;
	
	atomic_set(&threads_left, test_threads);
	atomic_set(&threads_come, test_threads);
	//atomic_set(&b_ts2Change, 1);
	//printk("1");
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
		
		//ARM65
		// if(test_threads<=32){
			// td->max_count = max_spinlock_count;
		// }else{
			// td->max_count = max_spinlock_count/10;
		// }
		
		//x86
		if(test_threads <= 2)
			td->max_count = max_spinlock_count;
		else 
			td->max_count = max_spinlock_count/4;
		
		td->counter = 0;

		ret = sched_setscheduler(monitor_task, SCHED_FIFO, &param);
		if (ret) {
			ret = 1;
			atomic_set(&threads_left, i);
			goto error_out;
		}
		
		//per_cpu(percpuCount,i) = i;
	}
	
	//printk("2");

	for (i = 0; i < test_threads; i++) {
		td = &per_cpu(thread_datas, i);
		wake_up_process(td->task);
	}
	mdelay(1);
	
	kt_begin_time = ktime_get();
	for(i = 0; i < test_threads; i++){
		lu_begin_count+=per_cpu(thread_datas, i).counter;
	}
	
	//ARM 64
	// if(test_threads==1){
		// mdelay(60);
	// }else if(test_threads > 1 && test_threads <= 4){
		// mdelay(120);
	// }else if(test_threads > 4 && test_threads <= 32){
		// mdelay(360);
	// }else {
		// mdelay(720);
	// }
	
	//x86
	if(test_threads==1){
		mdelay(90);
	}else if(test_threads==2){
		mdelay(300);
	}else{
		mdelay(300);
	}
	
	
	kt_end_time = ktime_get();
	for(i = 0; i < test_threads; i++){
		lu_end_count+=per_cpu(thread_datas, i).counter;
	}

	
	//x86
	for(i = 0; i < test_threads; i++){
		if(test_threads<=2){
			if(per_cpu(thread_datas, i).counter>max_spinlock_count)
			printk("thread %d out of range", i);
		}else {
			if(per_cpu(thread_datas, i).counter>max_spinlock_count/4)
			printk("thread %d out of range", i);
		}
	}
	
	
	
	//ARM64
	// for(i = 0; i < test_threads; i++){
		// if(test_threads<=32){
			// if(per_cpu(thread_datas, i).counter>max_spinlock_count)
			// printk("thread %d out of range", i);
		// }else {
			// if(per_cpu(thread_datas, i).counter>max_spinlock_count/10)
			// printk("thread %d out of range", i);
		// }
		
	// }
	
	lu_begin_time = ktime_to_us(kt_begin_time);
	lu_end_time = ktime_to_us(kt_end_time);
	printk("cores: %d kt_start: %lld kt_end: %lld count: %lld", test_threads, lu_begin_time, lu_end_time, lu_end_count-lu_begin_count);


	//wait_for_completion(&snap_start);
	//b_waitFlag=false;
	//printk("wait flag change %d",b_waitFlag);
	/* print this test result */
	
	wait_for_completion(&threads_done);
	printk("round: %d", test_threads);

	if (test_threads < threads_num)
		goto repeat;
	
	//percpu_counter_destroy(&counter_spinlock);
	printk("end monitor\n");
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