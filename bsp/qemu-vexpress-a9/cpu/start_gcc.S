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

.text
.global _reset
_reset:
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #Mode_SVC
    msr cpsr_c, r0

    /* stack setup */
    bl stack_setup

    /* bss clear */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0

bss_loop:
    cmp r0, r1
    beq bss_end
    str r2, [r0], #4
    b bss_loop
bss_end:

    /* ctor */
    ldr r0, =__ctors_start__
    ldr r1, =__ctors_end__

ctor_loop:
    cmp r0, r1
    beq ctor_end
    ldr r2, [r0], #4
    stmfd sp!, {r0-r1}
    blx r2
    ldmfd sp!, {r0-r1}
    b ctor_loop
ctor_end:

    /* kernel startup */
    bl rtthread_startup

.align 2
stack_setup:
    /* svc mode stack */
    ldr sp, =stack_top
    mov r0, sp

    /* udf mode */
    msr cpsr_c, #Mode_UND | I_Bit | F_Bit
    sub r0, #UND_Stack_Size //0
    mov sp, r0

    /* abt mode */
    msr cpsr_c, #Mode_ABT | I_Bit | F_Bit
    sub r0, #ABT_Stack_Size
    mov sp, r0

    /* fiq mode */
    msr cpsr_c, #Mode_FIQ | I_Bit | F_Bit
    sub r0, #RT_FIQ_STACK_PGSZ
    mov sp, r0

    /* irq mode */
    msr cpsr_c, #Mode_IRQ | I_Bit | F_Bit
    sub r0, #RT_IRQ_STACK_PGSZ
    mov sp, r0

    msr cpsr_c, #Mode_SVC
    bx lr

.section .text.isr, "ax"

.align 5
.global vector_fiq
vector_fiq:
    stmfd sp!, {r0-r7, lr}
    bl rt_hw_trap_fiq
    ldmfd sp!, {r0-r7, lr}
    subs pc, lr, #4 // 这里lr要注意的

.align 5
.global vector_irq
vector_irq:
    // a任务发生中断，保存a的context
    // 此时是irq模式，保存在irq的栈里
    sub lr, #4
    stmfd sp!, {r0-r12, lr}

    bl rt_interrupt_enter
    bl rt_hw_trap_irq
    bl rt_interrupt_leave

    // flag在schedule里nest=1时，context_switch_interrupt里设置
    // 中断处理里，发生调度，此时不能切换，flag置位
    ldr r0, =rt_thread_switch_interrupt_flag
    ldr r1, [r0]
    cmp r1, #1
    beq rt_hw_context_switch_interrupt_do

    // 中断处理完成，弹栈a的context，返回a继续运行
    ldmfd sp!, {r0-r12, lr}
    //subs pc, lr, #4 // irq的lr -->pc，且同时irq的spsr -->cpsr
    movs pc, lr

// a任务发生中断，中断处理完成，切换b
// 将a的context弹栈，保存到svc的栈里
rt_hw_context_switch_interrupt_do:
    mov r1, #0
    str r1, [r0]

    mov r0, sp
    mrs r1, spsr // irq的spsr

    msr cpsr_c, #Mode_SVC|I_Bit|F_Bit

    add r0, #14*4
    ldmdb r0!, {r2}
    stmfd sp!, {r2} // pc
    ldmdb r0!, {r4-r12}
    stmfd sp!, {r4-r12, lr}
    ldmdb r0!, {r4-r7}
    stmfd sp!, {r4-r7}
    stmfd sp!, {r1}

    ldr r0, =rt_interrupt_from_thread
    ldr r1, [r0]
    str sp, [r1]

    cps #Mode_IRQ
    add sp, #14*4
    cps #Mode_SVC

    ldr r0, =rt_interrupt_to_thread
    ldr r1, [r0]
    ldr sp, [r1]

    ldmfd sp!, {r0}
    msr spsr_cxsf, r0

    ldmfd sp!, {r0-r12, lr, pc}^

.macro push_svc_reg
    sub sp, #17 * 4
    stmia sp, {r0-r12}
    mov r0, sp
    mrs r1, spsr
    str r1, [r0, #16*4] // pc
    str lr, [r0, #15*4] // lr
    cps #Mode_SVC // 切换svc模式
    str sp, [r0, #13*4]
    str lr, [r0, #14*4] // 先压lr行不行？
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
