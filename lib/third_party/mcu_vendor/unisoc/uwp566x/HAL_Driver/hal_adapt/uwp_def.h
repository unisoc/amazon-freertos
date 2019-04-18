/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __UWP_DEF_H
#define __UWP_DEF_H

#ifdef __cplusplus
extern "C" {
#endif


#define TRUE  (1)
#define FALSE (0)
#define BIT(nr) (1UL << (nr))
#define CLR_BIT(reg, bit) ((reg) &= ~(1<<(bit)))
#define SET_BIT(reg, bit) ((reg) |=  (1<<(bit)))

#define set_bits(value, addr)	\
	(*(volatile unsigned long *)(addr)) |= (value)

#define clr_bits(value, addr)	\
	(*(volatile unsigned long *)(addr)) &= ~(value)

#if defined(__arm__) && defined(__ARMCC_VERSION) 
    /* Keil uVision4 tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __align(n)
#elif defined (__IAR_SYSTEMS_ICC__) 
    /* IAR Embedded Workbench tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define IAR_STR(a) #a
    #define ALIGNED(n) _Pragma(IAR_STR(data_alignment= ## n ##))
#else 
    /* GCC tools (CodeSourcery) */
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __attribute__((aligned (n)))
#endif

#ifdef __cplusplus
}
#endif

#endif
