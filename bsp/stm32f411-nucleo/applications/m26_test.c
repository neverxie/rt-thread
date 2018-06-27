/*
 * File      : m26_test.c
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
#include <rtthread.h>
#include "drv_usart.h"
#include <string.h>
#include "m26_usart.h"

extern uint8_t USART2RxByte;

extern uint8_t USART2RxBuf[USART2_MAX_RECV_LEN];
/* ����״̬ */
extern uint16_t USART2RxSta;
/*  ���ճ�ʱ���� */
extern uint8_t USART2RcvTimeoutCnt;
/* ���ն�ʱ���� */
extern uint8_t USART2RcvTime;



uint8_t M26_State = 0;

/*-------------------------------------------------------------------*/
/* �ڲ�����ԭ�� -----------------------------------------------------*/
void        m26_poweron(void);
uint8_t     m26_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime);
uint8_t    *m26_check_cmd(uint8_t *str);
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/**
 * @brief M26 ģ�鲦�Ų���
 */
void    m26_call_test(void)
{
    switch (M26_State)
    {
    case 0:
        m26_poweron();
        M26_State++;
        break;
    case 1:
        /* ����������� */
        m26_send_cmd("ATE1\r\n", "OK", 200);
        /* ��ʾ����ʶ�� */
        m26_send_cmd("AT+COLP=1\r\n", "OK", 200);
        /* �л���Ƶͨ��Ϊ����ͨ�� */
        m26_send_cmd("AT+QAUDCH=1\r\n", "OK", 200);
        /* ������������, ���� 10086 */
        m26_send_cmd("ATD10086;\r\n", "OK", 200);

        M26_State++;
        break;
    case 2:

        break;
    case 3:

        break;
    default:
        break;
    }
}

/**
  * @brief  ������������
  * @param
  * @retval
  */
void m26_ring_test(void)
{
    switch (M26_State)
    {
    case 0:
        m26_poweron();
        M26_State++;
        break;
    case 1:
        if (m26_check_cmd("RING"))
        {
            M26_State++;
        }

        break;
    case 2:
        /* ������������ */
        m26_send_cmd("ATA\r\n", "OK", 200);

        M26_State++;
        break;
    default:
        break;
    }
}

/**
  * @brief ʹ�� TEXT ģʽ���Ͷ���
  */
void m26_sms_text(void)
{
    rt_uint8_t  res;
    rt_uint8_t buf[4] = {0x67, 0x1A, 0x0D, 0x0A};
    
    switch (M26_State)
    {
    case 0:
        m26_poweron();
        M26_State++;
        break;
    case 1:
        res = m26_send_cmd("AT+CMGF=1\r\n", "OK", 200);
        rt_kprintf("%d\n", res);
        res = m26_send_cmd("AT+CSCS=\"GSM\"\r\n", "OK", 200);
        rt_kprintf("%d\n", res);
        res = m26_send_cmd("AT+CMGS=\"13676243871\"\r\n", ">", 200);
        rt_kprintf("%d\n", res);
        res = m26_send_cmd(buf, "OK", 200);
        rt_kprintf("%d\n", res);

        M26_State++;
        break;
    case 2:
        break;
    default:
        break;
    }
}

/**
  * @brief ʹ�� PDU ģʽ���Ͷ���
  */
void m26_sms_pdu(void)
{
    rt_uint8_t res;
    switch (M26_State)
    {
    case 0:
        m26_poweron();
        M26_State++;
        break;
    case 1:
        res = m26_send_cmd("AT+CSCS=\"GSM\"\r\n", "OK", 200);
        rt_kprintf("%d\n", res);
        res = m26_send_cmd("AT+CMGF=0\r\n", "OK", 200);
        rt_kprintf("%d\n", res);
        res = m26_send_cmd("AT+CSCS=\"UCS2\"\r\n", "OK", 200);
        rt_kprintf("%d\n", res);
        res = m26_send_cmd("AT+CMGS=39\r\n", ">", 200);
        rt_kprintf("%d\n", res);
        res = m26_send_cmd("0891683108501705F011000D91688152613201F800080018\
						  670968A660F376844EBAFF0C547D90FD662F82E67684FF01\r\n", "OK", 200);

        M26_State++;
        break;
    case 2:
        break;
    default:
        break;
    }
}


/*-------------------------------------------------------------------*/
/* �ڲ�����ʵ�� -----------------------------------------------------*/

/**
  * @brief  M26 ģ�鿪��
  */
void m26_poweron(void)
{
    /* �ֶ����� */
//  M26_ON_L();
//  M26_PWR_L();
//  HAL_Delay(5000);
//  M26_PWR_H();
//  HAL_Delay(1000);
//  M26_ON_H();
//  HAL_Delay(1000);
//  M26_ON_L();
//  HAL_Delay(5000);

    /* �ϵ�ȴ�ģ�������� */
    rt_thread_delay(100);
}

/**
  * @brief  �� M26 ģ�鷢������
  * @param  cmd         ���͵�����
  * @param  ack         �ڴ���Ӧ����
  * @param  waittime    �ȴ�ʱ��(��λ:10ms)
  * @retval 0 ���ͳɹ�
            1 ����ʧ��
  */
uint8_t  m26_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 0;

    /* ������һ������֮ǰ�������״̬ */
    USART2RxSta = 0;

    rt_usart_send_data(cmd, strlen((const char *)cmd));

    if (ack && waittime)
    {
        while (--waittime)
        {
            rt_thread_delay(10);

            /* ���յ�һ֡���� */
            if (USART2RxSta & 0x8000)
            {
                /* ���յ��ڴ���Ӧ���� */
                if (m26_check_cmd(ack)) break;
                USART2RxSta = 0;
            }
        }

        if (waittime == 0)
            res = 1;
    }

    return res;
}

/**
  * @brief  �� M26 ģ�鷢������󣬼����յ���Ӧ��
  * @param  str     �ڴ���Ӧ����
  * @retval 0       û�еõ��ڴ���Ӧ����
            ����    �ڴ�Ӧ������λ��
  */
uint8_t *m26_check_cmd(uint8_t *str)
{
    char *strx = 0;

    /* ���յ�һ������ */
    if (USART2RxSta & 0x8000)
    {
        /* ��ӽ����� */
        USART2RxBuf[USART2RxSta & 0x7FFF] = 0;

        strx = strstr((const char *)USART2RxBuf, (const char *)str);
    }

    return (uint8_t *)strx;
}

