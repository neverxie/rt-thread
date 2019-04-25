/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-07-29     Arda.Fu      first implementation
 */
#include <rtthread.h>
#include <board.h>
#include <stm32f4xx_hal.h>

int bt_io_init(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStructure.Pin =  GPIO_PIN_13;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
    return 0;
}

int main(void)
{
    /* user app entry */

    while (1) {
        rt_thread_delay(10);
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == RESET)
            rt_kprintf("B1 press down\n");
    }
    return 0;
}

INIT_APP_EXPORT(bt_io_init);
