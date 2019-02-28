/*
 * uwp_led.h
 *
 *  Created on: 2019Äê2ÔÂ27ÈÕ
 *      Author: xiangkai.gao
 */

#ifndef LIB_THIRD_PARTY_UNISOC_UWP566X_COMPONENTS_UWP566X_BSP_LED_UWP_LED_H_
#define LIB_THIRD_PARTY_UNISOC_UWP566X_COMPONENTS_UWP566X_BSP_LED_UWP_LED_H_

#include "gpio_api.h"

#define LED1 PIN1
#define LED2 PIN2
#define LED3 PIN3

#ifdef __cplusplus
extern "C"{
#endif

void vled_Init(gpio_t *pxobj, PinName eled);
void vled_On(gpio_t *pxobj);
void vled_Off(gpio_t *pxobj);
void vled_turn_all(gpio_t *led1, gpio_t *led2, gpio_t *led3);

#ifdef __cplusplus
}
#endif

#endif /* LIB_THIRD_PARTY_UNISOC_UWP566X_COMPONENTS_UWP566X_BSP_LED_UWP_LED_H_ */
