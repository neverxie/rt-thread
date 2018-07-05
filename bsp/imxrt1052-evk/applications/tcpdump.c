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
#include <stdio.h>

 #define TCPDUMP_DEBUG

#define TCPDUMP_MAX_MSG      (10)
#define TCPDUMP_DEFAULT_NAME ("75.pcap")

struct tcpdump_msg
{
    void *pbuf;
    rt_uint32_t sec;
    rt_uint32_t msec;
};

#define PACP_FILE_HEADER_CREEATE(_head)     \
    do {                                    \
    (_head)->magic = 0xa1b2c3d4;            \
    (_head)->version_major = 0x200;         \
    (_head)->version_minor = 0x400;         \
    (_head)->thiszone = 0;                  \
    (_head)->sigfigs = 0;                   \
    (_head)->snaplen = 0xff;                \
    (_head)->linktype = 1;                  \
    } while (0)

#define PACP_PKTHDR_CREEATE(_head, _msg)                            \
    do {                                                            \
    (_head)->ts.tv_sec = (_msg)->sec;                               \
    (_head)->ts.tv_msec = (_msg)->msec;                             \
    (_head)->caplen = ((struct pbuf *)((_msg)->pbuf))->tot_len;     \
    (_head)->len = ((struct pbuf *)((_msg)->pbuf))->tot_len;        \
    } while (0) 

static rt_mq_t tcpdump_mq; 
static struct netif *netif;
static netif_linkoutput_fn link_output;
static rt_uint32_t tcpdump_flag;
static char *filename;

static netif_input_fn input;

static int fd = -1;    

rt_uint8_t ip[74] =
{
    0x00, 0x04, 0x9f, 0x05, 0x44, 0xe5, 0xe0, 0xd5, 0x5e, 0x71, 0x99, 0x95, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x3c, 0x28, 0x6a, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x6d, 0xc0, 0xa8,
    0x01, 0x1e, 0x08, 0x00, 0x4d, 0x1a, 0x00, 0x01, 0x00, 0x41, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69
};
    
#ifdef TCPDUMP_DEBUG
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const rt_uint8_t *ptr, rt_size_t buflen)
{
    unsigned char *buf = (unsigned char*)ptr;
    int i, j;

    for (i=0; i<buflen; i+=16)
    {
        rt_kprintf("%08X: ", i);

        for (j=0; j<16; j++)
            if (i+j < buflen)
                rt_kprintf("%02X ", buf[i+j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j=0; j<16; j++)
            if (i+j < buflen)
                rt_kprintf("%c", __is_print(buf[i+j]) ? buf[i+j] : '.');
        rt_kprintf("\n");
    }
}

//static void rt_tcpdump_file_dump(rt_pcap_file_t *file)
//{
//    if (file == RT_NULL)
//    {
//        rt_kprintf("file is null\n");
//        return;
//    }

//    rt_kprintf("------------------------file header---------------------\n");
//    rt_kprintf("magi       major  minor  zone   sigfigs  snaplen linktype\n");
//    rt_kprintf("0x%08x ",file->p_f_h.magic);
//    rt_kprintf("0x%04x ", file->p_f_h.version_major);
//    rt_kprintf("0x%04x ", file->p_f_h.version_minor);
//    rt_kprintf("0x%04x ", file->p_f_h.thiszone);
//    rt_kprintf("0x%04x   ", file->p_f_h.sigfigs);
//    rt_kprintf("0x%04x  ", file->p_f_h.snaplen);
//    rt_kprintf("0x%04x\n\n", file->p_f_h.linktype);

//    rt_kprintf("       msec         sec         len      caplen \n");
//    rt_kprintf("%11d ", file->p_h.ts.tv_msec);
//    rt_kprintf("%11d ", file->p_h.ts.tv_sec);
//    rt_kprintf("%11d ", file->p_h.len);
//    rt_kprintf("%11d \n\n", file->p_h.caplen);

//    dump_hex(file->ip_mess, file->ip_len);
//    rt_kprintf("--------------------------end---------------------------\n");
//}
#endif

static err_t _netif_linkoutput(struct netif *netif, struct pbuf *p)
{
    struct tcpdump_msg msg;
    rt_uint32_t tick = rt_tick_get();
    
    if (p != RT_NULL)
    {
        pbuf_ref(p);
        msg.pbuf = p;
        msg.sec = tick / 1000;
        msg.msec = tick % 1000;
        if (rt_mq_send(tcpdump_mq, &msg, sizeof(msg)) != RT_EOK)
        {
            pbuf_free(p);
        }
    }

    return link_output(netif, p);
}

static err_t _netif_input(struct pbuf *p, struct netif *inp)
{
    struct tcpdump_msg msg;
    rt_uint32_t tick = rt_tick_get();
    
    if (p != RT_NULL)
    {
        pbuf_ref(p);
        msg.pbuf = p;
        msg.sec = tick / 1000;
        msg.msec = tick % 1000;
        if (rt_mq_send(tcpdump_mq, &msg, sizeof(msg)) != RT_EOK)
        {
            pbuf_free(p);
        }
    }
    
    return input(p, inp);
}

static rt_err_t rt_tcpdump_pcap_file_write(void *buf, int len)
{
    int length;
    static int count = 0;

    if (filename == RT_NULL)
    {
        rt_kprintf("file name is null\n");
        return -RT_ERROR;
    }
    
    if (fd < 0)
    {
        fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0);
        if (fd < 0)
        {
            rt_kprintf("open file for write failed\n");
            return -RT_ERROR;
        }
    }
    
    length = write(fd, buf, len);
    if (length != len)
    {
        rt_kprintf("write data failed, length: %d\n", length);
        close(fd);
        return -RT_ERROR;
    }
    count += length;
    
    if (count > 1024)
    {
        count = 0;
        rt_kprintf("tcpdump file write done and auto save!\n");
        close(fd);
        rt_kprintf("after close, fd: %d\n", fd);
    }    
    
    return RT_EOK;
}

