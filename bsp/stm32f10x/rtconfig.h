#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Project Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 100
#define RT_DEBUG
#define RT_USING_OVERFLOW_CHECK
#define RT_DEBUG_INIT 0
#define RT_DEBUG_THREAD 0
#define RT_USING_HOOK
#define IDLE_THREAD_STACK_SIZE 256

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart1"
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M3

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT

/* Device virtual file system */


/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_USING_SERIAL
#define RT_USING_CAN
#define RT_CAN_USING_HDR
#define RT_USING_PIN

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC

/* Network stack */

/* light weight TCP/IP stack */


/* Modbus master and slave stack */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* ARM CMSIS */

#define RT_USING_RTT_CMSIS

/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* security packages */


/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */

/* RT-Thread GUI Engine */


/* peripheral libraries and drivers */


/* miscellaneous packages */

#define PKG_USING_MULTIBUTTON
#define PKG_USING_MULTIBUTTON_LATEST_VERSION

/* MultiButton Options */

#define MULTIBUTTON_USING_EXAMPLE_ASYNC

/* sample package */


/* example package: hello */

#define PKG_USING_HELLO
#define PKG_USING_HELLO_LATEST_VERSION
#define SOC_STM32F1
#define RT_USING_UART1
#define RT_USING_UART2
#define RT_USING_UART3

#endif
