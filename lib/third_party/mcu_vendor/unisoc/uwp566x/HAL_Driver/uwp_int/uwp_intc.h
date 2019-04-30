/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_UWP_INTC_H
#define MBED_UWP_INTC_H

typedef enum{
    FIQ_IRQ_TYPE,
    IRQ_IRQ_TYPE,
    AON_IRQ_TYPE
}uwp_irq_type_t;

typedef void (*uwp_intc_callback_t) (int channel, void *user);

extern void uwp_intc_set_irq_callback(int channel,
        uwp_intc_callback_t cb, void *arg);
extern void uwp_intc_unset_irq_callback(int channel);
extern void uwp_intc_set_fiq_callback(int channel,
        uwp_intc_callback_t cb, void *arg);
extern void uwp_intc_unset_fiq_callback(int channel);
extern void uwp_aon_intc_set_irq_callback(int channel,
        uwp_intc_callback_t cb, void *arg);
extern void uwp_aon_intc_unset_irq_callback(int channel);

void aon_intc_uwp_init(void);
void intc_uwp_init(void);
void uwp_ictl_irq_enable(uwp_irq_type_t irq_type, unsigned int irq);

#endif /* _INTC_UWP_H_ */
