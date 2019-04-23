#ifndef __DRV_LED_H
#define __DRV_LED_H

#include <rtthread.h>
#include "stm32f4xx_hal.h"

#define LD2_ON     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET)
#define LD2_OFF    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)

void led_init(void);

#endif
