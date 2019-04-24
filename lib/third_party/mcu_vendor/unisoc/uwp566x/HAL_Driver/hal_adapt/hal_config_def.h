#ifndef __HAL_CONFIG_DEF_H
#define __HAL_CONFIG_DEF_H

/* soc target */
#define CONFIG_SOC_UWP5661
//#define CONFIG_SOC_UWP5662

/* Provide interfaces to initialize SRAM. */
#define CONFIG_USE_UWP_HAL_SRAM


#define CONFIG_FLASH_BASE_ADDRESS (0x02000000UL)
#define CONFIG_UWP_PKT_BUF_SIZE   (1600)
#define CONFIG_UWP_PKT_BUF_MAX    (120)

#endif
