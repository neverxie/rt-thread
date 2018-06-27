/*
 * File      : m26_usart.h
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
#ifndef __M26_USART_H__
#define __M26_USART_H__

#define USART2_MAX_RECV_LEN     64      // 最大接收缓存字节数
#define USART2_MAX_SEND_LEN     64      // 最大发送缓存字节数
#define USART2_RCV_TIMEOUT      10      // 接收超时值 10*1ms

void rt_usart_send_data(rt_uint8_t *data, rt_size_t size);

#endif
