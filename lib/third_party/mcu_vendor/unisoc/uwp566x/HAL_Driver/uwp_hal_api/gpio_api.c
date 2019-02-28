/*
 * Copyright (c) 2016, Spreadtrum Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio_api.h"
#include "uwp_hal.h"

#define PORT0_INITED_FLAG 0x01
#define PORT1_INITED_FLAG 0x02
#define PORT2_INITED_FLAG 0x04
static unsigned char Port_Inited_Flag = 0x00;

static void gpio_uwp_p0_init(u32_t base)
{
    if(!(Port_Inited_Flag & PORT0_INITED_FLAG)){
        uwp_aon_enable(BIT(AON_EB_GPIO0));
        uwp_aon_reset(BIT(AON_RST_GPIO0));
    /* enable all gpio read/write by default */
        uwp_gpio_enable(base, 0xFFFF);

        uwp_gpio_int_disable(base, 0xFFFF);
        uwp_gpio_disable(base, 0xFFFF);

        Port_Inited_Flag |= PORT0_INITED_FLAG;
    }
}

static void gpio_uwp_p1_init(u32_t base)
{
    if(!(Port_Inited_Flag & PORT1_INITED_FLAG)){
        uwp_aon_enable(BIT(AON_EB_GPIO1));
        uwp_aon_reset(BIT(AON_RST_GPIO1));
    /* enable all gpio read/write by default */
        uwp_gpio_enable(base, 0xFFFF);
        uwp_gpio_int_disable(base, 0xFFFF);
        uwp_gpio_disable(base, 0xFFFF);

        Port_Inited_Flag |= PORT1_INITED_FLAG;
    }
}

static void gpio_uwp_p2_init(u32_t base)
{
    if(!(Port_Inited_Flag & PORT2_INITED_FLAG)){
        uwp_aon_enable(BIT(AON_EB_GPIO2));
        uwp_aon_reset(BIT(AON_RST_GPIO2));
    /* enable all gpio read/write by default */
        uwp_gpio_enable(base, 0xFFFF);
        uwp_gpio_int_disable(base, 0xFFFF);
        uwp_gpio_disable(base, 0xFFFF);

        Port_Inited_Flag |= PORT2_INITED_FLAG;
    }
}

void gpio_dir(gpio_t *obj, PinDirection direction){
    uwp_gpio_set_dir(obj->port_base, BIT(obj->pin), direction);
    uwp_gpio_enable(obj->port_base, BIT(obj->pin));
}

void gpio_write(gpio_t *obj, int value){
    uwp_gpio_write(obj->port_base, BIT(obj->pin), value);
}

int gpio_read(gpio_t *obj){
    return ((uwp_gpio_read(obj->port_base, BIT(obj->pin))) >> (obj->pin));
}

// TODO: relationship between port and pin 
void gpio_init(gpio_t *obj, PinName pin){
    obj->pin = pin;
    switch(obj->port_base){
        case BASE_AON_GPIOP0: gpio_uwp_p0_init(BASE_AON_GPIOP0); break;
        case BASE_AON_GPIOP1: gpio_uwp_p1_init(BASE_AON_GPIOP1); break;
        case BASE_AON_GPIOP2: gpio_uwp_p2_init(BASE_AON_GPIOP2); break;
        default:         
                              obj->port_base = BASE_AON_GPIOP0;
                              gpio_uwp_p0_init(BASE_AON_GPIOP0); break;
    }
}

void gpio_mode(gpio_t *obj, PinMode mode)
{
    //pin_mode(obj->pin, mode);
}

#ifdef __cplusplus
extern "C" {
#endif


