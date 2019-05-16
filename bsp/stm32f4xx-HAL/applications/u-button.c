/*
 * File       : u-button.c
 *
 * Change Logs:
 * Date         Author      Notes
 * 2019-05-04   never       the first version
 */
#include <rtthread.h>
#include "u-button.h"
#include <rtdevice.h>

// todo: exti中断的问题
// todo: 非中断模式的问题

static ub_dev_t g_dev;

enum U_BUTTON_STATE {
    UB_STATE_INIT = 0,
    UB_STATE_UNSTABLE,
    UB_STATE_STABLE,
    UB_STATE_WAIT_BUTTON_UP,
    UB_STATE_WAIT_BUTTON_DOWN_AGAIN,
};

static void __ub_dev_state_reset(ub_dev_t *dev) {
    uint32_t i, dev_read = 0;

    // todo: 是否需要判断还有按键没有弹起？
    for (i = 0; i < BUTTON_NUM_MAX; i++) {
        dev->btn[i].state = UB_STATE_INIT;
        dev->btn[i].ticks = 0;
        dev->btn[i].repeat = 0;
    }

    // 什么时候应该关中断？所有按键都弹起
    if (dev->ops.ub_time_irq_control_callback != NULL) {
        dev->ops.ub_time_irq_control_callback(UB_TIME_IRQ_DISABLE);
    }

    if (dev->ops.ub_exti_irq_control_callback != NULL) {
        dev->ops.ub_exti_irq_control_callback(UB_EXTI_IRQ_ENABLE);
    }

}

/**
 * This is the core of the universal button driver, please call is periodically.
 *
 * @param dev
 */
void ub_device_state_handle(ub_dev_t *dev) {
    uint32_t i, dev_read = 0;

    if (dev == NULL) {
        return;
    }

    if (dev->ub_device_read_callback != NULL) {
        dev_read = dev->ub_device_read_callback(); // 应该在哪里调用？
    }

    for (i = 0; i < BUTTON_NUM_MAX; i++) {

        if (dev->btn[i].state > 0) {
            dev->btn[i].ticks++;
        }

        switch (dev->btn[i].state) {
            case UB_STATE_INIT:
                if (dev_read & (1 << i)) { // button down
                    dev->btn[i].state = UB_STATE_UNSTABLE;
                }
                break;

            case UB_STATE_UNSTABLE:
                if (dev_read & (1 << i)) { // button down
                    rt_kprintf("button down\n");
                    if (dev->ub_event_callback != NULL) {
                        dev->ub_event_callback(dev);
                    }

                    dev->btn[i].state = UB_STATE_STABLE;
                    dev->btn[i].repeat++;
                    if (dev->btn[i].repeat > 1) {
                        rt_kprintf("button repeat\n");
                        if (dev->ub_event_callback != NULL) {
                            dev->ub_event_callback(dev);
                        }
                    }
                } else {
                    if (dev->btn[i].repeat == 0) {
                        __ub_dev_state_reset(dev);
                    } else {
                        dev->btn[i].state = UB_STATE_WAIT_BUTTON_DOWN_AGAIN;
                    }
                }
                break;

            case UB_STATE_STABLE:
                if ((dev_read & (1 << i)) == 0) { // up
                    rt_kprintf("button up\n");
                    if (dev->ub_event_callback != NULL) {
                        dev->ub_event_callback(dev);
                    }
                    dev->btn[i].state = UB_STATE_WAIT_BUTTON_DOWN_AGAIN;
                } else {
                    if (dev->btn[i].ticks > dev->hold_active_time) {
                        rt_kprintf("hold\n");
                        if (dev->ub_event_callback != NULL) {
                            dev->ub_event_callback(dev);
                        }
                        dev->btn[i].state = UB_STATE_WAIT_BUTTON_UP;
                    }
                }
                break;

            case UB_STATE_WAIT_BUTTON_UP:
                if ((dev_read & (1 << i)) == 0) {
                    rt_kprintf("button up\n");
                    if (dev->ub_event_callback != NULL) {
                        dev->ub_event_callback(dev);
                    }
                    __ub_dev_state_reset(dev);
                }
                break;

            case UB_STATE_WAIT_BUTTON_DOWN_AGAIN:
                rt_kprintf("ticks: %d\n", dev->btn[i].ticks);
                if (dev->btn[i].ticks > dev->repeat_speed) {
                    // 超时了，应该输出单击，双击，返回init了
                    rt_kprintf("repeat click: %d\n", dev->btn[i].repeat);
                    if (dev->ub_event_callback != NULL) {
                        dev->ub_event_callback(dev);
                    }
                    __ub_dev_state_reset(dev);
                } else {
                    dev->btn[i].state = UB_STATE_UNSTABLE;
                }
                break;

            default:
                break;
        }
    }
}

/**
 * This function is to set the unstable state(UB_STATE_UNSTABLE)
 * of the state machine. If you use interrupt mode(button_irq_enabled set to 1),
 * please put this function in the interrupt handler, otherwise ignore it.
 *
 * @param dev
 */
void ub_device_state_set(ub_dev_t *dev) {
    uint8_t i;
    uint32_t dev_read = 0;

    if (dev->ub_device_read_callback != NULL) {
        dev_read = dev->ub_device_read_callback();
    }

    for (i = 0; i < BUTTON_NUM_MAX; i++) {
        // 有按键按下
        if (dev_read & (1 << i)) {
            dev->btn[i].state = UB_STATE_UNSTABLE;
            dev->btn[i].ticks = 0;
            dev->btn[i].repeat = 0;

            // 这个需要注意！
            if (dev->ops.ub_exti_irq_control_callback != NULL) {
                dev->ops.ub_exti_irq_control_callback(UB_EXTI_IRQ_DISABLE);
            }

            if (dev->ops.ub_time_irq_control_callback != NULL) {
                dev->ops.ub_time_irq_control_callback(UB_TIME_IRQ_ENABLE);
            }
        }
    }
}

/**
 * This is the initialzation function of the universal button.
 *
 * @param dev
 *
 * @return the operation status, UB_OK on successful
 */
ub_err_t ub_device_init(ub_dev_t *dev) {
    uint32_t i;

    if (dev == NULL) {
        goto __exit;
    }

    g_dev.filter_time = dev->filter_time;
    g_dev.hold_active_time = dev->hold_active_time;
    g_dev.repeat_speed = dev->repeat_speed;
    g_dev.button_irq_enabled = dev->button_irq_enabled;

    for (i = 0; i < BUTTON_NUM_MAX; i++) {
        g_dev.btn[i].ticks = 0;
        g_dev.btn[i].repeat = 0;
        g_dev.btn[i].state = UB_STATE_INIT;
    }

    if (g_dev.button_irq_enabled) {
        g_dev.ops.ub_exti_irq_control_callback = dev->ops.ub_exti_irq_control_callback;
        g_dev.ops.ub_time_irq_control_callback = dev->ops.ub_time_irq_control_callback;
    } else {
        g_dev.ops.ub_exti_irq_control_callback = NULL;
        g_dev.ops.ub_time_irq_control_callback = NULL;
    }

    g_dev.ub_event_callback = dev->ub_event_callback;

    return UB_OK;

__exit:
    return -UB_ERR;
}
