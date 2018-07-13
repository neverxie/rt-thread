#include <rtthread.h>
#include "rdbd.h"

static void dump_data(const char *buffer, rt_size_t count)
{
    int i = 0;
    for(i = 0; i < count; i++)
    {
        rt_kprintf("0x%02X ",buffer[i]);
        if((1+i)%8 == 0)
        {
            rt_kprintf("\n");
        }
    }
}

static rt_size_t rdbd_pipe_test_write(rt_device_t device, rt_off_t pos, const void *buffer, rt_size_t count)
{
    dump_data(buffer, count);
}
static void rdbd_test_entry(void * _pipe)
{
    rdbd_pipe_device_t pipe = (rdbd_pipe_device_t)_pipe;
    rt_size_t read_size = 0;
    rt_device_open((rt_device_t)pipe, 0);
    while(1)
    {
        read_size = rt_device_read((rt_device_t)pipe, 0, pipe->msg->msg, pipe->parent.bufsz);
        if(read_size)
        {
            dump_data(pipe->msg->msg, read_size);
        }
    }
}

int rdbd_pipe_test_case(void)
{
    rdbd_device_t rdbd = rdbd_device_register("rdbd", rdbd_pipe_test_write, 2);
    rt_device_t pipe;
    rt_thread_t tid;
    if(rdbd == RT_NULL)
    {
        rt_kprintf("rdbd create error\n");
        return -1;
    }
    pipe = (rt_device_t)rdbd_pipe_create(rdbd, "pt0", RDBD_SERVICE_ID_GENERAL, RDBD_PIPE_DEVICE_TO_HOST, 128);
    
    if(pipe == RT_NULL)
    {
        rt_kprintf("pipe create error\n");
        return -1;
    }
    pipe = (rt_device_t)rdbd_pipe_create(rdbd, "pt1", RDBD_SERVICE_ID_FILE, RDBD_PIPE_HOST_TO_DVICE, 128);
    if(pipe == RT_NULL)
    {
        rt_kprintf("pipe create error\n");
        return -1;
    }
    tid = rt_thread_create("pt1", rdbd_test_entry, pipe, 1024, 8, 10);
    if(tid == RT_NULL)
    {
        return -1;
    }
    rt_thread_startup(tid);
    return 0;
}
MSH_CMD_EXPORT(rdbd_pipe_test_case, rdbd_pipe_test_case);


static int rdbd_write(void)
{
    rt_device_t pipe;
    pipe = rt_device_find("pt0");
    rt_device_open(pipe, 0);
    rt_device_write(pipe, 0, "11\n", 4);
    rt_device_write(pipe, 0, "22\n", 4);
    rt_device_write(pipe, 0, "33\n", 4);
    rt_device_write(pipe, 0, "44\n", 4);
}
MSH_CMD_EXPORT(rdbd_write, rdbd_write);


static int rdbd_host_write(void)
{
    rdbd_device_t rdbd;
    rdbd_pipe_msg_t msg;
    rdbd = (rdbd_device_t)rt_device_find("rdbd");
    msg = rt_malloc(4+4);
    msg->pipe_address = rdbd->get_pipe_addr(rdbd, RDBD_SERVICE_ID_FILE, RDBD_PIPE_HOST_TO_DVICE);
    msg->msg_len = 4;
    rt_memcpy(msg->msg, "55\n", 4);
    rdbd->msg_receiver(rdbd, RDBD_RAW_MSG(msg));
    return 0;
}

MSH_CMD_EXPORT(rdbd_host_write, rdbd_host_write);

static int rdbd_dump_service(void)
{
    rdbd_device_t rdbd;
    rt_uint8_t * service;
    rdbd = (rdbd_device_t)rt_device_find("urdbd");
    service = rdbd->get_service_list(rdbd);
    dump_data(service, service[0]);
    return 0;
}
MSH_CMD_EXPORT(rdbd_dump_service, rdbd_dump_service);
