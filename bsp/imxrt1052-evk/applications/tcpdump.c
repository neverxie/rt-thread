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

static struct netif *netif;
static struct rt_messagequeue *mq;
static netif_linkoutput_fn link_output;
static char *filename;
static rt_uint32_t tcpdump_flag;
static int fd;

#define TCPDUMP_WRITE_FLAG (0x1 << 2)
#define TCPDUMP_DEFAULT_NAME    ("tcpdump_file.pcap")
#define TCPDUMP_FILE_SIZE(_file) \
    (sizeof(struct rt_pcap_file) + _file->ip_len) 

#ifdef TCPDUMP_DEBUG
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void hex_dump(const rt_uint8_t *ptr, rt_size_t buflen)
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

static void rt_tcpdump_file_dump(struct rt_pcap_file *file)
{
    rt_uint8_t buf[PCAP_FILE_FORMAT_SIZE] = {0};
    
    if (file == RT_NULL)
    {
        rt_kprintf("file is null\n");
        return;
    }
    rt_kprintf("\n\n");
    rt_kprintf("-------------------------file header---------------------\n");
    rt_kprintf("magi       major  minor  zone   sigfigs  snaplen linktype\n");
    rt_kprintf("0x%08x ", file->p_f_h.magic);
    rt_kprintf("0x%04x ", file->p_f_h.version_major);
    rt_kprintf("0x%04x ", file->p_f_h.version_minor);
    rt_kprintf("0x%04x ", file->p_f_h.thiszone);
    rt_kprintf("0x%04x   ", file->p_f_h.sigfigs);
    rt_kprintf("0x%04x  ", file->p_f_h.snaplen);
    rt_kprintf("0x%04x\n\n", file->p_f_h.linktype);

    rt_kprintf("       msec         sec         len      caplen \n");
    rt_kprintf("%11d ", file->p_pktdr.ts.tv_msec);
    rt_kprintf("%11d ", file->p_pktdr.ts.tv_sec);
    rt_kprintf("%11d ", file->p_pktdr.len);
    rt_kprintf("%11d \n\n", file->p_pktdr.caplen);

    rt_struct_to_u8(file, buf);
    hex_dump(buf, sizeof(buf));
    hex_dump(file->ip_mess, file->ip_len);
    rt_kprintf("---------------------------end---------------------------\n");
    rt_kprintf("\n\n");
}
#endif

void rt_tcpdump_pcap_file_save(struct pbuf *p)
{
    struct pbuf *pbuf = p;
    struct tcpdump_msg msg;    
    struct rt_pcap_pkthdr pkthdr;
    rt_uint8_t ip_len = p->tot_len;
    rt_uint8_t *ip_mess = p->payload;
    
    if (p != RT_NULL)
    {   
        pkthdr.ts.tv_sec = msg.sec;      //  os_tick
        pkthdr.ts.tv_msec = msg.msec;    //  os_tick
        pkthdr.caplen = ip_len;          //  ip len
        pkthdr.len = ip_len;             //

        write(fd, &pkthdr, PCAP_PKTHDR_SIZE);
    
        while (p) 
        {   
            write(fd, ip_mess, p->len);
            ip_mess += p->len;
            p = p->next;
        }
        pbuf_free(pbuf);  //    where?
    }
    else
    {
        rt_kprintf("recieve ip mess failed\n");
        close(fd);
    }
}

//static void rt_tcpdump_pcap_file_close(int fd)
//{
//    close(fd);
//    rt_kprintf("tcpdump file write done!\n");
//}

static int rt_tcpdump_pcap_file_init(int fd)
{
    int fe,length;
    struct rt_pcap_file_header file_header;
    
    rt_kprintf("333333333333333333333\n");
    
//    if (filename == RT_NULL) 
//    {
//        rt_kprintf("filename failed\n");
//        return -RT_ERROR;
//    }

    /* write and creat */
    fe = open("mm.pcap", O_WRONLY | O_CREAT | O_APPEND, 0);
    rt_kprintf("fe: %d\n", fe);
    if (fe < 0) 
    {
        rt_kprintf("tcpdump file for write failed\n");
        return -RT_ERROR;
    }

    /* write pcap file */
    file_header.magic = PCAP_FILE_ID;
    file_header.version_major = PCAP_VERSION_MAJOR;
    file_header.version_minor = PCAP_VERSION_MINOR;
    file_header.thiszone = GREENWICH_MEAN_TIME;
    file_header.sigfigs = PRECISION_OF_TIME_STAMP;
    file_header.snaplen = MAX_LENTH_OF_CAPTURE_PKG;
    file_header.linktype = ETHERNET;
    length = write(fd, &file_header, PCAP_FILE_HEADER_SIZE);
    if (length != PCAP_FILE_HEADER_SIZE)
    {
        rt_kprintf("write data failed\n");
        close(fe);
        return -RT_ERROR;
    }
    return fe;
}

static struct pbuf *rt_tcpdump_ip_mess_recv(void)
{
    struct tcpdump_msg msg;
    struct pbuf *p;

