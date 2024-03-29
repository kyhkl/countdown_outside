/*
 * 程序清单：创建和删除线程例程
 *
 * 这个例子会创建两个线程，在一个线程中删除另外一个线程。
 */
#include <rtthread.h>

#define THREAD_PRIORITY     25
#define THREAD_STACK_SIZE   512
#define THREAD_TIMESLICE    5

/*
 * 线程删除(rt_thread_delete)函数仅适合于动态线程，为了在一个线程
 * 中访问另一个线程的控制块，所以把线程块指针声明成全局类型以供全
 * 局访问
 */
static rt_thread_t tid1 = RT_NULL, tid2 = RT_NULL;
/* 线程1的入口函数 */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count = 0;

    while (1)
    {
        /* 线程1采用低优先级运行，一直打印计数值 */
        rt_kprintf("thread count: %d\n", count ++);
        count ++;
    }
}

static void thread1_cleanup(struct rt_thread *tid)
{
    if (tid != tid1)
    {
        return ;
    }
    rt_kprintf("thread1 end\n");
    tid1 = RT_NULL;
}

/* 线程2的入口函数 */
static void thread2_entry(void *parameter)
{
    /* 线程2拥有较高的优先级，以抢占线程1而获得执行 */

    /* 线程2启动后先睡眠10个OS Tick */
    rt_thread_delay(10);

    /*
     * 线程2唤醒后直接删除线程1，删除线程1后，线程1自动脱离就绪线程
     * 队列
     */
    rt_thread_delete(tid1);

    /*
     * 线程2继续休眠10个OS Tick然后退出，线程2休眠后应切换到idle线程
     * idle线程将执行真正的线程1控制块和线程栈的删除
     */
    rt_thread_delay(10);
}

static void thread2_cleanup(struct rt_thread *tid)
{
    /*
     * 线程2运行结束后也将自动被删除(线程控制块和线程栈在idle线
     * 程中释放)
     */

    if (tid != tid2)
    {
        return ;
    }
    rt_kprintf("thread2 end\n");
    tid2 = RT_NULL;
}

/* 线程示例的初始化 */
int thread_sample_init()
{
    /* 创建线程1 */
    tid1 = rt_thread_create("t1", /* 线程1的名称是t1 */
                            thread1_entry, RT_NULL,   /* 入口是thread1_entry，参数是RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL) /* 如果获得线程控制块，启动这个线程 */
    {
        tid1->cleanup = thread1_cleanup;
        rt_thread_startup(tid1);
    }

    /* 创建线程1 */
    tid2 = rt_thread_create("t2", /* 线程1的名称是t2 */
                            thread2_entry, RT_NULL,   /* 入口是thread2_entry，参数是RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid2 != RT_NULL) /* 如果获得线程控制块，启动这个线程 */
    {
        tid2->cleanup = thread2_cleanup;
        rt_thread_startup(tid2);
    }

    return 0;
}
/* 如果设置了RT_SAMPLES_AUTORUN，则加入到初始化线程中自动运行 */
#if defined (RT_SAMPLES_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
    INIT_APP_EXPORT(thread_sample_init);
#endif
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_sample_init, run signal sample);
