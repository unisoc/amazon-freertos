#ifndef __HAL_RAMFUNC_H
#define __HAL_RAMFUNC_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>

#include "hal_type.h"

static inline void sci_write32(mem_addr_t addr, u32_t data);

#define sci_reg_and(reg, val) \
    sci_write32(reg, (sci_read32(reg) & val))

#define sci_reg_or(reg, val) \
    sci_write32(reg, (sci_read32(reg) | val))

#define sci_glb_set(reg, val) \
    sci_write32(__REG_SET_ADDR(reg), val)

#define sci_glb_clr(reg, val) \
    sci_write32(__REG_CLR_ADDR(reg), val)

static inline u32_t sci_read32(mem_addr_t addr)
{
    return *(volatile u32_t *)addr;
}

static inline void sci_write32(mem_addr_t addr, u32_t data)
{
    *(volatile u32_t *)addr = data;
}

static inline u32_t sys_read32(mem_addr_t addr)
{
    return *(volatile u32_t *)addr;
}

static inline void sys_write32(u32_t data, mem_addr_t addr)
{
    *(volatile u32_t *)addr = data;
}

extern int flash_uwp_write_protection(bool enable);
extern int flash_uwp_erase(uint32_t offset, uint32_t len);
extern int flash_uwp_write(uint32_t offset, const void *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