    if (rt_mq_recv(mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK) 
    {
        p = msg.pbuf;
        return p;
    } 
    else 
        return RT_NULL;
}

static err_t _netif_linkoutput(struct netif *netif, struct pbuf *p)
{
    struct tcpdump_msg msg;
    rt_uint32_t tick = rt_tick_get();

    if (p != RT_NULL) 
    {
        pbuf_ref(p);
        msg.pbuf = p;
        msg.sec  = tick / 1000;
        msg.msec = tick % 1000;

        if (rt_mq_send(mq, &msg, sizeof(msg)) != RT_EOK) 
        {
            rt_kprintf("mq send failed\n");
            pbuf_free(p);
            return -RT_ERROR;
        }
    }
    return link_output(netif, p);
}

#if 0
static void rt_struct_to_u8(struct rt_pcap_file *file, rt_uint8_t *buf)
{
    union rt_u32_data u32_data;
    union rt_u16_data u16_data;
    int k, i, j;
    
    struct rt_pcap_file_header *p_p_f_h = (struct rt_pcap_file_header *)file;
    rt_uint32_t *p32_p_f_h = (rt_uint32_t *)p_p_f_h + 2;

    struct rt_pcap_pkthdr *p32_pktdr = (struct rt_pcap_pkthdr *)(p_p_f_h + 1);
    rt_uint32_t *p32 = (rt_uint32_t *)p32_pktdr;

    rt_uint16_t *p16 = (rt_uint16_t *)p_p_f_h + 2;

    /* struct rt_pcap_file_header. magic */
    u32_data.u32byte = 0;
    u32_data.u32byte = file->p_f_h.magic;
    for (k = 3; k != -1; k--)
        buf[k] = u32_data.a[k];
    
    /* struct rt_pcap_file_header. version_major & version_minor*/
    for (i = 0, j = 4; i < 4; i++)
    {
        u16_data.u16byte = 0;
        u16_data.u16byte = *(p16 + 0);
        for (k = 1; k != -1; k--) 
        {
            buf[k+j] = u16_data.a[k];
        }
        j += 4;
    }

    /* struct rt_pcap_header.thiszone ~ linktype */
    for (i = 0, j = 8; i < 4; i++) 
    {
        u32_data.u32byte = 0;
        u32_data.u32byte = *(p32_p_f_h + i);
        for (k = 3; k != -1; k--) 
        {
            buf[k+j] = u32_data.a[k];
        }
        j += 4;
    }
    /* struct rt_pcap_header */
    for (i = 0, j = 24; i < 4; i++) 
    {
        u32_data.u32byte = 0;
        u32_data.u32byte = *(p32 + i);
        for (k = 3; k != -1; k--) 
        {
            buf[k+j] = u32_data.a[k];
        }
        j += 4;
    }
}
#endif

static void rt_tcp_dump_thread(void *param)
{
    struct pbuf *p = RT_NULL;

    while (1) 
    {
        p = rt_tcpdump_ip_mess_recv();

        rt_tcpdump_pcap_file_save(p);
        
        
#ifdef TCPDUMP_DEBUG
        rt_tcpdump_file_dump(file);
#endif
    }
}

/**
 * This function will enable to write into file system.
 *
 * @param none.
 *
 * @return none.
 */
void rt_tcpdump_write_enable(void)
{
//    tcpdump_flag |= TCPDUMP_WRITE_FLAG;
}

/**
 * This function will disable to write into file system.
 *
 * @param none.
 *
 * @return none.
 */
void rt_tcpdump_write_disable(void)
{
//    tcpdump_flag &= ~TCPDUMP_WRITE_FLAG;
}

/**
 * This function will set filename.
 *
 * @param name.
 *
 * @return none.
 */
void rt_tcpdump_set_filename(const char *name)
{
    if (filename != RT_NULL) 
    {
        rt_free(filename);
    }

    filename = rt_strdup(name);
}

/**
 * This function will initialize thread, mailbox, device etc.
 *
 * @param none.
 *
 * @return status.
 */
int rt_tcp_dump_init(void)
{
    static struct eth_device *dev = RT_NULL;
    struct rt_thread *tid = RT_NULL;
    rt_base_t level;
    
    dev = (struct eth_device *)rt_device_find("e0");
    if (dev == RT_NULL)
        return -RT_ERROR;

    mq = rt_mq_create("tcp_dump", sizeof(struct tcpdump_msg), TCPDUMP_MAX_MSG, RT_IPC_FLAG_FIFO);
    if (mq == RT_NULL) 
    {
        rt_kprintf("mq error\n");
        return -RT_ERROR;
    }

    tid = rt_thread_create("tcp_dump", rt_tcp_dump_thread, RT_NULL, 2048, 10, 10);
    if (tid == RT_NULL) 
    {
        rt_kprintf("tcp dump thread create fail\n");
        rt_mq_delete(mq);
        return -RT_ERROR;
    }
//    rt_tcpdump_set_filename("test1.pcap");
    rt_tcpdump_pcap_file_init(fd);
    
    level = rt_hw_interrupt_disable();
    netif = dev->netif;
    link_output = netif->linkoutput;    //   save
    netif->linkoutput = _netif_linkoutput;
    rt_hw_interrupt_enable(level);
    rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(rt_tcp_dump_init);

/**
 * This function will reset thread, mailbox, device etc.
 *
 * @param none.
 *
 * @return none.
 */
void rt_tcpdump_deinit(void)
{
    rt_base_t level;
    
    level = rt_hw_interrupt_disable();
    netif->linkoutput = link_output;
    netif = RT_NULL;
    rt_mq_delete(mq);
    rt_hw_interrupt_enable(level);
    mq = RT_NULL;
}

void tcpdump_init(void)
{
    rt_tcp_dump_init();
}
MSH_CMD_EXPORT(tcpdump_init, init);

void tcpdump_deinit(void)
{
    rt_tcp_dump_init();
}
MSH_CMD_EXPORT(tcpdump_deinit, deinit);

void tcpdump_save(void)
{
    rt_tcpdump_write_enable();
}
MSH_CMD_EXPORT(tcpdump_save, save);

int tcpdump_name(int argc, char *argv[])
{
    if (argc != 2)   
    {
        rt_kprintf("user: tcpdump filename\n");
    }
    rt_tcpdump_set_filename(argv[1]);
    rt_kprintf("set file name: %s\n", argv[1]);
    return 0;
}
MSH_CMD_EXPORT(tcpdump_name, my command with args);