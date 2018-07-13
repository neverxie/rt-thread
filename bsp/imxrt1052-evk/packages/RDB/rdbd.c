#include <rdbd.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME  "[RDBD]"
#define DBG_LEVEL         DBG_WARNING
#define DBG_COLOR
#include <rtdbg.h>

static rt_uint8_t * rdbd_get_service_list(rdbd_device_t rdbd_device)
{
    int i, j;
    rt_uint8_t service_list_len = 1;
    int pipe_list_len = rdbd_device->pipe_max_num * 2;
    rt_uint8_t * service_list = rt_calloc(1, pipe_list_len + 1);
    for(i = 0; i < pipe_list_len; i++)
    {
        if(rdbd_device->rdbd_pipe_list[i] != RT_NULL)
        {
            for(j = 1; j < service_list_len + 1; j++)
            {
                if(rdbd_device->rdbd_pipe_list[i]->service_id == service_list[j])
                {
                    break;
                }
                if(j == service_list_len)
                {
                    service_list[j] = rdbd_device->rdbd_pipe_list[i]->service_id;
                    service_list_len ++;
                    break;
                }
            }
        }
    }
    service_list[0] = service_list_len;
    return rt_realloc(service_list, service_list_len);
}

static int rdbd_get_pipe_addr(rdbd_device_t rdbd_device, rt_uint8_t service_id, rt_uint8_t dir)
{
    int i;                                 
    rt_kprintf("find pipe %02X, serviceid %02X\n", dir, service_id);
    for(i = rdbd_device->pipe_max_num * (dir >> 7); i < rdbd_device->pipe_max_num * (dir >> 7) + rdbd_device->pipe_max_num; i++)
    {
        rt_kprintf("%d , 0x%02X\n",i, rdbd_device->rdbd_pipe_list[i]->service_id);
        if(rdbd_device->rdbd_pipe_list[i]->service_id == service_id)
        {
            return rdbd_device->rdbd_pipe_list[i]->addr;
        }
    }
    return -1;
}

static rt_err_t rdbd_msg_receiver(rdbd_device_t rdbd_device, const char * raw_data)
{
    rdbd_pipe_device_t pipe = rdbd_pipe_find(rdbd_device, RDBD_MSG(raw_data)->pipe_address);
    if(pipe == RT_NULL)
    {
        LOG_E("Not find pipe 0x%02X", RDBD_MSG(raw_data)->pipe_address);
        return RT_ERROR;
    }
    if(pipe->addr & 0x80)
    {
        LOG_E("Pipe dir error", pipe->addr);
        return RT_ERROR;
    }
    rt_device_open((rt_device_t)pipe, 0);
    rt_device_write((rt_device_t)pipe, 0, RDBD_MSG(raw_data)->msg, RDBD_MSG(raw_data)->msg_len);
    rt_device_close((rt_device_t)pipe);
    return RT_EOK;
}
static rt_err_t rdbd_open(rt_device_t device, rt_uint16_t oflag)
{
    return RT_EOK;
}

rdbd_device_t rdbd_device_register(const char * name, rt_size_t (*write)(rt_device_t, rt_off_t, const void *, rt_size_t), rt_uint8_t max_pipe)
{
    rdbd_device_t rdbd = rt_calloc(1,sizeof(struct rdbd_device));
    rdbd->pipe_max_num = max_pipe;
    rdbd->parent.write = write;
    rdbd->parent.type = RT_Device_Class_Miscellaneous;
    rdbd->parent.open = rdbd_open;
    rdbd->get_service_list = rdbd_get_service_list;
    rdbd->get_pipe_addr = rdbd_get_pipe_addr;
    rdbd->msg_receiver = rdbd_msg_receiver;
    rt_device_register(&(rdbd->parent), name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_WRONLY);
    return rdbd;
}