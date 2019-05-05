/*
 * uwp_led.c
 *
 *  Created on: 2019Äê2ÔÂ27ÈÕ
 *      Author: xiangkai.gao
 */

#include "uwp_led.h"

void vled_Init(gpio_t *pxobj, PinName eled){
    gpio_init(pxobj, eled);
    gpio_dir(pxobj, PIN_OUTPUT);
}

void vled_On(gpio_t *pxobj){
    gpio_write(pxobj, 0);
}

void vled_Off(gpio_t *pxobj){
    gpio_write(pxobj, 1);
}

void vled_turn_all(gpio_t *led1, gpio_t *led2, gpio_t *led3){
    static int prvscnt = 0;
    if( (prvscnt % 3) == 0 ){
        gpio_write(led1, 0);
        gpio_write(led2, 1);
        gpio_write(led3, 1);
    }
    else if( (prvscnt % 3) == 1 ){
        gpio_write(led1, 1);
        gpio_write(led2, 0);
        gpio_write(led3, 1);
    }
    else{
        gpio_write(led1, 1);
        gpio_write(led2, 1);
        gpio_write(led3, 0);
    }
    prvscnt ++;
}
