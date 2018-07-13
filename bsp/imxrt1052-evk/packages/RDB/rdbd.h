/*
 * File      : winusb.h
 * COPYRIGHT (C) 2008 - 2016, RT-Thread Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-11-16     ZYH          first version
 */
#ifndef __RDBD_H__
#define __RDBD_H__
#include <rtthread.h>
#include <rtdevice.h>
#ifndef PKGS_RDBD_PIPE_MAX_NUM
#define RDBD_PIPE_MAX_NUM 0x7f
#else
#define RDBD_PIPE_MAX_NUM PKGS_RDBD_PIPE_MAX_NUM
#endif

#define RDBD_SERVICE_ID_NONE         (0U)
#define RDBD_SERVICE_ID_FILE         (1U)
#define RDBD_SERVICE_ID_FILE_C       (2U)
#define RDBD_SERVICE_ID_SHELL        (3U)
#define RDBD_SERVICE_ID_SHELL_C      (4U)
#define RDBD_SERVICE_ID_RTI          (5U)
#define RDBD_SERVICE_ID_RTI_C        (6U)
#define RDBD_SERVICE_ID_TCP_DUMP     (7U)
#define RDBD_SERVICE_ID_TCP_DUMP_C   (8U)
#define RDBD_SERVICE_ID_OTA          (9U)
#define RDBD_SERVICE_ID_OTA_C        (10U)
#define RDBD_SERVICE_ID_GENERAL      (11U)


#define RDBD_PIPE_DEVICE_TO_HOST (0x80)
#define RDBD_PIPE_HOST_TO_DVICE  (0x00)
#define RDBD_PIPE_DIR_MASK       (0x80)
typedef struct rdbd_device * rdbd_device_t;
typedef struct rdbd_pipe_msg * rdbd_pipe_msg_t;
typedef struct rdbd_pipe_device * rdbd_pipe_device_t;

struct rdbd_pipe_msg
{
    rt_uint8_t pipe_address;
    rt_uint32_t msg_len : 24;
    rt_uint8_t msg[RT_UINT16_MAX];
};

struct rdbd_pipe_device
{
    struct rt_pipe_device parent;
    rt_uint8_t service_id;
    rt_uint8_t addr;
    rt_event_t read_event;
    rdbd_pipe_msg_t msg;
    rdbd_device_t rdbd_device;
};

struct rdbd_device
{
    struct rt_device parent;
    rt_uint8_t *  (*get_service_list)(rdbd_device_t rdbd_device);
    int (*get_pipe_addr)(rdbd_device_t rdbd_device, rt_uint8_t service_id, rt_uint8_t dir);
    rt_err_t (*msg_receiver)(rdbd_device_t rdbd_device, const char * raw_data);
    rdbd_pipe_device_t *rdbd_pipe_list;
    rt_uint8_t pipe_max_num;
};
extern rdbd_device_t rdbd_device_register(const char * name, rt_size_t (*write)(rt_device_t, rt_off_t, const void *, rt_size_t), rt_uint8_t max_pipe);
extern rdbd_pipe_device_t rdbd_pipe_create(rdbd_device_t rdbd_device, const char * name, rt_uint8_t service_id, rt_uint8_t dir, int bufsz);
extern rt_err_t rdbd_pipe_delete(rdbd_device_t rdbd_device, rdbd_pipe_device_t pipe);
extern rdbd_pipe_device_t rdbd_pipe_find(rdbd_device_t rdbd_device, rt_uint8_t addr);



#define RDBD_MSG(x)      ((rdbd_pipe_msg_t)x)
#define RDBD_RAW_MSG(x)  ((rt_uint8_t *)x)
#define RDBD_MSG_LEN(x)  (x->msg_len + 4)


#endif
