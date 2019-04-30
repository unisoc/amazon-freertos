/**************************************************************************//**
 * @file     system_UWP566X.c
 * @brief    CMSIS Cortex-M4 Device System Source File for
 *           UNISOC UWP566X Device Series
 * @version  V1.11
 * @date     12. June 2018
 *
 * @note
 * Copyright (C) 2009-2016 ARM Limited. All rights reserved.
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

#include "hal_type.h"
#include "UWP_5661.h"

/** @addtogroup UWP5661_System
 * @{
 */

/*
//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/

/*--------------------- Clock Configuration ----------------------------------
//
// <e> Clock Configuration
//   <h> Clock Gating Control 0 Register (CLKGATE0)
//     <o1.0>       DEEPSLEEP: Deep sleep mode enable
//     <o1.15>      EXIF: EXIF clock gating enable
//   </h>
//
//   <h> Clock Gating Control 1 Register (CLKGATE1)
//     <o2.0>       GPIO: GPIO clock gating enable
//     <o2.1>       I2S: I2S clock gating enable
//     <o2.2>       PWM: PWM clock gating enable
//     <o2.3>       TIMER: APB Timer clock gating enable
//     <o2.4>       PSRAM_PCLK: PSRAM PCLK clock gating enable
//     <o2.5>       SDMMC: SDMMC clock gating enable
//     <o2.6>       I2C: I2C clock gating enable
//     <o2.4>       PSRAM_HCLK: PSRAM HCLK clock gating enable
//   </h>
//
//   <h> Clock Gating Control 2 Register (CLKGATE2)
//     <o3.16>      I2SIN: I2SIN clock gating enable
//     <o3.17>      I2SOUT: I2SOUT clock gating enable
//     <o3.18>      GSPI: General SPI clock gating enable
//     <o3.19>      RFSPI: RF SPI clock gating enable
//     <o3.31>      SLOWFLASH: Slow flash clock gating enable
//   </h>
//
//   <h> Clock Gating Control 3 Register (CLKGATE3)
//     <o4.30>      DOZEMODE: Doze mode enable
//     <o4.31>      CLKMODE: Clock mode enable
//   </h>
//
//   <h> Clock Core Configure Register (CORECFG)
//     <o5.11>      HCLK: HCLK config
//     <o5.12..13>  CPUCLK: CPU Clock config
//   </h>
//
// </e>
*/



/** @addtogroup UWP566X_System_Defines  UWP566X System Defines
  @{
 */

/*
//-------- <<< end of configuration section >>> ------------------------------
*/

/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define   CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC                   (416000000UL)
#define   UNISOC_UART_40038000_CLOCK_FREQUENCY                 ( 26000000UL)


/**
 * @}
 */


/** @addtogroup UWP566X_System_Public_Variables  UWP566X System Public Variables
  @{
 */
/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC; /*!< System Clock Frequency (Core Clock)*/

/**
 * @}
 */


/** @addtogroup RDA5991H_System_Public_Functions  RDA5991H System Public Functions
  @{
 */

/**
 * Update SystemCoreClock variable
 *
 * @param  none
 * @return none
 *
 * @brief  Updates the SystemCoreClock with current core Clock
 *         retrieved from cpu registers.
 */
void SystemCoreClockUpdate (void)            /* Get Core/Bus Clock Frequency     */
{
    return;
}
#if 0
static inline void uwp_glb_init(void)
{
	/* Set system clock to 416M */
	/* 0x40088018 */
	sci_write32(REG_APB_EB, 0xffffffff); /* FIXME */
	/* 0x4083c050 */
	sci_glb_set(REG_AON_CLK_CTRL4, BIT_AON_APB_CGM_RTC_EN);
	/* 0x4083c024 */
	sci_write32(REG_AON_GLB_EB, 0xf7ffffff); /* FIXME */
	/* 0x40130004 */
	sci_reg_or(REG_AHB_EB0, BIT(0) | BIT(1));

	/* 0x4083c168 */
	sci_reg_and(REG_AON_PLL_CTRL_CFG, ~0xFFFF);
	sci_reg_or(REG_AON_PLL_CTRL_CFG, CLK_416M);
	/* 0x40844220 */
	sci_write32(REG_AON_CLK_RF_CGM_ARM_CFG,
			BIT_AON_CLK_RF_CGM_ARM_SEL(CLK_SRC5));

	/* 0x40130114 */
	sci_reg_or(REG_AHB_MTX_CTL1, BIT(22));
	/* 0x4083c038 */
	//sci_reg_and(REG_AON_PD_AT_RESET, ~BIT(0));
	/* 0x40844024 */
	sci_reg_and(REG_AON_CLK_PRE_DIV_PLL_WAIT_SEL0_CFG, ~BIT(2));
}
#endif
/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
extern void uwp_glb_init(void);
void SystemInit (void)
{
    uwp_glb_init();

#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));    /* set CP10, CP11 Full Access */
#endif /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */

    extern uint32_t __FLASH_ISR_VECTOR__;
    SCB->VTOR  = & __FLASH_ISR_VECTOR__;           /* vector table in flash      */
    NVIC_SetPriorityGrouping(0);                   /* 6 bit for pre-emption pri  */

#ifdef NVIC_RAM_VECTOR_ADDRESS
    uint32_t *old_vectors = (uint32_t *)SCB->VTOR;
    uint32_t *vectors = (uint32_t *)NVIC_RAM_VECTOR_ADDRESS;
    for (int i = 0; i < NVIC_NUM_VECTORS; i++) {
        vectors[i] = old_vectors[i];
    }
    SCB->VTOR = (uint32_t)NVIC_RAM_VECTOR_ADDRESS;
#endif /* NVIC_RAM_VECTOR_ADDRESS */

  //  __enable_irq();

    return;
}

/**
 * @}
 */

/**
 * @}
 */
