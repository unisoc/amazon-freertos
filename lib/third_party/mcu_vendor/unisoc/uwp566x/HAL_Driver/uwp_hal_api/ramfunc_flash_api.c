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

#define FLASH_WRITE_BLOCK_SIZE 0x1

/*
 * This is named flash_uwp_lock instead of flash_uwp_lock (and
 * similarly for flash_uwp_unlock) to avoid confusion with locking
 * actual flash pages.
 */

static struct spi_flash_struct uwp_flash_dev;
static void *flash_op_sem = NULL;
#if 0
static inline int flash_uwp_lock(void)
{
    return k_sem_acquire(flash_op_sem, osWaitForever);
}

static inline int flash_uwp_unlock(void)
{
    return k_sem_release(flash_op_sem);
}

int flash_uwp_write_protection(bool enable)
{
    int ret = 0;
    if(enable)
        ret = k_sem_acquire(flash_op_sem, osWaitForever);
    else
        ret = k_sem_release(flash_op_sem);

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
        mbed_error_printf("uwp spi flash init failed. ret:[%d]\n", ret);
        return ret;
    }

    return ret;
}
#if 0
int flash_init_supplement(void){
    int ret = -1;
    flash_op_sem = k_sem_create( 1, 0);
    return (flash_op_sem == NULL ? -1 : 0);
}
#endif
