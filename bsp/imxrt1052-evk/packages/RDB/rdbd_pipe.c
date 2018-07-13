#include "rdbd.h"
#include "rtdevice.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME  "[RDBD]"
#define DBG_LEVEL         DBG_WARNING
#define DBG_COLOR
#include <rtdbg.h>
#define MAX_PACKET_SIZE 512
static rt_bool_t check_rdbd_pipe_max(rdbd_device_t device)
{
    if(device->pipe_max_num & 0x80)
    {
        LOG_E("rdbd device: [%s] pipe max num error 0x%02X",device->parent.parent.name, device->pipe_max_num);
        return RT_FALSE;
    }
    return RT_TRUE;
}
static void rdbd_pipe_remove_in_list(rdbd_device_t device, rt_uint8_t addr)
{
    if(!check_rdbd_pipe_max(device))
    {
        return;
    }
    if((addr & 0x7f) < device->pipe_max_num)
    {
        device->rdbd_pipe_list[device->pipe_max_num * (addr >> 7) + addr & 0x7f] = RT_NULL;
    }
}
static rdbd_pipe_device_t rdbd_pipe_get(rdbd_device_t device, rt_uint8_t addr)
{
    if(!check_rdbd_pipe_max(device))
    {
        return RT_NULL;
    }
    if((addr & 0x7f) < device->pipe_max_num)
    {
        return device->rdbd_pipe_list[device->pipe_max_num * (addr >> 7) + addr & 0x7f];
    }
    return RT_NULL;
}

static rt_err_t rdbd_pipe_set(rdbd_device_t device, rdbd_pipe_device_t pipe)
{
    if(!check_rdbd_pipe_max(device))
    {
        return RT_ERROR;
    }
    if((pipe->addr & 0x7f) < device->pipe_max_num)
    {
        device->rdbd_pipe_list[device->pipe_max_num * (pipe->addr >> 7) + pipe->addr & 0x7f] = pipe;
    }
    return RT_EOK;
}

static void rdbd_pipe_write_thread_entry(void * _pipe)
{
    rdbd_pipe_device_t pipe = (rdbd_pipe_device_t)_pipe;
    rt_size_t read_size = 0;
    rt_size_t remain_size = 0;
    rt_device_open((rt_device_t)pipe, 0);
    while(1)
    {
        read_size = rt_device_read((rt_device_t)pipe, 0, pipe->msg->msg, MAX_PACKET_SIZE - 4);
        if(read_size)
        {
            pipe->msg->pipe_address = pipe->addr;
            pipe->msg->msg_len = read_size;
            rt_device_write((rt_device_t)pipe->rdbd_device, 0 , RDBD_RAW_MSG(pipe->msg), RDBD_MSG_LEN(pipe->msg));
        }
    }
}

static rt_size_t rdbd_pipe_write(rt_device_t device, rt_off_t pos, const void *buffer, rt_size_t count)
{
    uint8_t *pbuf;
    rt_size_t write_bytes = 0;
    rdbd_pipe_device_t pipe = (rdbd_pipe_device_t)device;

    if (device == RT_NULL)
    {
        rt_set_errno(-EINVAL);
        return 0;
    }
    if (count == 0) return 0;

    pbuf = (uint8_t*)buffer;
    rt_mutex_take(&pipe->parent.lock, RT_WAITING_FOREVER);

    while (write_bytes < count)
    {
        int len = rt_ringbuffer_put(pipe->parent.fifo, &pbuf[write_bytes], count - write_bytes);
        if (len <= 0) break;

        write_bytes += len;
    }
    rt_mutex_release(&pipe->parent.lock);
    rt_event_send(pipe->read_event, 0x01);
    return write_bytes;
}

static rt_size_t rdbd_pipe_read(rt_device_t device, rt_off_t pos, void *buffer, rt_size_t count)
{
    uint8_t *pbuf;
    rt_size_t read_bytes = 0;
    rdbd_pipe_device_t pipe = (rdbd_pipe_device_t)device;
    rt_uint32_t event_set = 0;

    if (device == RT_NULL)
    {
        rt_set_errno(-EINVAL);
        return 0;
    }
    if (count == 0) return 0;

    pbuf = (uint8_t*)buffer;
    while(1)
    {
        if(rt_ringbuffer_data_len(pipe->parent.fifo) == 0 && rt_event_recv(pipe->read_event, 0x01, 
                        RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, rt_tick_from_millisecond(1000), &event_set) != RT_EOK)
        {
            continue;
        }
        rt_mutex_take(&(pipe->parent.lock), RT_WAITING_FOREVER);

        while (read_bytes < count)
        {
            int len = rt_ringbuffer_get(pipe->parent.fifo, &pbuf[read_bytes], count - read_bytes);
            if (len <= 0) break;

            read_bytes += len;
        }
        rt_mutex_release(&pipe->parent.lock);

        return read_bytes;
    }
}

