#include "rtthread.h"
#include "rdbd.h"
#include "rthw.h"
#define PKGS_USING_USB_RDBD
#define DBG_ENABLE
#define DBG_SECTION_NAME  "[RDBD]"
#define DBG_LEVEL         DBG_WARNING
#define DBG_COLOR
#include <rtdbg.h>

#ifdef PKGS_USING_USB_RDBD
    #define RDBD_DEVICE_NAME "urdbd"
#else
    #define RDBD_DEVICE_NAME "rdbd"
#endif

#define SHELL_CMD_CONNECT    0x01
#define SHELL_CMD_DISCONNECT 0x02

#define SERVICE_ID RDBD_SERVICE_ID_SHELL
static struct rt_serial_device rdbd_shell_serial;
static rt_bool_t rdbd_shell_connected;
static rt_device_t h_to_d_pipe;
static rt_device_t d_to_h_pipe;
static rt_device_t control_pipe;
static rt_device_t control_resp_pipe;

static struct rt_ringbuffer rdbd_shell_rx_ringbuffer;
static struct rt_event rdbd_shell_tx_event;
static rt_uint32_t rdbd_shell_rx_rbuffer[512];

extern int set_finsh_to_rdbd(void);
extern int set_finsh_back(void);

static void rdbd_shell_cmd_exec(rt_uint8_t cmd)
{
    switch(cmd)
    {
    case SHELL_CMD_CONNECT:
        rdbd_shell_connected = RT_TRUE;
        set_finsh_to_rdbd();
        break;
    case SHELL_CMD_DISCONNECT:
        rdbd_shell_connected = RT_FALSE;
        set_finsh_back();
        break;
    }
}

int rdbd_exit(void)
{
    rt_uint8_t cmd = 0xff;
    if(rdbd_shell_connected)
    {
        rt_device_write(control_resp_pipe, 0, &cmd, 1);
        rdbd_shell_connected = RT_FALSE;
        set_finsh_back();
    }
    return 0;
}
MSH_CMD_EXPORT(rdbd_exit, disconnect rdbd shell);

static void rdbd_shell_cmd_thread_entry(void *arg)
{
    uint8_t cmd;
    rt_device_open(control_pipe, 0);
    while (1)
    {
        if (rt_device_read(control_pipe, 0, &cmd, 1))
        {
            rdbd_shell_cmd_exec(cmd);
        }
    }
}

static void rdbd_shell_read_thread_entry(void *arg)
{
    rt_size_t read_size = 0;
    rdbd_pipe_device_t pipe = (rdbd_pipe_device_t)h_to_d_pipe;
    rt_base_t level;
    rt_device_open(h_to_d_pipe, 0);
    while (1)
    {
        read_size = rt_device_read(h_to_d_pipe, 0,  pipe->msg->msg, pipe->parent.bufsz);
        if (read_size)
        {
            if ((rdbd_shell_serial.parent.flag & RT_DEVICE_FLAG_ACTIVATED) && (rdbd_shell_serial.parent.open_flag & RT_DEVICE_OFLAG_OPEN))
            {
                /* receive data from pipe */
                level = rt_hw_interrupt_disable();

                rt_ringbuffer_put(&rdbd_shell_rx_ringbuffer, pipe->msg->msg, read_size);
                rt_hw_interrupt_enable(level);

                /* notify receive data */
                rt_hw_serial_isr(&rdbd_shell_serial, RT_SERIAL_EVENT_RX_IND);
            }
        }
    }
}

static rt_err_t rdbd_shell_configure(struct rt_serial_device *serial,
                                    struct serial_configure *cfg)
{
    ((void)serial);
    ((void)cfg);
    return RT_EOK;
}

static rt_err_t rdbd_shell_control(struct rt_serial_device *serial,
                                  int cmd, void *arg)
{
    ((void)serial);
    ((void)cmd);
    ((void)arg);
    return RT_EOK;
}

