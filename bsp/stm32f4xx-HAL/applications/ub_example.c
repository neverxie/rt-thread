/*
 * File       : ub_example.c
 *
 * Change Logs:
 * Date         Author      Notes
 * 2019-05-04   never       the first version
 */

#include <stm32f4xx_hal.h>
#include "u-button.h"
#include <rtthread.h>
#include <rtdevice.h>

#define COL1_PIN      55 // read
#define COL2_PIN      43 // read
#define ROW1_PIN        41 // output
#define ROW2_PIN        29 // output
#define ROW3_PIN        56 // output
#define ROW4_PIN        57 // output

static ub_dev_t g_dev;
TIM_HandleTypeDef TIM3_Handler;

uint8_t btn_pin_output[4] = {41, 29, 56, 57};
uint8_t btn_pin_input[2] = {55, 43};

static uint32_t btnm_scan(void) {
    uint8_t i, j, btn_num = 0;
    uint32_t btn_read = 0;

    // set high
    for (i = 0; i < 4; i++) {
        rt_pin_write(btn_pin_output[i], PIN_HIGH);
    }

    for (i = 0; i < 4; i++) {
        // set low
        rt_pin_write(btn_pin_output[i], PIN_LOW);

        // read
        for (j = 0; j < 2; j++) {
            if (rt_pin_read(btn_pin_input[j]) == 0) {
                btn_read = btn_read | (1 << btn_num);
                //rt_kprintf("btn_read: 0x%x\n", btn_read);
            }
            btn_num++;
        }

        // set high
        rt_pin_write(btn_pin_output[i], PIN_HIGH);
    }

    // set low
    for (i = 0; i < 4; i++) {
        rt_pin_write(btn_pin_output[i], PIN_LOW);
    }

    return btn_read;
}

// 读按键
uint32_t ub_dev_read_cb(void) {
    return btnm_scan();
}

static void exti_irq_cb(void *args) {
    ub_dev_state_set(&g_dev);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim==(&TIM3_Handler)) {
        ub_dev_scan(&g_dev);
    }
}

static void ub_time_irq_control_cb(ub_bool_t enabled) {
    if (enabled) {
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    } else {
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
    }
}

static void ub_exti_irq_control_cb(ub_bool_t enabled) {
    if (enabled) {
        rt_pin_irq_enable(COL1_PIN , PIN_IRQ_ENABLE);
    } else {
        rt_pin_irq_enable(COL1_PIN , PIN_IRQ_DISABLE);
    }
}

static void tim3_init(void) {
    TIM3_Handler.Instance=TIM3;
    TIM3_Handler.Init.Prescaler=8400 - 1;
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;
    TIM3_Handler.Init.Period=200 - 1;
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&TIM3_Handler);

    HAL_TIM_Base_Start_IT(&TIM3_Handler);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    if(htim->Instance==TIM3) {
        __HAL_RCC_TIM3_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM3_IRQn,1,3);
    }
}

void TIM3_IRQHandler(void) {

    HAL_TIM_IRQHandler(&TIM3_Handler);
}

static void pin_init(void) {
    rt_pin_mode(ROW1_PIN , PIN_MODE_OUTPUT);
    rt_pin_write(ROW1_PIN , PIN_LOW);
    rt_pin_mode(ROW2_PIN , PIN_MODE_OUTPUT);
    rt_pin_write(ROW2_PIN , PIN_LOW);
    rt_pin_mode(ROW3_PIN , PIN_MODE_OUTPUT);
    rt_pin_write(ROW3_PIN , PIN_LOW);
    rt_pin_mode(ROW4_PIN , PIN_MODE_OUTPUT);
    rt_pin_write(ROW4_PIN , PIN_LOW);

    rt_pin_mode(COL1_PIN , PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(COL1_PIN , PIN_IRQ_MODE_FALLING ,\
            exti_irq_cb , RT_NULL);
    rt_pin_irq_enable(COL1_PIN , PIN_IRQ_ENABLE);

    rt_pin_mode(COL2_PIN , PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(COL2_PIN , PIN_IRQ_MODE_FALLING ,\
            exti_irq_cb , RT_NULL);
    rt_pin_irq_enable(COL2_PIN , PIN_IRQ_ENABLE);
}

static void ub_ex_thead(void *param) {
    while (1) {
        rt_thread_delay(500);
    }
}

static int ub_ex(void) {
    rt_thread_t tid;

    tim3_init();
    pin_init();

    g_dev.filter_time = 0;
    g_dev.hold_active_time = 40;
    g_dev.repeat_speed = 0;
    g_dev.irq_mode = 1;
    g_dev.ub_dev_read = ub_dev_read_cb;
    g_dev.ops.ub_exti_irq_control = ub_exti_irq_control_cb;
    g_dev.ops.ub_time_irq_control = ub_time_irq_control_cb;
    ub_dev_init(&g_dev);

    tid = rt_thread_create("ubex", ub_ex_thead, NULL, 512, 10, 5);
    if (tid == NULL) {
        goto __exit;
    }

    rt_thread_startup(tid);

    return 0;

__exit:
    return -1;

}
INIT_APP_EXPORT(ub_ex);
