/*
 * start_gcc.S
 * author: never
 */
.equ Mode_USR,        0x10
.equ Mode_FIQ,        0x11
.equ Mode_IRQ,        0x12
.equ Mode_SVC,        0x13
.equ Mode_ABT,        0x17
.equ Mode_UND,        0x1B
.equ Mode_SYS,        0x1F

.equ I_Bit,           0x80            @ when I bit is set, IRQ is disabled
.equ F_Bit,           0x40            @ when F bit is set, FIQ is disabled

.equ UND_Stack_Size,     0x00000000
.equ SVC_Stack_Size,     0x00000400
.equ ABT_Stack_Size,     0x00000000
.equ RT_FIQ_STACK_PGSZ,  0x00000000
.equ RT_IRQ_STACK_PGSZ,  0x00000800
.equ USR_Stack_Size,     0x00000400

//#define ISR_Stack_Size(UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + RT_FIQ_STACK_PGSZ + RT_IRQ_STACK_PGSZ + USR_Stack_Size)
#define ISR_Stack_Size  (UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + \
                         RT_FIQ_STACK_PGSZ + RT_IRQ_STACK_PGSZ)
.bss
.align 2
.global stack_start
.global stack_top

stack_start:
.rept ISR_Stack_Size
.byte 0
.endr
stack_top:

.global _reset
_reset:
    msr cpsr_c, #Mode_SVC|I_Bit|F_Bit // 和cps有啥区别？

    /* stack setup */
    bl stack_setup

    /* bss clear */
    ldr r0, =__bss_start
    ldr r1, =__bss_stop
    mov r2, #0

bss_loop:
    str r2, [r0], #4
    cmp r0, r1
    blo bss_loop //小于

    /* ctor */
    ldr r0, =__ctors_start__
    ldr r1, =__ctors_end__

ctor_loop:
    stmfd sp!, {r0-r1}
    bl [r0] // 逐个调用
    ldmfd sp!, {r0-r1}
    add r0, #4
    cmp r0, r1
    ble ctor_loop

    /* kernel startup */
    bl rtthread_startup

.align 5
.global vector_fiq
vector_fiq:
    stmfd sp!, {r0-r7, lr}
    bl rt_hw_trap_fiq
    ldmfd sp!, {r0-r7, lr}
    sub pc, lr, #4 // 这里lr要注意的

.align 5
.global vector_irq
vector_irq:
    stmfd sp!, {r0-r12, lr}

    bl rt_interrupt_enter
    bl rt_hw_trap_irq
    bl rt_interrupt_leave

    ldr r0, =rt_thread_switch_interrupt_flag
    ldr r1, [r0]
    cmp r1, #1
    beq rt_hw_context_switch_to

    ldmfd sp!, {r0-r12, lr}
    sub pc, lr, #4

rt_hw_context_switch_to:

.macro push_svc_reg
    sub sp, #17 * 4
    stmia sp, {r0-r12}
    mov r0, sp
    mrs r1, spsr
    str r1, [r0, #16*4]
    str lr, [r0, #15*4]
    cps #Mode_SVC // 切换svc模式
    str lr, [r0, #14*4] // 先压lr行不行？
    str sp, [r0, #13*4]
.endm

.align 5
.global vector_undef
vector_undef:
    bl rt_hw_trap_undef
	push_svc_reg
    b .

.align 5
.global vector_swi
vector_swi:
    bl rt_hw_trap_swi
	push_svc_reg
    b .

.align 5
.global vector_pabt
vector_pabt:
    bl rt_hw_trap_pabt
	push_svc_reg
    b .

.align 5
.global vector_dabt
vector_dabt:
    bl rt_hw_trap_dabt
	push_svc_reg
    b .

.align 5
.global vector_resv
vector_resv:
    bl rt_hw_trap_resv
	push_svc_reg
    b .