rdbd_pipe_device_t rdbd_pipe_create(rdbd_device_t rdbd_device, const char * name, rt_uint8_t service_id, rt_uint8_t dir, int bufsz)
{
    rt_uint8_t i = 0;
    rdbd_pipe_device_t pipe;
    rt_thread_t tid;
    char * str = RT_NULL;
    #ifdef RT_USING_DEVICE_OPS
    struct rt_device_ops * pipe_ops = RT_NULL;
    pipe_ops = rt_malloc(struct rt_device_ops);
    if(pipe_ops == RT_NULL)
    {
        goto error;
    }
    #endif
    
    if(rdbd_device == RT_NULL)
    {
        LOG_E("rdbd device is null");dbg_here
    }
    rt_device_open((rt_device_t)rdbd_device, 0);
    if(!check_rdbd_pipe_max(rdbd_device))
    {
        return RT_NULL;
    }
    if(rdbd_device->rdbd_pipe_list == RT_NULL)
    {
        rdbd_device->rdbd_pipe_list = rt_calloc(1, 8 * rdbd_device->pipe_max_num);
        if(rdbd_device->rdbd_pipe_list == RT_NULL)
        {
            goto error;
        }
    }
    
    for(i = 0; i < rdbd_device->pipe_max_num; i++)
    {
        if(rdbd_pipe_get(rdbd_device, dir | i) == RT_NULL)
        {
            dir &= RDBD_PIPE_DIR_MASK;
            dir |= i;
            pipe = (rdbd_pipe_device_t)rt_pipe_create(name, bufsz);
            rt_device_unregister((rt_device_t)pipe);
            rt_mutex_detach(&(pipe->parent.lock));
            if(pipe == RT_NULL)
            {
                goto error;
            }
            pipe = rt_realloc(pipe, sizeof(struct rdbd_pipe_device));
            if(pipe == RT_NULL)
            {
                goto error;
            }
            pipe->service_id = service_id;
            pipe->addr = dir;
            pipe->msg = rt_malloc(pipe->parent.bufsz + 4);
            if(pipe->msg == RT_NULL)
            {
                goto error;
            }
            str = rt_malloc(RT_NAME_MAX);
            if(str == RT_NULL)
            {
                goto error;
            }
            rt_snprintf(str, RT_NAME_MAX, "rp%02X", dir);
            pipe->read_event = rt_event_create(str, RT_IPC_FLAG_FIFO);
            if(pipe->read_event == RT_NULL)
            {
                goto error;
            }
            pipe->rdbd_device = rdbd_device;
            if(dir & RDBD_PIPE_DEVICE_TO_HOST)
            {
                tid = rt_thread_create(str, rdbd_pipe_write_thread_entry, pipe, 1024, 8, 10);
                if(tid == RT_NULL)
                {
                    goto error;
                }
            }
            rt_free(str);
            str = RT_NULL;
            
            #ifdef RT_USING_DEVICE_OPS
            pipe_ops->init = pipe->parent.parent.ops->init;
            pipe_ops->open = pipe->parent.parent.ops->open;
            pipe_ops->close = pipe->parent.parent.ops->close;
            pipe_ops->read = rdbd_pipe_read;
            pipe_ops->write = rdbd_pipe_write;
            pipe_ops->control = pipe->parent.parent.ops->control;
            pipe->parent.parent.ops = pipe_ops;
            #else
            pipe->parent.parent.read = rdbd_pipe_read;
            pipe->parent.parent.write = rdbd_pipe_write;
            #endif
            rdbd_pipe_set(rdbd_device, pipe);
            rt_mutex_init(&(pipe->parent.lock), name, RT_IPC_FLAG_FIFO);
            rt_device_register((rt_device_t)pipe, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE);
            if(dir & RDBD_PIPE_DEVICE_TO_HOST)
            {
                rt_thread_startup(tid);
            }
            return pipe;
        }
    }
    LOG_E("pipe list is full");
    return RT_NULL;

error:
    #ifdef RT_USING_DEVICE_OPS
    if(pipe_ops)
    {
        rt_free(pipe_ops);
        pipe_ops = RT_NULL;
    }
    #endif
    if(pipe)
    {
        if(pipe->read_event)
        {
            rt_event_delete(pipe->read_event);
            pipe->read_event = RT_NULL;
        }
        if(pipe->msg)
        {
            rt_free(pipe->msg);
            pipe->msg = RT_NULL;
        }
        if(str)
        {
            rt_free(str);
            str = RT_NULL;
        }
        rt_free(pipe);
        pipe = RT_NULL;
    }
    LOG_E("no memory to create pipe");
    return RT_NULL;
}

rt_err_t rdbd_pipe_delete(rdbd_device_t rdbd_device, rdbd_pipe_device_t pipe)
{
    if(pipe == RT_NULL || rdbd_device == RT_NULL)
    {
        LOG_E("error args");
        return RT_ERROR;
    }
    rdbd_pipe_remove_in_list(rdbd_device, pipe->addr);
    if(pipe)
    {
        if(pipe->read_event)
        {
            rt_event_delete(pipe->read_event);
            pipe->read_event = RT_NULL;
        }
        if(pipe->msg)
        {
            rt_free(pipe->msg);
            pipe->msg = RT_NULL;
        }
        rt_pipe_delete(pipe->parent.parent.parent.name);
        pipe = RT_NULL;
    }
}

rdbd_pipe_device_t rdbd_pipe_find(rdbd_device_t rdbd_device, rt_uint8_t addr)
{
    return rdbd_pipe_get(rdbd_device, addr);
}




