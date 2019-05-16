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

/* ------------------------------------- *
 *       |  COL1    COL2    COL3    COL4 *
 * ------------------------------------- *
 * ROW1  |  K1      K2      K3      K4   *
 *       |                               *
 * ROW2  |  K5      K6      K7      K8   *
 *       |                               *
 * ROW3  |  K9      K10     K11     K12  *
 *       |                               *
 * ROW4  |  K13     K14     K15     K16  *
 * ------------------------------------- */

/* input */
#define COL1_PIN        55 // PB3
#define COL2_PIN        43 // PA10
#define COL3_PIN        42 // PA9
#define COL4_PIN        38 // PC7
#define COL_NUM         4

/* output */
#define ROW1_PIN        41 // PA8
#define ROW2_PIN        29 // PB10
#define ROW3_PIN        56 // PB4
#define ROW4_PIN        57 // PB5
#define ROW_NUM         4

static ub_dev_t g_dev;
TIM_HandleTypeDef TIM3_Handler;

uint8_t btn_pin_output[4] = {41, 29, 56, 57};
uint8_t btn_pin_input[4] = {55, 43, 42, 38};

uint32_t ub_dev_read_cb(void) {
    uint8_t i, j, btn_num = 0;
    uint32_t btn_read = 0;

    // 1.set high
    for (i = 0; i < ROW_NUM; i++) {
        rt_pin_write(btn_pin_output[i], PIN_HIGH);
    }

    for (i = 0; i < ROW_NUM; i++) {
        // 2.set low
        rt_pin_write(btn_pin_output[i], PIN_LOW);

        // 3.read
        for (j = 0; j < COL_NUM; j++) {
            if (rt_pin_read(btn_pin_input[j]) == 0) {
                btn_read = btn_read | (1 << btn_num);
                //rt_kprintf("btn num: %d\n", btn_num);
            }
            btn_num++;
        }

        // 2.set high
        rt_pin_write(btn_pin_output[i], PIN_HIGH);
    }

    // 1.set low
    for (i = 0; i < ROW_NUM; i++) {
        rt_pin_write(btn_pin_output[i], PIN_LOW);
    }

    return btn_read;
}

void ub_even_cb(ub_dev_t *dev) {

    if (dev == NULL) {
        return;
    }

}

static void exti_irq_cb(void *args) {
    ub_device_state_set(&g_dev);
}

ub_err_t read_cb(void) {
    uint32_t i = rt_pin_read(COL1_PIN);
    if (i) { // i = 1,按键弹起
        i = i & (~(1 << 0));
    } else { // 按下
        i = i | (1 << 0);
    }
    // 貌似是这里的问题
    //rt_kprintf("i << 0: %d\n", i);
    return i;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim==(&TIM3_Handler)) {
        ub_device_state_handle(&g_dev);
    }
}

static void ub_time_irq_control_cb(ub_bool_t enabled) {
    if (enabled) {
        //rt_kprintf("TIM3 Enable\n");
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    } else {
        //rt_kprintf("TIM3 Disable\n");
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
    }
}

static void ub_exti_irq_control_cb(ub_bool_t enabled) {
    if (enabled) {
        //rt_kprintf("PIN Enable\n");
        rt_pin_irq_enable(COL1_PIN , PIN_IRQ_ENABLE);
        rt_pin_irq_enable(COL2_PIN , PIN_IRQ_ENABLE);
        rt_pin_irq_enable(COL3_PIN , PIN_IRQ_ENABLE);
        rt_pin_irq_enable(COL4_PIN , PIN_IRQ_ENABLE);
    } else {
        //rt_kprintf("PIN Disable\n");
        rt_pin_irq_enable(COL1_PIN , PIN_IRQ_DISABLE);
        rt_pin_irq_enable(COL2_PIN , PIN_IRQ_DISABLE);
        rt_pin_irq_enable(COL3_PIN , PIN_IRQ_DISABLE);
        rt_pin_irq_enable(COL4_PIN , PIN_IRQ_DISABLE);
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
    rt_pin_mode(COL2_PIN , PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(COL3_PIN , PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(COL4_PIN , PIN_MODE_INPUT_PULLUP);

    rt_pin_attach_irq(COL1_PIN , PIN_IRQ_MODE_FALLING ,\
            exti_irq_cb , RT_NULL);
    rt_pin_irq_enable(COL1_PIN , PIN_IRQ_ENABLE);

    rt_pin_attach_irq(COL2_PIN , PIN_IRQ_MODE_FALLING ,\
            exti_irq_cb , RT_NULL);
    rt_pin_irq_enable(COL2_PIN , PIN_IRQ_ENABLE);

    rt_pin_attach_irq(COL3_PIN , PIN_IRQ_MODE_FALLING ,\
            exti_irq_cb , RT_NULL);
    rt_pin_irq_enable(COL3_PIN , PIN_IRQ_ENABLE);

    rt_pin_attach_irq(COL4_PIN , PIN_IRQ_MODE_FALLING ,\
            exti_irq_cb , RT_NULL);
    rt_pin_irq_enable(COL4_PIN , PIN_IRQ_ENABLE);
}

static void ub_ex_thead(void *param) {
    while (1) {
        rt_thread_delay(500);
    }
}

static int ub_ex(void) {
    rt_thread_t tid;

    tim3_init();
    //HAL_NVIC_EnableIRQ(TIM3_IRQn);
    pin_init();

    g_dev.filter_time = 0;
    g_dev.hold_active_time = 40;
    g_dev.button_irq_enabled = UB_EXTI_IRQ_USE;
    g_dev.repeat_speed = 30;

    g_dev.ub_device_read_callback = ub_dev_read_cb;
    g_dev.ub_event_callback = ub_even_cb;
    g_dev.ops.ub_exti_irq_control_callback = ub_exti_irq_control_cb;
    g_dev.ops.ub_time_irq_control_callback = ub_time_irq_control_cb;
    ub_device_init(&g_dev);

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
