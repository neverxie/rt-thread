#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
/* RT_USING_ARCH_DATA_TYPE is not set */
/* RT_USING_SMP is not set */
#define RT_ALIGN_SIZE 4
/* RT_THREAD_PRIORITY_8 is not set */
#define RT_THREAD_PRIORITY_32
/* RT_THREAD_PRIORITY_256 is not set */
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDEL_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 1024
/* RT_USING_TIMER_SOFT is not set */
#define RT_DEBUG
#define RT_DEBUG_COLOR
/* RT_DEBUG_INIT_CONFIG is not set */
/* RT_DEBUG_THREAD_CONFIG is not set */
/* RT_DEBUG_SCHEDULER_CONFIG is not set */
/* RT_DEBUG_IPC_CONFIG is not set */
/* RT_DEBUG_TIMER_CONFIG is not set */
/* RT_DEBUG_IRQ_CONFIG is not set */
/* RT_DEBUG_MEM_CONFIG is not set */
/* RT_DEBUG_SLAB_CONFIG is not set */
/* RT_DEBUG_MEMHEAP_CONFIG is not set */
/* RT_DEBUG_MODULE_CONFIG is not set */

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
/* RT_USING_SIGNALS is not set */

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
/* RT_USING_NOHEAP is not set */
#define RT_USING_SMALL_MEM
/* RT_USING_SLAB is not set */
/* RT_USING_MEMHEAP_AS_HEAP is not set */
/* RT_USING_MEMTRACE is not set */
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
/* RT_USING_DEVICE_OPS is not set */
/* RT_USING_INTERRUPT_INFO is not set */
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart2"
#define RT_VER_NUM 0x40001
/* ARCH_CPU_STACK_GROWS_UPWARD is not set */

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */

/* RT_USING_CPLUSPLUS is not set */

/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
/* FINSH_ECHO_DISABLE_DEFAULT is not set */
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
/* FINSH_USING_AUTH is not set */
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10

/* Device virtual file system */

/* RT_USING_DFS is not set */

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
/* RT_USING_SYSTEM_WORKQUEUE is not set */
#define RT_USING_SERIAL
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ 64
/* RT_USING_CAN is not set */
#define RT_USING_HWTIMER
/* RT_USING_CPUTIME is not set */
/* RT_USING_I2C is not set */
#define RT_USING_PIN
/* RT_USING_ADC is not set */
/* RT_USING_PWM is not set */
/* RT_USING_MTD_NOR is not set */
/* RT_USING_MTD_NAND is not set */
/* RT_USING_MTD is not set */
/* RT_USING_PM is not set */
/* RT_USING_RTC is not set */
/* RT_USING_SDIO is not set */
/* RT_USING_SPI is not set */
/* RT_USING_WDT is not set */
/* RT_USING_AUDIO is not set */
/* RT_USING_SENSOR is not set */

/* Using WiFi */

/* RT_USING_WIFI is not set */

/* Using USB */

/* RT_USING_USB_HOST is not set */
/* RT_USING_USB_DEVICE is not set */

/* POSIX layer and C standard library */

#define RT_USING_LIBC
/* RT_USING_PTHREADS is not set */

/* Network */

/* Socket abstraction layer */

/* RT_USING_SAL is not set */

/* light weight TCP/IP stack */

/* RT_USING_LWIP is not set */

/* Modbus master and slave stack */

/* RT_USING_MODBUS is not set */

/* AT commands */

/* RT_USING_AT is not set */

/* VBUS(Virtual Software BUS) */

/* RT_USING_VBUS is not set */

/* Utilities */

/* RT_USING_LOGTRACE is not set */
/* RT_USING_RYM is not set */
/* RT_USING_ULOG is not set */
/* RT_USING_UTEST is not set */

/* RT-Thread online packages */

/* IoT - internet of things */

