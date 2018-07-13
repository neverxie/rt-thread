#include <rtthread.h>
#include <finsh.h>
#if defined(RT_USING_POSIX)
#include <libc.h>
#include <dfs_posix.h>
static int dev_old_flag;
#endif
#define RDBD_DEVICE_NAME         "rdbdsh"//can not be edit

extern void rt_show_version(void);
extern void finsh_set_echo(rt_uint32_t echo);
int set_finsh_to_rdbd(void)
{
    /* set finsh device */
#if defined(RT_USING_POSIX)
    //    rt_device_find(RDBD_DEVICE_NAME)->rx_indicate = rt_console_get_device()->rx_indicate;
    rt_console_set_device(RDBD_DEVICE_NAME);
    /* backup flag */
    dev_old_flag = ioctl(libc_stdio_get_console(), F_GETFL, (void *)RT_NULL);
    /* add non-block flag */
    ioctl(libc_stdio_get_console(), F_SETFL, (void *)(dev_old_flag | O_NONBLOCK));
    /* set tcp shell device for console */
    libc_stdio_set_console(RDBD_DEVICE_NAME, O_RDWR);
    /* resume finsh thread, make sure it will unblock from last device receive */
    rt_thread_t tid = rt_thread_find(FINSH_THREAD_NAME);
    if (tid)
    {
        rt_thread_resume(tid);
        rt_schedule();
    }
#else
    rt_console_set_device(RDBD_DEVICE_NAME);
    /* set finsh device */
    finsh_set_device(RDBD_DEVICE_NAME);
#endif /* RT_USING_POSIX */
    finsh_set_echo(1);
    rt_show_version();
    rt_kprintf("msh>");
    return 0;
}
int set_finsh_back(void)
{
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
    /* set finsh device */
#if defined(RT_USING_POSIX)
    ioctl(libc_stdio_get_console(), F_SETFL, (void *)dev_old_flag);
    libc_stdio_set_console(RT_CONSOLE_DEVICE_NAME, O_RDWR);
#else
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif /* RT_USING_POSIX */
    finsh_set_echo(1);
    rt_show_version();
    return 0;
}

