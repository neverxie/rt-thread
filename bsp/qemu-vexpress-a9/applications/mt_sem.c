#include <rtthread.h>
#include <stdio.h>

static rt_sem_t s1;

static void thread_1_entry(void *param) {
    int ret;
    while (1) {
        rt_kprintf("\033[36mnow in %s, %d\033[0m\n", __FUNCTION__, __LINE__);
        ret = rt_sem_take(s1, RT_WAITING_FOREVER);
        //ret = rt_sem_take(s1, 1000);
        //ret = rt_sem_take(s1, RT_WAITING_NO);
        rt_kprintf("\033[36m%s，%d，sem_take result: %d\033[0m\n", __FUNCTION__, __LINE__, ret);
        //rt_thread_delay(100);
    }
}

#if 1
static void thread_2_entry(void *param) {
    int ret;
    while (1) {
        rt_kprintf("\033[35mnow in %s，%d\033[0m\n", __FUNCTION__, __LINE__);
        ret = rt_sem_take(s1, RT_WAITING_FOREVER);
        rt_kprintf("\033[35m%s，%d，sem_take result: %d\033[0m\n", __FUNCTION__, __LINE__, ret);
        //rt_thread_delay(100);
    }
}

static void thread_3_entry(void *param) {
    int ret;
    while (1) {
        rt_kprintf("\033[34mnow in %s，%d\033[0m\n", __FUNCTION__, __LINE__);
        ret = rt_sem_take(s1, RT_WAITING_FOREVER);
        rt_kprintf("\033[34m%s，%d，sem_take result: %d\033[0m\n", __FUNCTION__, __LINE__, ret);
        //rt_thread_delay(100);
    }
}
#endif

void mt_sem_debug(struct rt_object *obj) {
    struct rt_thread *thread; 

    if (rt_object_get_type(obj) == RT_Object_Class_Semaphore) {
        struct rt_ipc_object *pipc = rt_list_entry(obj, struct rt_ipc_object, parent);
        rt_sem_t psem = rt_list_entry(pipc, struct rt_semaphore, parent);
        thread = rt_thread_self();
        rt_kprintf("\033[32m%s, %d, 信号量：%d, 当前线程名：%s\033[0m\n", __FUNCTION__, __LINE__, psem->value, psem, thread->name);
    }
}

void mt_sem_test(void) {
    rt_thread_t tid1, tid2, tid3;
    //RT_IPC_FLAG_PRIO                
    s1 = rt_sem_create("s1", 3, RT_IPC_FLAG_PRIO);
    if (s1 == NULL) {
        rt_kprintf("sem 1 create failed\n");
    }

    tid1 = rt_thread_create("t1", thread_1_entry, NULL, 1024, 10, 1);
    if (tid1 == NULL) {
        rt_kprintf("thread 1 create failed\n");
    }

#if 1
    tid2 = rt_thread_create("t2", thread_2_entry, NULL, 1024, 8, 1);
    if (tid2 == NULL) {
        rt_kprintf("thread 2 create failed\n");
    }
    tid3 = rt_thread_create("t3", thread_3_entry, NULL, 1024, 7, 1);
    if (tid3 == NULL) {
        rt_kprintf("thread 3 create failed\n");
    }
#endif

    rt_kprintf("\033[31m线程1启动前--------------------\033[0m\n");
    rt_thread_startup(tid1);
    rt_kprintf("\033[31m线程1已启动完毕!!!!!!!!!!!!!!!!\033[0m\n");

#if 1
    rt_kprintf("\033[31m线程2启动前--------------------\033[0m\n");
    rt_thread_startup(tid2);
    rt_kprintf("\033[31m线程2已启动完毕!!!!!!!!!!!!!!!!\033[0m\n");

    rt_kprintf("\033[31m线程3启动前--------------------\033[0m\n");
    rt_thread_startup(tid3);
    rt_kprintf("\033[31m线程3已启动完毕!!!!!!!!!!!!!!!!\033[0m\n");
#endif

    rt_kprintf("所有线程已启动完毕~~~~~~~~~~!!!!!!!!!!!\n");

    rt_object_trytake_sethook(mt_sem_debug);
}

INIT_APP_EXPORT(mt_sem_test);

void e(void) {
    rt_kprintf("释放信号量前\n");
    rt_sem_release(s1);
    rt_kprintf("释放信号量后\n");
}

MSH_CMD_EXPORT(e, send sem);
