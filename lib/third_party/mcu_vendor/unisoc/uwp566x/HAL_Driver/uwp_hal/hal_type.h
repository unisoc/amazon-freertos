/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __HAL_TYPE_H
#define __HAL_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __ramfunc

typedef uint32_t       u32_t;
typedef int32_t        s32_t;
typedef unsigned short u16_t;
typedef signed   short s16_t;
typedef unsigned char  u8_t;
typedef signed   char  s8_t;
typedef u32_t          mem_addr_t;

#ifdef __cplusplus
}
#endif

#endif
