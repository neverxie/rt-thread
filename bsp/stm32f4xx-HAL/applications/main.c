/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-07-29     Arda.Fu      first implementation
 */
#include <rtthread.h>
#include <board.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"

int main(void)
{
    /* user app entry */
    stm32_spi_bus_attach_device(33, "spi2", "spi20");
    rt_sfud_flash_probe("W25Q128", "spi20"); 
    return 0;
}
