/*
 * File      : tcpdump.c
 * This is file that captures the IP message based on the RT-Thread
 * and saves in the file system.
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
 * 2018-06-21     never        the first version
 */
 
#include <rtthread.h>
#include <dfs_posix.h>
#include <rtdef.h>
#include "tcpdump.h"
#include "netif/ethernetif.h"

//static struct rt_mailbox mb;
extern rt_mailbox_t tcpdump_mb;
rt_uint8_t ip[74] = 
{
    0x00, 0x04, 0x9f, 0x05, 0x44, 0xe5, 0xe0, 0xd5, 0x5e, 0x71, 0x99, 0x95, 0x08, 0x00, 0x45, 0x00, 
    0x00, 0x3c, 0x28, 0x6a, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x6d, 0xc0, 0xa8,
    0x01, 0x1e, 0x08, 0x00, 0x4d, 0x1a, 0x00, 0x01, 0x00, 0x41, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69    
};

rt_pcap_file_t *rt_creat_pcap_file(rt_uint8_t *ip, rt_uint16_t len)
{
    rt_pcap_file_t *file = RT_NULL;
    
    file = rt_malloc(sizeof(struct rt_pcap_file) + len);
    if (file == RT_NULL)
        return RT_NULL;
    file->ip_mess = (rt_uint8_t *)file + sizeof(struct rt_pcap_file);
    
    file->p_f_h.magic = 0xa1b2c3d4;
    file->p_f_h.version_major = 0x200;
    file->p_f_h.version_minor = 0x400;
    file->p_f_h.thiszone = 0;
    file->p_f_h.sigfigs = 0;
    file->p_f_h.snaplen = 0xff;
    file->p_f_h.linktype = 1;
    
    file->p_h.ts.tv_sec = 0;   //  os_tick
    file->p_h.ts.tv_msec = 0;  //  os_tick
    file->p_h.caplen = 0x45;   //  ip len
    file->p_h.len = 0x45;      //
    
    rt_memcpy(file->ip_mess, ip, len);
    file->ip_len = len;
    
    return file;
}

int rt_del_pcap_file(rt_pcap_file_t *file)
{
    if (file == RT_NULL)
        return -1;    
    rt_free(file);
    return 0;
}

void rt_save_pcap_file(rt_pcap_file_t *file)
{
    int fd;
    
    fd = open("/1.pcap", O_WRONLY | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        rt_kprintf("open file for write failed\n"); 
        return;
    }
    
    write(fd, file, sizeof(struct rt_pcap_file) - PCAP_HEADER_LENGTH);
    close(fd);
    
    fd = open("/1.pcap", O_WRONLY | O_CREAT | O_APPEND, 0);
    write(fd, file->ip_mess, file->ip_len);
    close(fd);
    
    rt_kprintf("read/write done.\n");
}


void rt_pbufcpy(struct pbuf *pbuf)
{
    struct pdata *pdata;
    
    rt_kprintf("pbuf->totallen:%d\n", pbuf->tot_len);
    rt_kprintf("pbuf->len:%d\n", pbuf->len);
    pdata = rt_malloc(sizeof(struct pdata) + pbuf->tot_len);
    if (pdata == RT_NULL)
        return;
    pdata->buf = (rt_uint8_t *)pdata + sizeof(struct pdata);
    pdata->len = pbuf->tot_len;
    while (pbuf)
    {
        rt_memcpy(pdata->buf, (rt_uint8_t *)pbuf->payload, pbuf->tot_len);
        pdata->buf += pbuf->len;
        pbuf = pbuf->next;
    }
       int i,j;
                for (j = 0; j < pdata->len; j++)
            {
                if ((i % 8) == 0)
                {
                    rt_kprintf("  ");  
                }
                if ((i % 16) == 0)
                {
                    rt_kprintf("\r\n");
                }
                rt_kprintf("%02x ", *(pdata->buf));

                i++;
                pdata->buf++;
            }
    rt_mb_send(tcpdump_mb, (rt_uint32_t)pdata);
}

//void test(void)
//{
//    rt_pcap_file_t *p = RT_NULL;
//    rt_uint32_t tick_start;
//    struct pbuf *p1;
//    
//   
//    p = rt_creat_pcap_file(ip, sizeof(ip));
//    rt_save_pcap_file(p);
//    rt_del_pcap_file(p);
//}
//MSH_CMD_EXPORT(test, pcap test);

void rt_tcp_dump_thread(void *param)
{
    struct pdata *pdata;
    int i = 0,j=0;
    rt_uint32_t mbval;
    
    while(1)
    {           
        if (rt_mb_recv(tcpdump_mb, &mbval, RT_WAITING_FOREVER) == RT_EOK)
        {
            pdata = (struct pdata *)mbval;
            rt_kprintf("mb_recv->len:%d\n", pdata->len);
            for (j = 0; j < pdata->len; j++)
            {
                if ((i % 8) == 0)
                {
                    rt_kprintf("  ");  
                }
                if ((i % 16) == 0)
                {
                    rt_kprintf("\r\n");
                }
                rt_kprintf("%02x ", *(pdata->buf));

                i++;
                pdata->buf++;
            }
            rt_kprintf("\n\n");
        }
    }
}