/* PKG_USING_PAHOMQTT is not set */
/* PKG_USING_WEBCLIENT is not set */
/* PKG_USING_WEBNET is not set */
/* PKG_USING_MONGOOSE is not set */
/* PKG_USING_WEBTERMINAL is not set */
/* PKG_USING_CJSON is not set */
/* PKG_USING_JSMN is not set */
/* PKG_USING_LIBMODBUS is not set */
/* PKG_USING_LJSON is not set */
/* PKG_USING_EZXML is not set */
/* PKG_USING_NANOPB is not set */

/* Wi-Fi */

/* Marvell WiFi */

/* PKG_USING_WLANMARVELL is not set */

/* Wiced WiFi */

/* PKG_USING_WLAN_WICED is not set */
/* PKG_USING_RW007 is not set */
/* PKG_USING_COAP is not set */
/* PKG_USING_NOPOLL is not set */
/* PKG_USING_NETUTILS is not set */
/* PKG_USING_AT_DEVICE is not set */
/* PKG_USING_WIZNET is not set */

/* IoT Cloud */

/* PKG_USING_ONENET is not set */
/* PKG_USING_GAGENT_CLOUD is not set */
/* PKG_USING_ALI_IOTKIT is not set */
/* PKG_USING_AZURE is not set */
/* PKG_USING_TENCENT_IOTKIT is not set */
/* PKG_USING_NIMBLE is not set */
/* PKG_USING_OTA_DOWNLOADER is not set */

/* security packages */

/* PKG_USING_MBEDTLS is not set */
/* PKG_USING_libsodium is not set */
/* PKG_USING_TINYCRYPT is not set */

/* language packages */

/* PKG_USING_LUA is not set */
/* PKG_USING_JERRYSCRIPT is not set */
/* PKG_USING_MICROPYTHON is not set */

/* multimedia packages */

/* PKG_USING_OPENMV is not set */
/* PKG_USING_MUPDF is not set */
/* PKG_USING_STEMWIN is not set */

/* tools packages */

/* PKG_USING_CMBACKTRACE is not set */
/* PKG_USING_EASYFLASH is not set */
/* PKG_USING_EASYLOGGER is not set */
/* PKG_USING_SYSTEMVIEW is not set */
/* PKG_USING_RDB is not set */
/* PKG_USING_QRCODE is not set */
/* PKG_USING_ULOG_EASYFLASH is not set */
/* PKG_USING_ADBD is not set */

/* system packages */

/* PKG_USING_GUIENGINE is not set */
/* PKG_USING_CAIRO is not set */
/* PKG_USING_PIXMAN is not set */
/* PKG_USING_LWEXT4 is not set */
/* PKG_USING_PARTITION is not set */
/* PKG_USING_FAL is not set */
/* PKG_USING_SQLITE is not set */
/* PKG_USING_RTI is not set */
/* PKG_USING_LITTLEVGL2RTT is not set */
/* PKG_USING_CMSIS is not set */
/* PKG_USING_DFS_YAFFS is not set */
/* PKG_USING_LITTLEFS is not set */
/* PKG_USING_THREAD_POOL is not set */

/* peripheral libraries and drivers */

/* PKG_USING_SENSORS_DRIVERS is not set */
/* PKG_USING_REALTEK_AMEBA is not set */
/* PKG_USING_SHT2X is not set */
/* PKG_USING_AHT10 is not set */
/* PKG_USING_AP3216C is not set */
/* PKG_USING_STM32_SDIO is not set */
/* PKG_USING_ICM20608 is not set */
/* PKG_USING_U8G2 is not set */
/* PKG_USING_BUTTON is not set */
/* PKG_USING_MPU6XXX is not set */
/* PKG_USING_PCF8574 is not set */
/* PKG_USING_SX12XX is not set */
/* PKG_USING_SIGNAL_LED is not set */
/* PKG_USING_WM_LIBRARIES is not set */
/* PKG_USING_KENDRYTE_SDK is not set */
/* PKG_USING_INFRARED is not set */
/* PKG_USING_ROSSERIAL is not set */

