/*
 * File       : led.c
 *
 * Change Logs:
 * Date         Author      Notes
 * 2019.4.20    never       the first version
 */
#include <rtthread.h>
#include "drv_led.h"

void led_init(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStructure.Pin =  GPIO_PIN_5;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}