static void rt_header_to_u8(struct rt_pcap_file_header *file_header, rt_uint8_t *buf)
{
    union rt_u32_data u32_data;
    union rt_u16_data u16_data;
    int k, i, j;

    rt_uint16_t *p16 = (rt_uint16_t *)file_header + 2;
    rt_uint32_t *p32 = (rt_uint32_t *)file_header + 2;
    
    /* struct rt_pcap_file_header. magic */
    u32_data.u32byte = 0;
    u32_data.u32byte = file_header->magic;
    for (k = 3; k != -1; k--)
        buf[k] = u32_data.a[k];
    
    /* struct rt_pcap_file_header. version_major & version_minor*/
    for (i = 0, j = 4; i < 2; i++)
    {
        u16_data.u16byte = 0;
        u16_data.u16byte = *(p16 + i);
        for (k = 1; k != -1; k--)
        {
            buf[k + j] = u16_data.a[k];
        }
        j += 2;
    }

    /* struct rt_pcap_header.thiszone ~ linktype */
    for (i = 0, j = 8; i < 4; i++)
    {
        u32_data.u32byte = 0;
        u32_data.u32byte = *(p32 + i);
        for (k = 3; k != -1; k--)
        {
            buf[k + j] = u32_data.a[k];
        }
        j += 4;
    }
}

static void rt_pkthdr_to_u8(struct rt_pkthdr *pkthdr, rt_uint8_t *buf)
{
    union rt_u32_data u32_data;
    union rt_u16_data u16_data;
    int k, i, j;

    /* struct rt_pcap_header */
    for (i = 0, j = 0; i < 4; i++)
    {
        u32_data.u32byte = 0;
        u32_data.u32byte = *((rt_uint32_t *)pkthdr + i);
        for (k = 3; k != -1; k--)
        {
            buf[k + j] = u32_data.a[k];
        }
        j += 4;
    }
}

static void rt_tcpdump_pcap_file_init(void)
{
    struct rt_pcap_file_header file_header;
    rt_uint8_t buf[24] = {0};
    
    PACP_FILE_HEADER_CREEATE(&file_header);

//    rt_header_to_u8(&file_header, buf);
//    dump_hex(buf, 24);

    rt_tcpdump_pcap_file_write(&file_header, sizeof(file_header));
}    

