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
#define TCPDUMP_DEFAULT_NAME ("tcpdump_file.pcap")

#define TCPDUMP_WRITE_FLAG (0x1 << 2)

#define TCPDUMP_FILE_SIZE(_file) \
    (sizeof(struct rt_pcap_file) + _file->ip_len)

struct tcpdump_msg
{
    void *pbuf;
    rt_uint32_t sec;
    rt_uint32_t msec;
};

static rt_mq_t tcpdump_mq; 
static struct netif *netif;
static netif_linkoutput_fn link_output;
static rt_uint32_t tcpdump_flag;
static char *filename;

static netif_input_fn input;

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

static void rt_tcpdump_file_dump(rt_pcap_file_t *file)
{
    if (file == RT_NULL)
    {
        rt_kprintf("file is null\n");
        return;
    }

    rt_kprintf("------------------------file header---------------------\n");
    rt_kprintf("magi       major  minor  zone   sigfigs  snaplen linktype\n");
    rt_kprintf("0x%08x ",file->p_f_h.magic);
    rt_kprintf("0x%04x ", file->p_f_h.version_major);
    rt_kprintf("0x%04x ", file->p_f_h.version_minor);
    rt_kprintf("0x%04x ", file->p_f_h.thiszone);
    rt_kprintf("0x%04x   ", file->p_f_h.sigfigs);
    rt_kprintf("0x%04x  ", file->p_f_h.snaplen);
    rt_kprintf("0x%04x\n\n", file->p_f_h.linktype);

    rt_kprintf("       msec         sec         len      caplen \n");
    rt_kprintf("%11d ", file->p_h.ts.tv_msec);
    rt_kprintf("%11d ", file->p_h.ts.tv_sec);
    rt_kprintf("%11d ", file->p_h.len);
    rt_kprintf("%11d \n\n", file->p_h.caplen);

    dump_hex(file->ip_mess, file->ip_len);
    rt_kprintf("--------------------------end---------------------------\n");
}
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

static rt_err_t rt_tcpdump_pcap_file_write(rt_pcap_file_t *file, int len)
{
    int fd, length;

    if (filename == RT_NULL)
    {
        rt_kprintf("file name is null\n");
        return -RT_ERROR;
    }
    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0);
    if (fd < 0)
    {
        rt_kprintf("open file for write failed\n");
        return -RT_ERROR;
    }

    length = write(fd, file, len);
    if (length != len)
    {
        rt_kprintf("write data failed\n");
        close(fd);
        return -RT_ERROR;
    }
    close(fd);
    rt_kprintf("read/write done.\n");
    return RT_EOK;
}

static void rt_tcpdump_pcap_file_delete(rt_pcap_file_t *file)
{
    rt_free(file);
}

static rt_pcap_file_t *rt_tcpdump_pcap_file_create(struct pbuf *p)
{
    rt_pcap_file_t *file = RT_NULL;
    rt_uint8_t *ip_mess = RT_NULL;
    int p_len = p->tot_len;

    file = rt_malloc(sizeof(struct rt_pcap_file) + p_len);
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
    file->p_h.caplen = p_len;   //  ip len
    file->p_h.len = p_len;      //

    ip_mess = (rt_uint8_t *)(file->ip_mess);
    while (p)
    {
        rt_memcpy(ip_mess, p->payload, p->len);
        ip_mess += p->len;
        p = p->next;
    }
    file->ip_len = p_len;

    return file;
}

static void rt_tcpdump_thread_entry(void *param)
{
    struct pbuf *pbuf;
    struct tcpdump_msg msg;
    rt_pcap_file_t *dump_file;

    while (1)
    {
        if (rt_mq_recv(tcpdump_mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
        {
            pbuf = msg.pbuf;

            dump_file = rt_tcpdump_pcap_file_create(pbuf);
            pbuf_free(pbuf);

            if ((tcpdump_flag & TCPDUMP_WRITE_FLAG) && (dump_file != RT_NULL))
            {
                if (rt_tcpdump_pcap_file_write(dump_file, TCPDUMP_FILE_SIZE(dump_file)) != RT_EOK)
                {
                    rt_kprintf("tcp dump write file fail\nstop write file\n");
                    tcpdump_flag &= ~TCPDUMP_WRITE_FLAG;
                }
                rt_kprintf("write?\n");
            }
#ifdef TCPDUMP_DEBUG
            rt_tcpdump_file_dump(dump_file);
#endif
            rt_tcpdump_pcap_file_delete(dump_file);
        }
        else
        {
            rt_kprintf("tcp dump thread exit\n");
            return;
        }
    }
}

void rt_tcpdump_write_enable(void)
{
    tcpdump_flag |= TCPDUMP_WRITE_FLAG;
}

void rt_tcpdump_write_disable(void)
{
    tcpdump_flag &= ~TCPDUMP_WRITE_FLAG;
}

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
    
}
MSH_CMD_EXPORT(tcpdump_save, save);

//msh opt

