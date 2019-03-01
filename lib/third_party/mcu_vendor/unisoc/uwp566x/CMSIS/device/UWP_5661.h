/******************************************************************************
 * @file     UWP_5661.h
 * @brief    CMSIS Cortex-M4 Core Peripheral Access Layer Header File for
 *           UNISOC UWP566X Device Series
 * @version: V1.09
 * @date:    07. June 2018
 *
 * @note
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M
 * processor based microcontrollers.  This file can be freely distributed
 * within development tools that are supporting such ARM based processors.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/


#ifndef MBED_UWP_5661H_H
#define MBED_UWP_5661H_H

#ifdef __cplusplus
    extern "C" {
#endif

/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */

typedef enum IRQn
{
/******  Cortex-M4 Processor Exceptions Numbers **********************************************************/
  NonMaskableInt_IRQn                  = -14,      /*!< 2 Non Maskable Interrupt                         */
  MemoryManagement_IRQn                = -12,      /*!< 4 Cortex-M4 Memory Management Interrupt          */
  BusFault_IRQn                        = -11,      /*!< 5 Cortex-M4 Bus Fault Interrupt                  */
  UsageFault_IRQn                      = -10,      /*!< 6 Cortex-M4 Usage Fault Interrupt                */
  SVCall_IRQn                          = -5,       /*!< 11 Cortex-M4 SV Call Interrupt                   */
  DebugMonitor_IRQn                    = -4,       /*!< 12 Cortex-M4 Debug Monitor Interrupt             */
  PendSV_IRQn                          = -2,       /*!< 14 Cortex-M4 Pend SV Interrupt                   */
  SysTick_IRQn                         = -1,       /*!< 15 Cortex-M4 System Tick Interrupt               */

/******  RDA5991H Specific Interrupt Numbers *************************************************************/
  INT_FIQ_IRQn                         = 0,        /*!< FIQ Interrupt                                    */
  INT_IRQ_IRQn				           = 1, 	   /*!< IRQ Flash Interrupt			     			     */
  BT_MASKED_PAGE_TIMEOUT_INTR_IRQn     = 2,        /*!<                                                  */
  BT_MASKED_SYNC_DET_INTR_IRQn         = 3,        /*!<                                                  */
  BT_MASKED_PKD_RX_HDR_IRQn            = 4,        /*!<                                                  */
  BT_MASKED_TIM_INTR0_IRQn             = 5,        /*!<                                                  */
  BT_MASKED_TIM_INTR1_IRQn             = 6,        /*!<                                                  */
  BT_MASKED_TIM_INTR2_IRQn             = 7,        /*!<                                                  */
  BT_MASKED_TIM_INTR3_IRQn             = 8,        /*!<                                                  */
  BT_MASKED_PKD_INTR_IRQn              = 9,        /*!<                                                  */
  BT_MASKED_PKA_INTR_IRQn              = 10,       /*!<                                                  */
  BT_MASKED_AUX_TMR_INTR_IRQn          = 11,       /*!<                                                  */
  BT_ACCELERATOR_INTR0_IRQn            = 12,       /*!<                                                  */
  BT_ACCELERATOR_INTR1_IRQn            = 13,       /*!<                                                  */
  BT_ACCELERATOR_INTR2_IRQn            = 14,       /*!<                                                  */
  BT_ACCELERATOR_INTR3_IRQn            = 15,       /*!<                                                  */
  BT_ACCELERATOR_INTR4_IRQn            = 16,       /*!<                                                  */       
  WCI2_IRQn                            = 18,       /*!< WCI2 Interrupt                                   */
  INT_IPI_IRQn						   = 19,	   /*!< IPI Interrupt								     */
  AON_IRQn                             = 20,       /*!< AON Interrupt                                    */
  TIMER0_IRQn                          = 21,       /*!< Group TIMER0 Interrupt                           */
  TIMER1_IRQn                          = 22,       /*!< Group TIEMR1 Interrupt                           */
  TIMER2_IRQn                          = 23,       /*!< Group TIMER2 Interrupt                           */
  SYSTEMTIMER_IRQn                     = 26,       /*!< GPIO Interrupt                                   */
  REQ_WIFI_CAP_IRQn                    = 31,       /*!< REQ_WIFI_CAP interrupt                           */
  DPD_IRQn                             = 32,       /*!< WiFi DPD Interrupt                               */
  MAC_IRQn                             = 33,       /*!< WiFi MAC Interrupt                               */
  UART0_IRQn                           = 35,       /*!< UART0 Interrupt                                  */
  UART1_IRQn                           = 36,       /*!< UART1 Interrupt                                  */
  COMTMR_IRQn                          = 37,       /*!< Common Timer Interrupt                           */
  WATCHDOG_IRQn                        = 40,       /*!< WATCHDOG Interrupt                               */
  GNSS2BTWIFI_IPI_IRQn                 = 50,       /*!< GNSS2BTWIFI_IPI Interrupt                        */
} IRQn_Type;


/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/* Configuration of the Cortex-M4 Processor and Core Peripherals */
#define __MPU_PRESENT             0         /*!< MPU present or not                               */
#define __NVIC_PRIO_BITS          6         /*!< Number of Bits used for Priority Levels          */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used     */
#define __FPU_PRESENT             1         /*!< FPU present                                      */


#include "core_cm4.h"                       /* Cortex-M4 processor and core peripherals           */
//#include "system_UWP5661.h"                 /* System Header                                      */


/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/

#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif

/******************************************************************************/
/*                 data type header and bit operation definition              */
/******************************************************************************/
//#include "uwp_def.h"

/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/

/* Base addresses                                                             */
#define UWP_IRAM_BASE            (0x00100000UL)
#define UWP_DRAM_BASE            (0x00180000UL)
#define UWP_FLASH_BASE           (0x02000000UL)
#define NVIC_RAM_VECTOR_ADDRESS  UWP_IRAM_BASE
#define NVIC_NUM_VECTORS         ( 16 + 128 )

/* peripheral base address definition                                         */


#ifdef __cplusplus
}
#endif

#endif /* UWP_5661_H */