/* miscellaneous packages */

/* PKG_USING_LIBCSV is not set */
/* PKG_USING_OPTPARSE is not set */
/* PKG_USING_FASTLZ is not set */
/* PKG_USING_MINILZO is not set */
/* PKG_USING_QUICKLZ is not set */
#define PKG_USING_MULTIBUTTON
/* PKG_USING_MULTIBUTTON_V102 is not set */
#define PKG_USING_MULTIBUTTON_LATEST_VERSION

/* MultiButton Options */

#define MULTIBUTTON_USING_EXAMPLE_ASYNC
/* MULTIBUTTON_USING_EXAMPLE_INQUIRE is not set */
/* PKG_USING_CANFESTIVAL is not set */
/* PKG_USING_ZLIB is not set */
/* PKG_USING_DSTR is not set */
/* PKG_USING_TINYFRAME is not set */
/* PKG_USING_KENDRYTE_DEMO is not set */

/* samples: kernel and components samples */

/* PKG_USING_KERNEL_SAMPLES is not set */
/* PKG_USING_FILESYSTEM_SAMPLES is not set */
/* PKG_USING_NETWORK_SAMPLES is not set */
/* PKG_USING_PERIPHERAL_SAMPLES is not set */
/* PKG_USING_HELLO is not set */
/* PKG_USING_VI is not set */
/* PKG_USING_NNOM is not set */
/* SOC_STM32F405RG is not set */
/* SOC_STM32F405VG is not set */
/* SOC_STM32F405ZG is not set */
/* SOC_STM32F415RG is not set */
/* SOC_STM32F415VG is not set */
/* SOC_STM32F415ZG is not set */
/* SOC_STM32F407VG is not set */
/* SOC_STM32F407VE is not set */
/* SOC_STM32F407ZG is not set */
/* SOC_STM32F407ZE is not set */
/* SOC_STM32F407IG is not set */
/* SOC_STM32F407IE is not set */
/* SOC_STM32F417VG is not set */
/* SOC_STM32F417VE is not set */
/* SOC_STM32F417ZG is not set */
/* SOC_STM32F417ZE is not set */
/* SOC_STM32F417IG is not set */
/* SOC_STM32F417IE is not set */
/* SOC_STM32F427VG is not set */
/* SOC_STM32F427VI is not set */
/* SOC_STM32F427ZG is not set */
/* SOC_STM32F427ZI is not set */
/* SOC_STM32F427IG is not set */
/* SOC_STM32F427II is not set */
/* SOC_STM32F437VG is not set */
/* SOC_STM32F437VI is not set */
/* SOC_STM32F437ZG is not set */
/* SOC_STM32F437ZI is not set */
/* SOC_STM32F437IG is not set */
/* SOC_STM32F437II is not set */
/* SOC_STM32F429VG is not set */
/* SOC_STM32F429VI is not set */
/* SOC_STM32F429ZG is not set */
/* SOC_STM32F429ZI is not set */
/* SOC_STM32F429BG is not set */
/* SOC_STM32F429BI is not set */
/* SOC_STM32F429NG is not set */
/* SOC_STM32F429NI is not set */
/* SOC_STM32F429IG is not set */
/* SOC_STM32F429II is not set */
/* SOC_STM32F439VG is not set */
/* SOC_STM32F439VI is not set */
/* SOC_STM32F439ZG is not set */
/* SOC_STM32F439ZI is not set */
/* SOC_STM32F439BG is not set */
/* SOC_STM32F439BI is not set */
/* SOC_STM32F439NG is not set */
/* SOC_STM32F439NI is not set */
/* SOC_STM32F439IG is not set */
/* SOC_STM32F439II is not set */
/* SOC_STM32F401CB is not set */
/* SOC_STM32F401CC is not set */
/* SOC_STM32F401RB is not set */
/* SOC_STM32F401RC is not set */
/* SOC_STM32F401VB is not set */
/* SOC_STM32F401VC is not set */
/* SOC_STM32F401CD is not set */
/* SOC_STM32F401RD is not set */
/* SOC_STM32F401VD is not set */
/* SOC_STM32F401CE is not set */
#define SOC_STM32F401RE
/* SOC_STM32F401VE is not set */
/* SOC_STM32F410T8 is not set */
/* SOC_STM32F410TB is not set */
/* SOC_STM32F410C8 is not set */
/* SOC_STM32F410CB is not set */
/* SOC_STM32F410R8 is not set */
/* SOC_STM32F410RB is not set */
/* SOC_STM32F411CC is not set */
/* SOC_STM32F411RC is not set */
/* SOC_STM32F411VC is not set */
/* SOC_STM32F411CE is not set */
/* SOC_STM32F411RE is not set */
/* SOC_STM32F411VE is not set */
/* SOC_STM32F446MC is not set */
/* SOC_STM32F446ME is not set */
/* SOC_STM32F446RC is not set */
/* SOC_STM32F446RE is not set */
/* SOC_STM32F446VC is not set */
/* SOC_STM32F446VE is not set */
/* SOC_STM32F446ZC is not set */
/* SOC_STM32F446ZE is not set */
/* SOC_STM32F469AI is not set */
/* SOC_STM32F469II is not set */
/* SOC_STM32F469BI is not set */
/* SOC_STM32F469NI is not set */
/* SOC_STM32F469AG is not set */
/* SOC_STM32F469IG is not set */
/* SOC_STM32F469BG is not set */
/* SOC_STM32F469NG is not set */
/* SOC_STM32F469AE is not set */
/* SOC_STM32F469IE is not set */
/* SOC_STM32F469BE is not set */
/* SOC_STM32F469NE is not set */
/* SOC_STM32F479AI is not set */
/* SOC_STM32F479II is not set */
/* SOC_STM32F479BI is not set */
/* SOC_STM32F479NI is not set */
/* SOC_STM32F479AG is not set */
/* SOC_STM32F479IG is not set */
/* SOC_STM32F479BG is not set */
/* SOC_STM32F479NG is not set */
/* SOC_STM32F412CEU is not set */
/* SOC_STM32F412CGU is not set */
/* SOC_STM32F412ZET is not set */
/* SOC_STM32F412ZGT is not set */
/* SOC_STM32F412ZEJ is not set */
/* SOC_STM32F412ZGJ is not set */
/* SOC_STM32F412VET is not set */
/* SOC_STM32F412VGT is not set */
/* SOC_STM32F412VEH is not set */
/* SOC_STM32F412VGH is not set */
/* SOC_STM32F412RET is not set */
/* SOC_STM32F412RGT is not set */
/* SOC_STM32F412REY is not set */
/* SOC_STM32F412RGY is not set */
/* SOC_STM32F413CH is not set */
/* SOC_STM32F413MH is not set */
/* SOC_STM32F413RH is not set */
/* SOC_STM32F413VH is not set */
/* SOC_STM32F413ZH is not set */
/* SOC_STM32F413CG is not set */
/* SOC_STM32F413MG is not set */
/* SOC_STM32F413RG is not set */
/* SOC_STM32F413VG is not set */
/* SOC_STM32F413ZG is not set */
/* SOC_STM32F423CH is not set */
/* SOC_STM32F423RH is not set */
/* SOC_STM32F423VH is not set */
/* SOC_STM32F423ZH is not set */
#define RT_USING_HSI
#define RT_HSE_HCLK 84000000
#define BSP_USING_UART1
#define BSP_USING_UART2
/* BSP_USING_UART3 is not set */
/* BSP_USING_UART6 is not set */
/* BSP_USING_PWM1 is not set */
/* BSP_USING_PWM2 is not set */
/* BSP_USING_PWM3 is not set */
/* BSP_USING_PWM4 is not set */
/* BSP_USING_PWM5 is not set */

#endif
