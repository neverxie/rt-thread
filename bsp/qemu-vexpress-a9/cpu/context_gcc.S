/*
 * never
 *
 */

.section .text, "ax"

.global rt_hw_interrupt_disable
/* rt_hw_interrupt_disable(void) */
rt_hw_interrupt_disable:
    mrs r0, cpsr
    cpsid i
    bx lr

.global rt_hw_interrupt_enable
/* rt_hw_interrupt_enable(rt_base_t level) */
rt_hw_interrupt_enable:
    msr cpsr_c, r0
    bx lr

.global rt_hw_context_switch_to
/* void rt_hw_context_switch_to(rt_uint32_t to); */
rt_hw_context_switch_to:
    ldr sp, [r0]
    ldmfd sp!, {r2}
    // bug1
    msr spsr_cxsf, r2
    ldmfd sp!, {r0-r12, lr, pc}^

.section .text.isr, "ax"
.global rt_hw_context_switch
/* void rt_hw_context_switch(rt_uint32_t from, rt_uint32_t to); */
rt_hw_context_switch:
    // 保存from
    stmfd sp!, {lr}
    stmfd sp!, {r0-r12, lr}
    // bug2
    mrs r2, cpsr
    tst lr, #0x01
    orrne r2, #0x20 // set thumb mode
    stmfd sp!, {r2}
    str sp, [r0]

    // 恢复to
    ldr sp, [r1]
    ldmfd sp!, {r2}
    msr spsr_cxsf, r2
    ldmfd sp!, {r0-r12, lr, pc}^

/* void rt_hw_context_switch_interrupt(rt_uint32_t from, rt_uint32_t to); */
.global rt_hw_context_switch_interrupt
rt_hw_context_switch_interrupt:
    ldr r2, =rt_thread_switch_interrupt_flag
    ldr r3, [r2]
    cmp r3, #1
    beq irq_flag_set
    mov r3, #1
    str r3, [r2]

irq_flag_set:
    ldr r3, =rt_interrupt_from_thread
    str r0, [r3]

    // bug3
    /*
     * ldr r2, [r0]
     * ldr r3, =rt_interrupt_from_thread
     * ldr r4, [r3]
     * str r2, [r4]
     */
    ldr r3, =rt_interrupt_to_thread
    str r1, [r3]

    // bug4
    bx lr
