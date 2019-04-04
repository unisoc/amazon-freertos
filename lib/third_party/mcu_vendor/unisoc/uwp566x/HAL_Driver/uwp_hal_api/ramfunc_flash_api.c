/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>

#include "hal_sfc.h"
#include "hal_config_def.h"
#include "mbed_retarget.h"
#include "hal_ramfunc.h"
#include "uwp_rtos_posix.h"

#define FLASH_WRITE_BLOCK_SIZE 0x1

/*
 * This is named flash_uwp_lock instead of flash_uwp_lock (and
 * similarly for flash_uwp_unlock) to avoid confusion with locking
 * actual flash pages.
 */

static struct spi_flash_struct uwp_flash_dev;
static k_sem_t flash_op_sem;
#if 1
static inline int flash_uwp_lock(void)
{
    if ( k_sem_take(flash_op_sem, K_FOREVER) == RTOS_RETURN_VALUE_SUCCESS )
        return 0;
    return -1;
}

static inline int flash_uwp_unlock(void)
{
    if ( k_sem_give(flash_op_sem) == RTOS_RETURN_VALUE_SUCCESS )
        return 0;
    return -1;
}

int flash_uwp_write_protection(bool enable)
{
    int ret = -1;

    if(enable){
        if ( k_sem_take(flash_op_sem, K_FOREVER) == RTOS_RETURN_VALUE_SUCCESS )
            ret = 0;
    }
    else{
        if ( k_sem_give(flash_op_sem) == RTOS_RETURN_VALUE_SUCCESS )
            ret = 0;
    }

    return ret;
}

int flash_uwp_read(uint32_t offset, void *data, uint32_t len)
{
    int ret = 0;
    struct spi_flash *flash = &(uwp_flash_dev.flash);

    if (!len) {
        return 0;
    }

    ret = flash->read(flash, ((u32_t)CONFIG_FLASH_BASE_ADDRESS + offset),
        (u32_t *)data, len, READ_SPI_FAST);

    return ret;
}

int flash_uwp_erase(uint32_t offset, uint32_t len)
{
    int ret;
    unsigned int key;
    struct spi_flash *flash = &(uwp_flash_dev.flash);

    if (!len) {
        return 0;
    }

    if (flash_uwp_lock()) {
        return -EACCES;
    }

    key = irq_lock_primask();
    ret = flash->erase(flash, ((u32_t)CONFIG_FLASH_BASE_ADDRESS + offset),
            len);
    if(!ret){
        cache_invalid_range_hal((u8_t *)(CONFIG_FLASH_BASE_ADDRESS + offset), len);
        dcache_clean_range_hal((u8_t *)(CONFIG_FLASH_BASE_ADDRESS + offset), len);
    }
    irq_unlock_primask(key);

    ret = flash_uwp_unlock();

    return ret;
}

int flash_uwp_write(uint32_t offset, const void *data, uint32_t len)
{
    int ret;
    unsigned int key;
    struct spi_flash *flash = &(uwp_flash_dev.flash);

    if (!len) {
        return 0;
    }

    if (flash_uwp_lock()) {
        return -EACCES;
    }

    key = irq_lock_primask();
    ret = flash->write(flash, ((u32_t)CONFIG_FLASH_BASE_ADDRESS + offset),
            len, data);
    if(!ret){
        cache_invalid_range_hal((u8_t *)(CONFIG_FLASH_BASE_ADDRESS + offset), len);
        dcache_clean_range_hal((u8_t *)(CONFIG_FLASH_BASE_ADDRESS + offset), len);
    }
    irq_unlock_primask(key);

    ret = flash_uwp_unlock();

    return ret;
}
#endif
int uwp_flash_init(void)
{
    int ret = 0;

    struct spi_flash_params *params = uwp_flash_dev.params;
    struct spi_flash *flash = &(uwp_flash_dev.flash);

    spiflash_select_xip(FALSE);

    sfcdrv_intcfg(FALSE);

    spiflash_reset_anyway();

    spiflash_set_clk();

    ret = uwp_spi_flash_init(flash, &params);
    if (ret) {
        printk("uwp spi flash init failed. ret:[%d]\n", ret);
        return ret;
    }

    return ret;
}
#if 1
extern void vUWP5661FLASHPartitionInit(void);
int flash_init_supplement(void){
    int ret = -1;
    k_sem_init( flash_op_sem, 1, 0);
    vUWP5661FLASHPartitionInit();
    return (flash_op_sem == NULL ? -1 : 0);
}
#endif