static int rdbd_shell_getc(struct rt_serial_device *serial)
{
    int result;
    rt_uint8_t ch;
    rt_uint32_t level;
    result = -1;
    ((void)serial);
    level = rt_hw_interrupt_disable();

    if (rt_ringbuffer_getchar(&rdbd_shell_rx_ringbuffer, &ch) != 0)
    {
        result = ch;
    }
    rt_hw_interrupt_enable(level);
    return result;
}

static rt_size_t rdbd_shell_tx(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    ((void)serial);
    if (rdbd_shell_connected)
    {
        return rt_device_write(d_to_h_pipe, 0, buf, size);
    }
    return 0;
}
static int rdbd_shell_putc(struct rt_serial_device *serial, char c)
{
    ((void)serial);
    if (rdbd_shell_connected)
    {
        return rt_device_write(d_to_h_pipe, 0, &c, 1);
    }
    return 0;
}

static const struct rt_uart_ops rdbd_shell_ops =
{
    rdbd_shell_configure,
    rdbd_shell_control,
    rdbd_shell_putc,
    rdbd_shell_getc,
    rdbd_shell_tx
};


int rdbd_shell_init(void)
{
    rt_thread_t tid;
    rdbd_device_t rdbd = (rdbd_device_t)rt_device_find(RDBD_DEVICE_NAME);
    if (rdbd == RT_NULL)
    {
        LOG_E("rdbd device [%s] not found", RDBD_DEVICE_NAME);
        return -1;
    }
    d_to_h_pipe = (rt_device_t)rdbd_pipe_create(rdbd, "shin", RDBD_SERVICE_ID_SHELL, RDBD_PIPE_DEVICE_TO_HOST, 128);
    if (d_to_h_pipe == RT_NULL)
    {
        LOG_E("pipe device shin create failed");
        return -1;
    }
    h_to_d_pipe = (rt_device_t)rdbd_pipe_create(rdbd, "shout", RDBD_SERVICE_ID_SHELL, RDBD_PIPE_HOST_TO_DVICE, 128);
    if (h_to_d_pipe == RT_NULL)
    {
        LOG_E("pipe device shout create failed");
        return -1;
    }
    control_pipe = (rt_device_t)rdbd_pipe_create(rdbd, "shc", RDBD_SERVICE_ID_SHELL_C, RDBD_PIPE_HOST_TO_DVICE, 8);
    if (control_pipe == RT_NULL)
    {
        LOG_E("pipe device shc create failed");
        return -1;
    }
    control_resp_pipe = (rt_device_t)rdbd_pipe_create(rdbd, "shcr", RDBD_SERVICE_ID_SHELL_C, RDBD_PIPE_DEVICE_TO_HOST, 8);
    if (control_resp_pipe == RT_NULL)
    {
        LOG_E("pipe device shcr create failed");
        return -1;
    }

    rt_ringbuffer_init(&rdbd_shell_rx_ringbuffer, (rt_uint8_t *)rdbd_shell_rx_rbuffer, sizeof(rdbd_shell_rx_rbuffer));

    rt_memset(&rdbd_shell_serial, 0, sizeof(rdbd_shell_serial));

    rdbd_shell_serial.ops = &rdbd_shell_ops;
    
    tid = rt_thread_create("shc", rdbd_shell_cmd_thread_entry, RT_NULL, 512, 8, 10);
    if (tid == RT_NULL)
    {
        LOG_E("shc thread create failed");
        return -1;
    }
    rt_thread_startup(tid);

    tid = rt_thread_create("shout", rdbd_shell_read_thread_entry, RT_NULL, 512, 8, 10);
    if (tid == RT_NULL)
    {
        LOG_E("shout thread create failed");
        return -1;
    }
    rt_thread_startup(tid);

    rt_hw_serial_register(&rdbd_shell_serial, "rdbdsh", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX, RT_NULL);
    return 0;
}
INIT_COMPONENT_EXPORT(rdbd_shell_init);
