/* File: startup_ARMCM4.S
 * Purpose: startup file for Cortex-M4 devices. Should use with
 *   GCC for ARM Embedded Processors
 * Version: V2.0
 * Date: 16 August 2013
 */
/* Copyright (c) 2011 - 2013 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
	.syntax	unified
	.arch	armv7e-m

	.extern __SRAM_segment_end__

	.section .heap
	.align 3
#ifdef __HEAP_SIZE
	.equ    Heap_Size, __HEAP_SIZE
#else
	.equ    Heap_Size, 0x66000
#endif
	.globl    __HeapBase
	.globl    __HeapLimit
__HeapBase:
	.space    Heap_Size
	.size __HeapBase, . - __HeapBase
__HeapLimit:
	.size __HeapLimit, . - __HeapLimit

	.section .isr_vector,"a",%progbits
	.align	4
	.globl	__isr_vector
	.global __Vectors

__Vectors:
__isr_vector:
	.long	__SRAM_segment_end__ - 4  /* Top of Stack at top of RAM*/
	.long	Reset_Handler         /* Reset Handler */
	.long	NMI_Handler           /* NMI Handler */
	.long	HardFault_Handler     /* Hard Fault Handler */
	.long	MemManage_Handler     /* MPU Fault Handler */
	.long	BusFault_Handler      /* Bus Fault Handler */
	.long	UsageFault_Handler    /* Usage Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	vPortSVCHandler       /* SVCall Handler */
	.long	DebugMon_Handler      /* Debug Monitor Handler */
	.long	0                     /* Reserved */
	.long	xPortPendSVHandler        /* PendSV Handler */
	.long	xPortSysTickHandler       /* SysTick Handler */

	/* External interrupts */
    .long    _irq_spurious         					 //     0  INT FIQ
    .long    _irq_spurious         					 //     1  INT IRQ
    .long    _irq_spurious          				 //     2  BT Page Timeout
    .long    _irq_spurious         					 //     3  BT SYNC DET INTR
    .long    _irq_spurious         					 //     4  BT PKD RX HDR
    .long    _irq_spurious         					 //     5  BT TIM INTR0
    .long    _irq_spurious         					 //     6  BT TIM INTR1
    .long    _irq_spurious         					 //     7  BT TIM INTR2
    .long    _irq_spurious         					 //     8  BT TIM INTR3
    .long    _irq_spurious         					 //     9  BT PKD INTR
    .long    _irq_spurious         					 //     10 BT PKA_INTR
    .long    _irq_spurious         					 //     11 BT AUX_TMR_INTR
    .long    _irq_spurious         					 //     12 BT ACCELERATOR INTR0
    .long    _irq_spurious         					 //     13 BT ACCELERATOR INTR1
    .long    _irq_spurious         					 //     14 BT ACCELERATOR INTR2
    .long    _irq_spurious         					 //     15 BT ACCELERATOR INTR3
    .long    _irq_spurious         					 //     16 BT ACCELERATOR INTR4
    .long    _irq_spurious							 //     17
    .long    _irq_spurious          				 //     18
    .long    smsg_irq_handler      					 //     19 INT IPI
    .long    _irq_spurious        					 //     20 INT AON INTC
    .long    _irq_spurious							 //     21
    .long    _irq_spurious							 //     22
    .long    _irq_spurious							 //     23
    .long    _irq_spurious							 //     24
    .long    _irq_spurious				             //     25
    .long    _irq_spurious				             //     26
    .long    _irq_spurious				             //     27
    .long    _irq_spurious				             //     28
    .long    _irq_spurious				             //     29
    .long    _irq_spurious				             //     30
    .long    _irq_spurious				             //     31
    .long    wifi_int_irq_dpd_handler				 //     32 INT DPD
    .long    wifi_int_irq_mac_handler				 //     33 INT MAC
    .long    _irq_spurious				             //     34
    .long    uart_uwp_isr				             //     35 INT UART0
    .long    _irq_spurious				             //     36 INT UART1
    .long    wifi_int_irq_com_tmr_handler			 //     37 INT REQ COM TMR
    .long    _irq_spurious				             //     38
    .long    _irq_spurious				             //     39
    .long    _irq_spurious				             //     40 INT WDG
    .long    _irq_spurious				             //     41
    .long    _irq_spurious				             //     42
    .long    _irq_spurious				             //     43
    .long    _irq_spurious				             //     44
    .long    _irq_spurious				             //     45
    .long    _irq_spurious				             //     46
    .long    _irq_spurious				             //     47
    .long    _irq_spurious				             //     48
    .long    _irq_spurious				             //     49
    .long    smsg_irq_handler				         //     50  NVIC_INT_GNSS2BTWF_IPI
    .long    _irq_spurious				             //     51
    .long    _irq_spurious				             //     52
    .long    _irq_spurious				             //     53
    .long    _irq_spurious				             //     54
    .long    _irq_spurious				             //     55
    .long    _irq_spurious				             //     56
    .long    _irq_spurious				             //     57
    .long    _irq_spurious				             //     58
    .long    _irq_spurious				             //     59
    .long    _irq_spurious				             //     60
    .long    _irq_spurious				             //     61
    .long    _irq_spurious				             //     62
    .long    _irq_spurious				             //     63
    .long    _irq_spurious				             //     64
    .long    _irq_spurious				             //     65
    .long    _irq_spurious				             //     66
    .long    _irq_spurious				             //     67
    .long    _irq_spurious				             //     68
    .long    _irq_spurious				             //     69
    .long    _irq_spurious				             //     70
    .long    _irq_spurious				             //     71
    .long    _irq_spurious				             //     72
    .long    _irq_spurious				             //     73
    .long    _irq_spurious				             //     74
    .long    _irq_spurious				             //     75
    .long    _irq_spurious				             //     76
    .long    _irq_spurious				             //     77
    .long    _irq_spurious				             //     78
    .long    _irq_spurious				             //     79
    .long    _irq_spurious				             //     80
    .long    _irq_spurious				             //     81
    .long    _irq_spurious				             //     82
    .long    _irq_spurious				             //     83
    .long    _irq_spurious				             //     84
    .long    _irq_spurious				             //     85
    .long    _irq_spurious				             //     86
    .long    _irq_spurious				             //     87
    .long    _irq_spurious				             //     88
    .long    _irq_spurious				             //     89
    .long    _irq_spurious				             //     90
    .long    _irq_spurious				             //     91
    .long    _irq_spurious				             //     92
    .long    _irq_spurious				             //     93
    .long    _irq_spurious				             //     94
    .long    _irq_spurious				             //     95
    .long    _irq_spurious				             //     96
    .long    _irq_spurious				             //     97
    .long    _irq_spurious				             //     98
    .long    _irq_spurious				             //     99
    .long    _irq_spurious				             //     100
    .long    _irq_spurious				             //     101
    .long    _irq_spurious				             //     102
    .long    _irq_spurious				             //     103
    .long    _irq_spurious				             //     104
    .long    _irq_spurious				             //     105
    .long    _irq_spurious				             //     106
    .long    _irq_spurious				             //     107
    .long    _irq_spurious				             //     108
    .long    _irq_spurious				             //     109
    .long    _irq_spurious				             //     110
    .long    _irq_spurious				             //     111
    .long    _irq_spurious				             //     112
    .long    _irq_spurious				             //     113
    .long    _irq_spurious				             //     114
    .long    _irq_spurious				             //     115
    .long    _irq_spurious				             //     116
    .long    _irq_spurious				             //     117
    .long    _irq_spurious				             //     118
    .long    _irq_spurious				             //     119
    .long    _irq_spurious				             //     120
    .long    _irq_spurious				             //     121
    .long    _irq_spurious				             //     122
    .long    _irq_spurious				             //     123
    .long    _irq_spurious				             //     124
    .long    _irq_spurious				             //     125
    .long    _irq_spurious				             //     126
    .long    _irq_spurious				             //     127

	.text
	.thumb
	.thumb_func
	.align	2
	.globl __start
	.extern app_main
	.globl	Reset_Handler
	.type	Reset_Handler, %function
