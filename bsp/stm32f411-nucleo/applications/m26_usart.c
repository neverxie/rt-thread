/*
 * File      : m26_usart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-26     never        the first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "m26_usart.h"

/* 接收字节 */
uint8_t USART2RxByte;
/* 接收缓冲区 */
uint8_t USART2RxBuf[USART2_MAX_RECV_LEN];
/* 接收状态 */
uint16_t USART2RxSta = 0;
/*  接收超时计数 */
uint8_t USART2RcvTimeoutCnt = 0;
/* 接收定时开关 */
uint8_t USART2RcvTime = 0;

extern void m26_sms_text(void);
extern void m26_sms_pdu(void);

static rt_device_t usart_dev = RT_NULL;
static rt_sem_t    usart_sem = RT_NULL;

#define UART_RX_EVENT (1 << 0)
static struct rt_event event;

void rt_usart_send_chr(const rt_uint8_t c)
{
    rt_device_write(usart_dev, 0, &c, 1);
}

void rt_usart_send_str(const char *s, rt_size_t size)
{
    rt_device_write(usart_dev, 0, s, size);
}

void rt_usart_send_data(rt_uint8_t *data, rt_size_t size)
{
    rt_device_write(usart_dev, 0, data, size);
}

rt_err_t rt_usart_rxind(rt_device_t uart_dev, rt_size_t size)
{
    if (usart_sem != RT_NULL)
    {
        rt_sem_release(usart_sem);
    }

    return RT_EOK;
}

void m26_recv_thread(void *parameter)
{
    usart_sem = rt_sem_create("usart_sem", 0, RT_IPC_FLAG_FIFO);
    if (usart_sem == RT_NULL)
        return;

    usart_dev = rt_device_find("uart1");
    if (usart_dev == RT_NULL)
        return;
    rt_device_open(usart_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);

    rt_device_set_rx_indicate(usart_dev, rt_usart_rxind);

    while (1)
    {
        rt_sem_take(usart_sem, RT_WAITING_FOREVER);

        while (rt_device_read(usart_dev, 0, &USART2RxByte, 1) == 1)
        {
            /* begin, Write data to the buffer */
            if (USART2RxSta < USART2_MAX_RECV_LEN)
            {
                USART2RcvTime = 1;
                /* 重新更改定时器超时时间 */
                USART2RcvTimeoutCnt = 0;

                /* 保存接收到的值 */
                USART2RxBuf[USART2RxSta++] = USART2RxByte;
            }
            else
            {
                /* 接收完成 */
                USART2RxSta |= 1 << 15;
            }
            /* end */
        }
    }
}

void m26_send_thread(void *parameter)
{
    usart_dev = rt_device_find("uart1");
    if (usart_dev != RT_NULL)
    {
        rt_device_open(usart_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
    else
    {
        rt_kprintf("can't find usart_dev.\n");
        return;
    }
    
    while (1)
    {
        m26_sms_text();
    }
}

int rt_m26_thread_init(void)
{
    rt_thread_t tid;
    
    tid = rt_thread_create("send test",
                           m26_send_thread,
                           RT_NULL,
                           1024,
                           2,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    tid = rt_thread_create("recv test",
                           m26_recv_thread,
                           RT_NULL,
                           1024,
                           2,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    
    return 0;
}
INIT_APP_EXPORT(rt_m26_thread_init);
