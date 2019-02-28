/*
 * uwp_uart.h
 *
 *  Created on: 2019Äê2ÔÂ27ÈÕ
 *      Author: xiangkai.gao
 */

#ifndef LIB_THIRD_PARTY_UNISOC_UWP566X_COMPONENTS_UWP566X_BSP_UART_UWP_UART_H_
#define LIB_THIRD_PARTY_UNISOC_UWP566X_COMPONENTS_UWP566X_BSP_UART_UWP_UART_H_

#include "serial_api.h"

#ifdef __cplusplus
extern "C"{
#endif

void vMyUARTOutput(char *DataToOutput, size_t LengthToOutput);

#ifdef __cplusplus
}
#endif

#endif /* LIB_THIRD_PARTY_UNISOC_UWP566X_COMPONENTS_UWP566X_BSP_UART_UWP_UART_H_ */