__start:
Reset_Handler:
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    _etext: LMA of start of the section to copy from. Usually end of text
 *    _sdata: VMA of start of the section to copy to
 *    _edata: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
	ldr sp, =__SRAM_segment_end__
	sub sp, sp, #4

	ldr	r1, =_etext
	ldr	r2, =_sdata
	ldr	r3, =_edata

.L_loop1:
	cmp	r2, r3
	ittt	lt
	ldrlt	r0, [r1], #4
	strlt	r0, [r2], #4
	blt	.L_loop1

	ldr r1, =__ramfunc_start__
	ldr r2, =__ramtext_start__
	ldr r3, =__ramtext_end__

.L_loop2:
	cmp	r2, r3
	ittt	lt
	ldrlt	r0, [r1], #4
	strlt	r0, [r2], #4
	blt	.L_loop2

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */

 /*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    _sbss: start of the BSS section.
 *    _ebss: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
	ldr	r1, =_sbss
	ldr	r2, =_ebss

	movs	r0, 0
.L_loop3:
	cmp	r1, r2
	itt	lt
	strlt	r0, [r1], #4
	blt	.L_loop3

#ifndef __NO_SYSTEM_INIT
/*	bl	SystemInit */
#endif

	bl	app_main

	.pool
	.size	Reset_Handler, . - Reset_Handler

	.align	1
	.thumb_func
	.weak	Default_Handler
	.type	Default_Handler, %function
Default_Handler:
	b	.
	.size	Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
	.macro	def_irq_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm

	def_irq_handler	NMI_Handler
	def_irq_handler	HardFault_Handler
	def_irq_handler	MemManage_Handler
	def_irq_handler	BusFault_Handler
	def_irq_handler	UsageFault_Handler
//	def_irq_handler	SVC_Handler
	def_irq_handler	DebugMon_Handler
//	def_irq_handler	PendSV_Handler
//	def_irq_handler	SysTick_Handler
	def_irq_handler	DEF_IRQHandler
	.end
