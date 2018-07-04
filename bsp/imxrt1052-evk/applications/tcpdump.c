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
 * 2018-07-04     never        the first version
 */

#include <rtthread.h>
#include <dfs_posix.h>
#include <rtdef.h>
#include "lwip/netifapi.h"
#include <ethernetif.h>

#define DBG_ENABLE
//#undef  DBG_ENABLE
#define DBG_SECTION_NAME  "[TCPDUMP]"
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define TCPDUMP_MAX_MSG             (10)

#define PCAP_FILE_ID                (0xA1B2C3D4)
#define PCAP_VERSION_MAJOR          (0x200)
#define PCAP_VERSION_MINOR          (0x400)
#define GREENWICH_MEAN_TIME         (0)  
#define PRECISION_OF_TIME_STAMP     (0)
#define MAX_LENTH_OF_CAPTURE_PKG    (0xFFFF)
#define ETHERNET                    (1)

#define PCAP_FILE_HEADER_SIZE       (24)
#define PCAP_PKTHDR_SIZE            (16) 

union rt_u32_data
{
    rt_uint32_t u32byte;
    rt_uint8_t  a[4];
};

union rt_u16_data
{
    rt_uint16_t u16byte;
    rt_uint8_t  a[2];
};

struct rt_pcap_file_header
{
    rt_uint32_t magic;        
    rt_uint16_t version_major;  
    rt_uint16_t version_minor;  
    rt_int32_t  thiszone;     
    rt_uint32_t sigfigs;      
    rt_uint32_t snaplen;       
    rt_uint32_t linktype;    
};

struct rt_timeval
{
    rt_uint32_t tv_sec;
    rt_uint32_t tv_msec;
};

struct rt_pcap_pkthdr
{
    struct rt_timeval ts;
    rt_uint32_t caplen;
    rt_uint32_t len; 
};

struct tcpdump_msg 
{
    void *pbuf;
    rt_uint32_t sec;
    rt_uint32_t msec;
};

static struct tcpdump_msg msg;
static struct rt_pcap_file_header file_header;
static char *filename;

static struct rt_messagequeue *mq;
static struct netif *netif;
/* tx packet */
static netif_linkoutput_fn link_output;
/* rx packet */
static netif_input_fn input;

static int fd;

//#define TCPDUMP_DEBUG
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

static void rt_tcpdump_file_header_dump(struct rt_pcap_file_header *file_header)
{
    union rt_u32_data u32_data;
    union rt_u16_data u16_data;
    int k, i, j;
    rt_uint8_t buf[PCAP_FILE_HEADER_SIZE] = {0};
    rt_uint32_t *p32p_h  = (rt_uint32_t *)file_header + 2;
    rt_uint16_t *p16  = (rt_uint16_t *)file_header + 2;
    
//    rt_kprintf("\n\n");
//    rt_kprintf("-------------------------file header---------------------\n");
    rt_kprintf("magi       major  minor  zone   sigfigs  snaplen linktype\n");
    rt_kprintf("0x%08x ", file_header->magic);
    rt_kprintf("0x%04x ", file_header->version_major);
    rt_kprintf("0x%04x ", file_header->version_minor);
    rt_kprintf("0x%04x ", file_header->thiszone);
    rt_kprintf("0x%04x   ", file_header->sigfigs);
    rt_kprintf("0x%04x  ", file_header->snaplen);
    rt_kprintf("0x%04x\n\n", file_header->linktype);
    
    /* struct rt_pcap_file_header. magic */
    u32_data.u32byte = 0;
    u32_data.u32byte = file_header->magic;
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
        u32_data.u32byte = *(p32p_h + i);
        for (k = 3; k != -1; k--) 
        {
            buf[k+j] = u32_data.a[k];
        }
        j += 4;
    }
    
    hex_dump(buf, sizeof(buf));
}

static void rt_tcpdump_pkthdr_dump(struct pbuf *p, struct rt_pcap_pkthdr *pkthdr)
{
    rt_uint8_t buf[PCAP_PKTHDR_SIZE] = {0};
    rt_uint8_t  *ip_mess = p->payload;
    
    union rt_u32_data u32_data;
    union rt_u16_data u16_data;
    int k, i, j;

    rt_kprintf("       msec         sec         len      caplen \n");
    rt_kprintf("%11d ", pkthdr->ts.tv_msec);
    rt_kprintf("%11d ", pkthdr->ts.tv_sec);
    rt_kprintf("%11d ", pkthdr->len);
    rt_kprintf("%11d \n", pkthdr->caplen);

    rt_uint32_t *p32_dr = (rt_uint32_t *)pkthdr;
    
    /* struct rt_pcap_header */
    for (i = 0, j = 0; i < 4; i++) 
    {
        u32_data.u32byte = 0;
        u32_data.u32byte = *(p32_dr + i);
        for (k = 3; k != -1; k--) 
        {
            buf[k+j] = u32_data.a[k];
        }
        j += 4;
    }
    hex_dump(buf, sizeof(buf));

    while (p)
    {
        hex_dump(p->payload, p->len);
        ip_mess += p->len;
        p = p->next;    
    }    
}
#endif

