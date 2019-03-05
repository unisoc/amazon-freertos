/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "uwp_intc.h"
#include "hal_intc.h"
#include "uwp_log.h"
#include "UWP_5661.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

/* convenience defines */
struct __intc_cb {
    uwp_intc_callback_t cb;
    void *data;
};

struct intc_uwp_data {
    struct __intc_cb irq[INTC_MAX_CHANNEL];
    struct __intc_cb fiq[INTC_MAX_CHANNEL];
};

static struct intc_uwp_data intc_uwp_dev_data;
static struct intc_uwp_data aon_intc_uwp_dev_data;

void uwp_intc_set_irq_callback(int channel,
        uwp_intc_callback_t cb, void *arg)
{
    if (intc_uwp_dev_data.irq[channel].cb != NULL) {
        LOG_WRN("IRQ %d callback has been registered.\n", channel);
        return;
    }

    intc_uwp_dev_data.irq[channel].cb = cb;
    intc_uwp_dev_data.irq[channel].data = arg;
}

void uwp_intc_unset_irq_callback(int channel)
{
    intc_uwp_dev_data.irq[channel].cb = NULL;
    intc_uwp_dev_data.irq[channel].data = NULL;
}

void uwp_intc_set_fiq_callback(int channel,
        uwp_intc_callback_t cb, void *arg)
{
    if (intc_uwp_dev_data.fiq[channel].cb != NULL) {
        LOG_WRN("FIQ %d callback has been registered.\n", channel);
        return;
    }

    intc_uwp_dev_data.fiq[channel].cb = cb;
    intc_uwp_dev_data.fiq[channel].data = arg;
}

void uwp_intc_unset_fiq_callback(int channel)
{
    intc_uwp_dev_data.fiq[channel].cb = NULL;
    intc_uwp_dev_data.fiq[channel].data = NULL;
}

void uwp_aon_intc_set_irq_callback(int channel,
        uwp_intc_callback_t cb, void *arg)
{

    if (aon_intc_uwp_dev_data.irq[channel].cb != NULL) {
        LOG_WRN("IRQ %d callback has been registered.\n", channel);
        return;
    }

    aon_intc_uwp_dev_data.irq[channel].cb = cb;
    aon_intc_uwp_dev_data.irq[channel].data = arg;
}

void uwp_aon_intc_unset_irq_callback(int channel)
{

    aon_intc_uwp_dev_data.irq[channel].cb = NULL;
    aon_intc_uwp_dev_data.irq[channel].data = NULL;
}

void uwp_ictl_irq_enable(uwp_irq_type_t irq_type, unsigned int irq){
    volatile struct uwp_intc *intc = NULL;
    if(irq_type == AON_IRQ_TYPE)
        intc = (volatile struct uwp_intc *)BASE_AON_INTC;
    else if(irq_type == IRQ_IRQ_TYPE)
        intc = (volatile struct uwp_intc *)BASE_INTC;
    else if(irq_type == FIQ_IRQ_TYPE)
        intc = (volatile struct uwp_intc *)(BASE_INTC + 0x20);
    uwp_intc_enable(intc, irq);
}

void uwp_ictl_irq_disable(uwp_irq_type_t irq_type, unsigned int irq){
    volatile struct uwp_intc *intc = NULL;
    if(irq_type == AON_IRQ_TYPE)
        intc = (volatile struct uwp_intc *)BASE_AON_INTC;
    else if(irq_type == IRQ_IRQ_TYPE)
        intc = (volatile struct uwp_intc *)BASE_INTC;
    else if(irq_type == FIQ_IRQ_TYPE)
        intc = (volatile struct uwp_intc *)(BASE_INTC + 0x20);
    uwp_intc_disable(intc, irq);
}

static void intc_uwp_irq(void)
{
    mbed_error_printf("%s\r\n",__func__);
    volatile struct uwp_intc * intc = (volatile struct uwp_intc *)BASE_INTC;
    u32_t status = uwp_intc_status(intc);
    int ch;
    for (ch = 0; ch < INTC_MAX_CHANNEL; ch++) {
        if ((status & (0x1UL<<ch)) && intc_uwp_dev_data.irq[ch].cb) {
            uwp_intc_disable(intc, ch);
            intc_uwp_dev_data.irq[ch].cb(ch, intc_uwp_dev_data.irq[ch].data);
            uwp_intc_enable(intc, ch);
        }
    }
}

static void intc_uwp_fiq(void)
{
    mbed_error_printf("%s\r\n",__func__);
    volatile struct uwp_intc * intc = (volatile struct uwp_intc *)(BASE_INTC + 0x20);
    u32_t status = uwp_intc_status(intc);
    int ch;
    for (ch = 0; ch < INTC_MAX_CHANNEL; ch++) {
        if ((status & (0x1UL<<ch)) && intc_uwp_dev_data.fiq[ch].cb) {
            uwp_intc_disable(intc, ch);
            intc_uwp_dev_data.fiq[ch].cb(ch, intc_uwp_dev_data.fiq[ch].data);
            uwp_intc_enable(intc, ch);
        }
    }
}

static void aon_intc_uwp_irq(void)
{
    volatile struct uwp_intc * intc = (volatile struct uwp_intc *)BASE_AON_INTC;
    u32_t status = uwp_intc_status(intc);
    int ch;
    for (ch = 0; ch < INTC_MAX_CHANNEL; ch++) {
        if ((status & (0x1UL<<ch)) && aon_intc_uwp_dev_data.irq[ch].cb) {
            uwp_intc_disable(intc, ch);
            LOG_DBG("AON ch:%d\r\n",ch);
            aon_intc_uwp_dev_data.irq[ch].cb(ch, aon_intc_uwp_dev_data.irq[ch].data);
            uwp_intc_enable(intc, ch);
        }
    }
}

void intc_uwp_init(void)
{
    NVIC_DisableIRQ(INT_IRQ_IRQn);
    NVIC_DisableIRQ(INT_FIQ_IRQn);

    uwp_sys_enable(BIT(APB_EB_INTC));
    uwp_sys_reset(BIT(APB_EB_INTC));

    NVIC_SetVector(INT_IRQ_IRQn,(uint32_t)intc_uwp_irq);
    NVIC_SetPriority(INT_IRQ_IRQn,0x1FUL);
    NVIC_EnableIRQ(INT_IRQ_IRQn);

    NVIC_SetVector(INT_FIQ_IRQn,(uint32_t)intc_uwp_fiq);
    NVIC_SetPriority(INT_FIQ_IRQn,0x1FUL);
    NVIC_EnableIRQ(INT_FIQ_IRQn);
}

void aon_intc_uwp_init(void)
{
    NVIC_DisableIRQ(AON_IRQn);

    uwp_aon_enable(BIT(AON_EB_INTC));
    uwp_aon_reset(BIT(AON_RST_INTC));
    
    NVIC_SetVector(AON_IRQn,(uint32_t)aon_intc_uwp_irq);
    NVIC_SetPriority(AON_IRQn,0x1FUL);
    NVIC_EnableIRQ(AON_IRQn);
}

