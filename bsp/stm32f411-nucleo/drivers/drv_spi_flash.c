/*
 * File      : stm32f20x_40x_spi.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-01-01     aozima       first implementation.
 * 2012-07-27     aozima       fixed variable uninitialized.
 */
#include <board.h>
#include "drv_spi.h"
#include "spi_flash.h"
#include "spi_flash_sfud.h"

#include <rthw.h>
#include <finsh.h>


static int rt_hw_spi2_init(void)
{   
    /* register spi bus */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
		rt_err_t result;

		__HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        GPIO_InitStructure.Alternate  = GPIO_AF5_SPI2;
        GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Pull  = GPIO_PULLUP;
        GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
        GPIO_InitStructure.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

        GPIO_InitStructure.Alternate  = GPIO_AF5_SPI2;
        GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Pull  = GPIO_PULLUP;
        GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
        GPIO_InitStructure.Pin = GPIO_PIN_13;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
        
		result = stm32_spi_bus_register(SPI2, "spi2");
        if (result != RT_EOK)
		{
			return result;
		}
    }

    /* attach cs */
    {
        static struct rt_spi_device spi_device;
        static struct stm32_spi_cs  spi_cs;
		rt_err_t result;

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStructure.Pull  = GPIO_PULLUP;
        GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

        spi_cs.GPIOx = GPIOB;
        spi_cs.GPIO_Pin = GPIO_PIN_12;

        GPIO_InitStructure.Pin = spi_cs.GPIO_Pin;
        HAL_GPIO_WritePin(spi_cs.GPIOx, spi_cs.GPIO_Pin, GPIO_PIN_SET);
        HAL_GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);

        result = rt_spi_bus_attach_device(&spi_device, "spi20", "spi2", (void*)&spi_cs);
		if (result != RT_EOK)
		{
			return result;
		}
    }

	return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_spi2_init);