static void rt_tcpdump_pcap_file_save(struct pbuf *p)
{
    struct pbuf *pbuf = p;
    struct rt_pcap_pkthdr pkthdr;
    rt_uint8_t ip_len = p->tot_len;
    rt_uint8_t *ip_mess = p->payload;
    int length;
    
    if (p != RT_NULL)
    {   
        pkthdr.ts.tv_sec = msg.sec;      //  os_tick
        pkthdr.ts.tv_msec = msg.msec;    //  os_tick
        pkthdr.caplen = ip_len;          //  ip len
        pkthdr.len = ip_len;             //

        length = write(fd, &pkthdr, PCAP_PKTHDR_SIZE);
        if (length != PCAP_PKTHDR_SIZE)
        {
            dbg_log(DBG_ERROR, "write pkthdr failed\n");
            close(fd);
            return;
        }
    #ifdef TCPDUMP_DEBUG
        rt_tcpdump_pkthdr_dump(p, &pkthdr);
        rt_kprintf("\n\n");
    #endif
        while (p) 
        {   
            length = write(fd, ip_mess, p->len);
//            rt_kprintf("p->len: %d\n", length);
//            if (length != p->len)
//            {
//                rt_kprintf("pkthdr write data failed\n");
//            }
            ip_mess += p->len;
            p = p->next;
        }
        pbuf_free(pbuf); 
    }
}

static void rt_tcpdump_pcap_file_close(void)
{
    close(fd);
    dbg_log(DBG_INFO, "tcpdump file save done!\n");
}

static int rt_tcpdump_pcap_file_init(void)
{
    int length;

    if (filename == RT_NULL) 
    {
        dbg_log(DBG_ERROR, "filename failed\n");
        return -RT_ERROR;
    }

    /* write and creat */
    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0);
    if (fd < 0) 
    {
        dbg_log(DBG_ERROR, "open file failed\n");
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
        dbg_log(DBG_ERROR, "write pcap header failed\n");
        close(fd);
        return -RT_ERROR;
    }
    dbg_log(DBG_INFO, "tcpdump file init ok!\n");
#ifdef TCPDUMP_DEBUG
    rt_tcpdump_file_header_dump(&file_header);
#endif   
//    return fd_val;
    return RT_EOK;
}

static struct pbuf *rt_tcpdump_ip_mess_recv(void)
{
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
    rt_uint32_t tick = rt_tick_get();

    if (p != RT_NULL) 
    {
        pbuf_ref(p);
        msg.pbuf = p;
        msg.sec  = tick / 1000;
        msg.msec = tick % 1000;

        if (rt_mq_send(mq, &msg, sizeof(msg)) != RT_EOK) 
        {
            dbg_log(DBG_ERROR, "tx mq send failed\n");
            pbuf_free(p);
            return -RT_ERROR;
        }
    }
    return link_output(netif, p);
}

static err_t _netif_input(struct pbuf *p, struct netif *inp)
{
    rt_uint32_t tick = rt_tick_get();

    if (p != RT_NULL) 
    {
        pbuf_ref(p);
        msg.pbuf = p;
        msg.sec  = tick / 1000;
        msg.msec = tick % 1000;

        if (rt_mq_send(mq, &msg, sizeof(msg)) != RT_EOK) 
        {
            dbg_log(DBG_ERROR, "rx mq send failed\n");
            pbuf_free(p);
            return -RT_ERROR;
        }
    }
    return input(p, inp);
}

static void rt_tcp_dump_thread(void *param)
{
    struct pbuf *p = RT_NULL;
    
    while (1) 
    { 
        p = rt_tcpdump_ip_mess_recv(); 
        rt_tcpdump_pcap_file_save(p);
    }
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
        dbg_log(DBG_ERROR, "mq create failed\n");
        return -RT_ERROR;
    }

    tid = rt_thread_create("tcp_dump", rt_tcp_dump_thread, RT_NULL, 2048, 10, 10);
    if (tid == RT_NULL) 
    {
        dbg_log(DBG_ERROR, "tcpdump thread create failed\n");
        rt_mq_delete(mq);
        return -RT_ERROR;
    }
//    rt_tcpdump_set_filename("test.pcap");

    level = rt_hw_interrupt_disable();
    netif = dev->netif;
    link_output = netif->linkoutput;    //  save
    netif->linkoutput = _netif_linkoutput;
    
    input = netif->input;               //  save
    netif->input = _netif_input;
    
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
    netif->input = input;
    netif = RT_NULL;
    
    rt_mq_delete(mq);
    rt_hw_interrupt_enable(level);
    mq = RT_NULL;
}

void tcpdump_deinit(void)
{
    rt_tcp_dump_init();
}
MSH_CMD_EXPORT(tcpdump_deinit, deinit);

void tcpdump_inquire(void)
{
    if (fd < 0 || (fd == 0))
    {
        dbg_log(DBG_ERROR, "tcpdump file init failed!\n");
    }
    else
    {
        dbg_log(DBG_INFO, "tcpdump file init ok!; fd: %d\n", fd);
    }
}
MSH_CMD_EXPORT(tcpdump_inquire, inquire file status);

void tcpdump_get(void)
{
    rt_tcpdump_pcap_file_init();
}
MSH_CMD_EXPORT(tcpdump_get, start capturing packets);

void tcpdump_save(void)
{
    rt_tcpdump_pcap_file_close();
    dbg_log(DBG_ERROR, "close fd: %d\n", fd);
}
MSH_CMD_EXPORT(tcpdump_save, to save captured packets);

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
MSH_CMD_EXPORT(tcpdump_name, save it with what name?);