rt_uint8_t r = 1;
static void rt_tcpdump_thread_entry(void *param)
{
    struct pbuf *pbuf, *p, *q;
    struct tcpdump_msg msg;
    struct rt_pkthdr pkthdr;
    rt_uint8_t buf[16] = {0};
    rt_uint8_t *ip_mess , *ptr;
    rt_uint8_t ip_len;
    
    while (1)
    {
        if (rt_mq_recv(tcpdump_mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
        {
            pbuf = msg.pbuf;
            p = pbuf;
            ip_len = p->tot_len;
            if (r == 1)
            {
                r = 0;
                rt_tcpdump_pcap_file_init();
            }
            
            PACP_PKTHDR_CREEATE(&pkthdr, &msg);
            rt_tcpdump_pcap_file_write(&pkthdr, sizeof(pkthdr));
//            rt_pkthdr_to_u8(&pkthdr, buf);
//            dump_hex(buf, 16);            
            
//            ptr = rt_malloc(ip_len);
//            if (ptr == RT_NULL)
//            {
//                rt_kprintf("error\n");
//                return;
//            }    
            
            ip_mess = p->payload;
       
            while (p)
            {
//                rt_memcpy(ptr, ip_mess, p->len);
                rt_tcpdump_pcap_file_write(ip_mess, p->len);
                ip_mess += p->len;
                p = p->next;
            }
            pbuf_free(pbuf);
//            dump_hex(ptr, ip_len);
//            rt_free(ptr);
            
//#ifdef TCPDUMP_DEBUG
//            rt_tcpdump_file_dump(dump_file);
//#endif
//            rt_tcpdump_pcap_file_delete(dump_file);
        }
        else
        {
            rt_kprintf("tcp dump thread exit\n");
            return;
        }
    }
}

//void rt_tcpdump_write_enable(void)
//{
//    tcpdump_flag |= TCPDUMP_WRITE_FLAG;
//}

//void rt_tcpdump_write_disable(void)
//{
//    tcpdump_flag &= ~TCPDUMP_WRITE_FLAG;
//}

void rt_tcpdump_set_filename(const char *name)
{
    if (filename != RT_NULL)
    {
        rt_free(filename);
    }

    filename = rt_strdup(name);
}

int rt_tcpdump_init(void)
{
    struct eth_device *device;
    rt_thread_t tid;
    rt_base_t level;

    if (netif != RT_NULL)
    {
        return RT_EOK;
    }
    device = (struct eth_device *)rt_device_find("e0");
    if (device == RT_NULL)
    {
        rt_kprintf("device not find\n");
        return -RT_ERROR;
    }
    if ((device->netif == RT_NULL) || (device->netif->linkoutput == RT_NULL))
    {
        rt_kprintf("this device not eth\n");
        return -RT_ERROR;
    }
    tcpdump_mq = rt_mq_create("tcpdump", sizeof(struct tcpdump_msg), TCPDUMP_MAX_MSG, RT_IPC_FLAG_FIFO);
    if (tcpdump_mq == RT_NULL)
    {
        rt_kprintf("tcp dump mp create fail\n");
        return -RT_ERROR;
    }
    tid = rt_thread_create("tcp_dump", rt_tcpdump_thread_entry, RT_NULL, 2048, 10, 10);
    if (tid == RT_NULL)
    {
        rt_mq_delete(tcpdump_mq);
        tcpdump_mq = RT_NULL;
        rt_kprintf("tcp dump thread create fail\n");
        return -RT_ERROR;
    }
    
    if (filename == RT_NULL)
    {
        filename = rt_strdup(TCPDUMP_DEFAULT_NAME);
    }

    netif = device->netif;
    level = rt_hw_interrupt_disable();
    link_output = netif->linkoutput;
    netif->linkoutput = _netif_linkoutput;
    
    input = netif->input;
    netif->input = _netif_input;
    
    rt_hw_interrupt_enable(level);
    rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(rt_tcpdump_init);

void rt_tcpdump_deinit(void)
{
    rt_base_t level;

    if (netif == RT_NULL)
    {
        return;
    }
    level = rt_hw_interrupt_disable();
    netif->linkoutput = link_output;
    netif = RT_NULL;
    tcpdump_flag = 0;
    rt_hw_interrupt_enable(level);
    rt_mq_delete(tcpdump_mq);
    tcpdump_mq = RT_NULL;
}

int tcpdump_name(void)
{

}
MSH_CMD_EXPORT(tcpdump_name, name);

void tcpdump_get(void)
{
    
}
MSH_CMD_EXPORT(tcpdump_get, get);

void tcpdump_save(void)
{
    close(fd);
}
MSH_CMD_EXPORT(tcpdump_save, save);

//msh opt

