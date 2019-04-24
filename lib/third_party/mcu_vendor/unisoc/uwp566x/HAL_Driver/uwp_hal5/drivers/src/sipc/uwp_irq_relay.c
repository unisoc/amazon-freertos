/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "UWP_5661.h"
#include "sipc.h"
#include "hal_type.h"
#include "hal_intc.h"

//#define WIFI_LOG_INF
//#define WIFI_LOG_DBG
#include "uwp_log.h"

#if defined(CONFIG_BT_CTLR_WORKER_PRIO)
#define RADIO_TICKER_USER_ID_WORKER_PRIO CONFIG_BT_CTLR_WORKER_PRIO
#else
#define RADIO_TICKER_USER_ID_WORKER_PRIO 0
#endif

#if defined(CONFIG_BT_CTLR_JOB_PRIO)
#define RADIO_TICKER_USER_ID_JOB_PRIO CONFIG_BT_CTLR_JOB_PRIO
#else
#define RADIO_TICKER_USER_ID_JOB_PRIO 0
#endif

#define CTL_INTC_BASE            0x40000000

#define CEVA_IP_INT_CLEAR_ADDR (0x40246000+0x28)
#define CEVA_IP_int_clear(clear_bit) { int temp = \
	*((volatile u32_t *)CEVA_IP_INT_CLEAR_ADDR); \
	temp |= clear_bit; *((volatile u32_t *)CEVA_IP_INT_CLEAR_ADDR) = temp; }

#define CEVA_IP_int_MASK(bit) {int temp = \
	*((volatile u32_t *)CEVA_IP_INT_CLEAR_ADDR); \
	temp |= bit; *((volatile u32_t *)CEVA_IP_INT_CLEAR_ADDR) = temp; }
#define CEVA_IP_int_UNMASK(bit) {int temp = \
	*((volatile u32_t *)CEVA_IP_INT_CLEAR_ADDR); \
	temp &= ~bit; *((volatile u32_t *)CEVA_IP_INT_CLEAR_ADDR) = temp; }


#define HW_DEC_INT_CLEAR (0x40240000+0x304)
#define HW_DEC_INT1_CLEAR (0x40240000+0x308)

#define HW_DEC_int_clear(clear_bit) \
{unsigned int temp = *((volatile u32_t *)HW_DEC_INT_CLEAR); \
	temp |= clear_bit; \
	*((volatile u32_t *)HW_DEC_INT_CLEAR) = temp; \
	temp &= ~clear_bit; \
	*((volatile u32_t *)HW_DEC_INT_CLEAR) = temp; }

#define HW_DEC_int_clear_sts \
{	unsigned int temp1; \
	unsigned int temp = *((volatile u32_t *)HW_DEC_INT_CLEAR); \
	temp1 = (temp >> 16)&0xff; \
	*((volatile u32_t *)HW_DEC_INT_CLEAR) = temp | temp1; \
	*((volatile u32_t *)HW_DEC_INT_CLEAR) = temp&(~0xff); }

#define HW_DEC_int1_clear_sts \
{	unsigned int temp1; \
	unsigned int temp = *((volatile u32_t *)HW_DEC_INT1_CLEAR); \
	temp1 = (temp >> 16)&0xff; \
	*((volatile u32_t *)HW_DEC_INT1_CLEAR) = temp | temp1; \
	*((volatile u32_t *)HW_DEC_INT1_CLEAR) = temp&(~0xff); }

#define  PKD_INTR_MASK          (1<<4)
#define  AUX_TMR_INTR_MASK      (1<<5)
#define  PKA_INTR_MASK          (1<<6)
#define  PKD_RX_HDR_INTR_MASK   (1<<7)
#define  PKD_NO_PKD_INTR_MASK   (1<<13)
#define  SYNC_DET_INTR_MASK     (1<<14)


#define TIM_INTRO_CLR     (1<<16)
#define TIM_INTR1_CLR     (1<<17)
#define TIM_INTR2_CLR     (1<<18)
#define TIM_INTR3_CLR     (1<<19)
#define AUX_TMR_INTR      (1<<21)
#define PKA_INTR          (1<<22)
#define SYNC_DET_INTR     (1<<30)
#define PKD_RX_HDR        (1<<23)
#define PKD_INTR          (1<<20)
#define PAGE_TIMEOUT_INTR (1<<29)
#define ATOR_INTR0        (1<<0)
#define ATOR_INTR1        (1<<1)
#define ATOR_INTR2        (1<<2)

#define SHARE_MEM_WATCH   (0x1EEF00)
u16_t clear_bt_int(int irq_num)
{
	u16_t tem=0;
	switch (irq_num) {
	case NVIC_BT_MASKED_TIM_INTR0:
		*((u16_t *)(SHARE_MEM_WATCH+0x0c)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x0c));
		CEVA_IP_int_clear(TIM_INTRO_CLR); break;
	case NVIC_BT_MASKED_TIM_INTR1:
		*((u16_t *)(SHARE_MEM_WATCH+0x10)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x10));
		CEVA_IP_int_clear(TIM_INTR1_CLR); break;
	case NVIC_BT_MASKED_TIM_INTR2:
		*((u16_t *)(SHARE_MEM_WATCH+0x14)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x14));
		CEVA_IP_int_clear(TIM_INTR2_CLR); break;
	case NVIC_BT_MASKED_TIM_INTR3:
		*((u16_t *)(SHARE_MEM_WATCH+0x18)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x18));
		CEVA_IP_int_clear(TIM_INTR3_CLR); break;
	case NVIC_BT_MASKED_AUX_TMR_INTR:
		*((u16_t *)(SHARE_MEM_WATCH+0x24)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x24));
		CEVA_IP_int_clear(AUX_TMR_INTR); break;
	case NVIC_BT_MASKED_PKA_INTR:
		*((u16_t *)(SHARE_MEM_WATCH+0x20)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x20));
		CEVA_IP_int_clear(PKA_INTR); break;
	case NVIC_BT_MASKED_SYNC_DET_INTR:
		*((u16_t *)(SHARE_MEM_WATCH+0x04)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x04));
		CEVA_IP_int_clear(SYNC_DET_INTR); break;
	case NVIC_BT_MASKED_PKD_RX_HDR:
		*((u16_t *)(SHARE_MEM_WATCH+0x08)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x08));
		CEVA_IP_int_clear(PKD_RX_HDR); break;
	case NVIC_BT_MASKED_PKD_INTR:
		*((u16_t *)(SHARE_MEM_WATCH+0x1c)) += 1;
		tem =  *((u16_t *)(SHARE_MEM_WATCH+0x1c));
		CEVA_IP_int_clear(PKD_INTR); break;
	case NVIC_BT_MASKED_PAGE_TIMEOUT_INTR:
		*((u16_t *)(SHARE_MEM_WATCH+0)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0));
		CEVA_IP_int_MASK(PKD_NO_PKD_INTR_MASK); break;
	case NVIC_BT_ACCELERATOR_INTR0:
		*((u16_t *)(SHARE_MEM_WATCH+0x28)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x28));
		HW_DEC_int_clear(ATOR_INTR0); break;
	case NVIC_BT_ACCELERATOR_INTR1:
		*((u16_t *)(SHARE_MEM_WATCH+0x2c)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x2c));
		HW_DEC_int_clear(ATOR_INTR1); break;
	case NVIC_BT_ACCELERATOR_INTR2:
		*((u16_t *)(SHARE_MEM_WATCH+0x30)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x30));
		HW_DEC_int_clear(ATOR_INTR2); break;
	case NVIC_BT_ACCELERATOR_INTR3:
		*((u16_t *)(SHARE_MEM_WATCH+0x34)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x34));
		HW_DEC_int_clear_sts; break;
	case NVIC_BT_ACCELERATOR_INTR4:
		*((u16_t *)(SHARE_MEM_WATCH+0x38)) += 1;
		tem = *((u16_t *)(SHARE_MEM_WATCH+0x38));
		HW_DEC_int1_clear_sts; break;

    default:
		LOG_INF("bt clear irq error %d\n", irq_num); break;
	}
	if((tem == SMSG_OPEN_MAGIC) || (tem == SMSG_CLOSE_MAGIC)) {
		return 0;
	} else {
		return tem;
	}
}

void sprd_bt_irq_enable(void)
{
	NVIC_EnableIRQ(BT_MASKED_PAGE_TIMEOUT_INTR_IRQn);
	NVIC_EnableIRQ(BT_MASKED_SYNC_DET_INTR_IRQn);
	NVIC_EnableIRQ(BT_MASKED_PKD_RX_HDR_IRQn);
	NVIC_EnableIRQ(BT_MASKED_TIM_INTR0_IRQn);
	NVIC_EnableIRQ(BT_MASKED_TIM_INTR1_IRQn);
	NVIC_EnableIRQ(BT_MASKED_TIM_INTR2_IRQn);
	NVIC_EnableIRQ(BT_MASKED_TIM_INTR3_IRQn);
	NVIC_EnableIRQ(BT_MASKED_PKD_INTR_IRQn);
	NVIC_EnableIRQ(BT_MASKED_PKA_INTR_IRQn);
	NVIC_EnableIRQ(BT_MASKED_AUX_TMR_INTR_IRQn);
	NVIC_EnableIRQ(BT_ACCELERATOR_INTR0_IRQn);
	NVIC_EnableIRQ(BT_ACCELERATOR_INTR1_IRQn);
	NVIC_EnableIRQ(BT_ACCELERATOR_INTR2_IRQn);
	NVIC_EnableIRQ(BT_ACCELERATOR_INTR3_IRQn);
	NVIC_EnableIRQ(BT_ACCELERATOR_INTR4_IRQn);
}

void sprd_bt_irq_disable(void)
{
	NVIC_DisableIRQ(BT_MASKED_PAGE_TIMEOUT_INTR_IRQn);
	NVIC_DisableIRQ(BT_MASKED_SYNC_DET_INTR_IRQn);
	NVIC_DisableIRQ(BT_MASKED_PKD_RX_HDR_IRQn);
	NVIC_DisableIRQ(BT_MASKED_TIM_INTR0_IRQn);
	NVIC_DisableIRQ(BT_MASKED_TIM_INTR1_IRQn);
	NVIC_DisableIRQ(BT_MASKED_TIM_INTR2_IRQn);
	NVIC_DisableIRQ(BT_MASKED_TIM_INTR3_IRQn);
	NVIC_DisableIRQ(BT_MASKED_PKD_INTR_IRQn);
	NVIC_DisableIRQ(BT_MASKED_PKA_INTR_IRQn);
	NVIC_DisableIRQ(BT_MASKED_AUX_TMR_INTR_IRQn);
	NVIC_DisableIRQ(BT_ACCELERATOR_INTR0_IRQn);
	NVIC_DisableIRQ(BT_ACCELERATOR_INTR1_IRQn);
	NVIC_DisableIRQ(BT_ACCELERATOR_INTR2_IRQn);
	NVIC_DisableIRQ(BT_ACCELERATOR_INTR3_IRQn);
	NVIC_DisableIRQ(BT_ACCELERATOR_INTR4_IRQn);
}

static void wifi_mac_irq_handler(void)
{

    struct smsg msg;
    s32_t irq = (s32_t)MAC_IRQn;
    NVIC_DisableIRQ(MAC_IRQn);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    //printk("mac irq\r\n");
    NVIC_EnableIRQ(MAC_IRQn);
}

static void wifi_dpd_irq_handler(void)
{
    struct smsg msg;
    s32_t irq = (s32_t)DPD_IRQn;
    NVIC_DisableIRQ(DPD_IRQn);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    //printk("dpd irq\r\n");
    NVIC_EnableIRQ(DPD_IRQn);
}

static void wifi_comtmr_irq_handler(void)
{
    struct smsg msg;
    s32_t irq = (s32_t)COMTMR_IRQn;
    NVIC_DisableIRQ(COMTMR_IRQn);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    //printk("comtmr irq\r\n");
    NVIC_EnableIRQ(COMTMR_IRQn);
}

void wifi_irq_init(void)
{
    NVIC_DisableIRQ(MAC_IRQn);
    NVIC_SetVector(MAC_IRQn,(uint32_t)wifi_mac_irq_handler);
    NVIC_SetPriority(MAC_IRQn,0x5UL);

    NVIC_DisableIRQ(DPD_IRQn);
    NVIC_SetVector(DPD_IRQn,(uint32_t)wifi_dpd_irq_handler);
    NVIC_SetPriority(DPD_IRQn,0x5UL);

    NVIC_DisableIRQ(COMTMR_IRQn);
    NVIC_SetVector(COMTMR_IRQn,(uint32_t)wifi_comtmr_irq_handler);
    NVIC_SetPriority(COMTMR_IRQn,0x5UL);
}

void sprd_wifi_irq_enable_num(u32_t num)
{
    LOG_INF("wifi irq enable %d\n", num);

    switch (num) {
    case MAC_IRQn:
        NVIC_EnableIRQ(MAC_IRQn);
        break;
    case COMTMR_IRQn:
        NVIC_EnableIRQ(COMTMR_IRQn);
        break;
    case DPD_IRQn:
        NVIC_EnableIRQ(DPD_IRQn);
        break;
    default:
        LOG_INF("wifi irq enable error num %d\n", num);
        break;
    }
}

void sprd_wifi_irq_disable_num(u32_t num)
{
	LOG_INF("wifi irq disable %d\n", num);
	switch (num) {
	case MAC_IRQn:
		NVIC_DisableIRQ(MAC_IRQn);
	break;
	case COMTMR_IRQn:
		NVIC_DisableIRQ(COMTMR_IRQn);
	break;
	case DPD_IRQn:
		NVIC_DisableIRQ(DPD_IRQn);
	break;
	default:
		LOG_INF("wifi irq disable error num %d\n", num);
	break;
	}

}

static void bt_masked_page_timeout_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_PAGE_TIMEOUT_INTR_IRQn;

    NVIC_DisableIRQ(BT_MASKED_PAGE_TIMEOUT_INTR_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_PAGE_TIMEOUT_INTR_IRQn);
}

static void bt_masked_sync_det_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_SYNC_DET_INTR_IRQn;

    NVIC_DisableIRQ(BT_MASKED_SYNC_DET_INTR_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_SYNC_DET_INTR_IRQn);
}

static void bt_masked_pkd_rx_hdr_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_PKD_RX_HDR_IRQn;

    NVIC_DisableIRQ(BT_MASKED_PKD_RX_HDR_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_PKD_RX_HDR_IRQn);
}

static void bt_masked_tim_intr0_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_TIM_INTR0_IRQn;

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR0_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_TIM_INTR0_IRQn);
}

static void bt_masked_tim_intr1_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_TIM_INTR1_IRQn;

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR1_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_TIM_INTR1_IRQn);
}

static void bt_masked_tim_intr2_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_TIM_INTR2_IRQn;

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR2_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_TIM_INTR2_IRQn);
}

static void bt_masked_tim_intr3_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_TIM_INTR3_IRQn;

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR3_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_TIM_INTR3_IRQn);
}

static void bt_masked_pkd_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_PKD_INTR_IRQn;

    NVIC_DisableIRQ(BT_MASKED_PKD_INTR_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_PKD_INTR_IRQn);
}

static void bt_masked_pka_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_PKA_INTR_IRQn;

    NVIC_DisableIRQ(BT_MASKED_PKA_INTR_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_PKA_INTR_IRQn);
}

static void bt_masked_aux_tmr_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_MASKED_AUX_TMR_INTR_IRQn;

    NVIC_DisableIRQ(BT_MASKED_AUX_TMR_INTR_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_MASKED_AUX_TMR_INTR_IRQn);
}

static void bt_masked_accelerator_intr0_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_ACCELERATOR_INTR0_IRQn;

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR0_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_ACCELERATOR_INTR0_IRQn);
}

static void bt_masked_accelerator_intr1_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_ACCELERATOR_INTR1_IRQn;

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR1_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_ACCELERATOR_INTR1_IRQn);
}

static void bt_masked_accelerator_intr2_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_ACCELERATOR_INTR2_IRQn;

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR2_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_ACCELERATOR_INTR2_IRQn);
}

static void bt_masked_accelerator_intr3_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_ACCELERATOR_INTR3_IRQn;

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR3_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_ACCELERATOR_INTR3_IRQn);
}

static void bt_masked_accelerator_intr4_handler(void){
    struct smsg msg;
    s32_t irq = (s32_t)BT_ACCELERATOR_INTR4_IRQn;

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR4_IRQn);
    clear_bt_int(irq);
    LOG_DBG("%s\r\n",__func__);
    smsg_set(&msg, SMSG_CH_IRQ_DIS, SMSG_TYPE_EVENT, 0, irq);
    smsg_send_irq(SIPC_ID_AP, &msg);
    NVIC_EnableIRQ(BT_ACCELERATOR_INTR4_IRQn);
}

void sprd_bt_irq_init(void)
{
    NVIC_DisableIRQ(BT_MASKED_PAGE_TIMEOUT_INTR_IRQn);
    NVIC_SetVector(BT_MASKED_PAGE_TIMEOUT_INTR_IRQn,(uint32_t)bt_masked_page_timeout_handler);
    NVIC_SetPriority(BT_MASKED_PAGE_TIMEOUT_INTR_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_SYNC_DET_INTR_IRQn);
    NVIC_SetVector(BT_MASKED_SYNC_DET_INTR_IRQn,(uint32_t)bt_masked_sync_det_handler);
    NVIC_SetPriority(BT_MASKED_SYNC_DET_INTR_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_PKD_RX_HDR_IRQn);
    NVIC_SetVector(BT_MASKED_PKD_RX_HDR_IRQn,(uint32_t)bt_masked_pkd_rx_hdr_handler);
    NVIC_SetPriority(BT_MASKED_PKD_RX_HDR_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR0_IRQn);
    NVIC_SetVector(BT_MASKED_TIM_INTR0_IRQn,(uint32_t)bt_masked_tim_intr0_handler);
    NVIC_SetPriority(BT_MASKED_TIM_INTR0_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR1_IRQn);
    NVIC_SetVector(BT_MASKED_TIM_INTR1_IRQn,(uint32_t)bt_masked_tim_intr1_handler);
    NVIC_SetPriority(BT_MASKED_TIM_INTR1_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR2_IRQn);
    NVIC_SetVector(BT_MASKED_TIM_INTR2_IRQn,(uint32_t)bt_masked_tim_intr2_handler);
    NVIC_SetPriority(BT_MASKED_TIM_INTR2_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_TIM_INTR3_IRQn);
    NVIC_SetVector(BT_MASKED_TIM_INTR3_IRQn,(uint32_t)bt_masked_tim_intr3_handler);
    NVIC_SetPriority(BT_MASKED_TIM_INTR3_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_PKD_INTR_IRQn);
    NVIC_SetVector(BT_MASKED_PKD_INTR_IRQn,(uint32_t)bt_masked_pkd_handler);
    NVIC_SetPriority(BT_MASKED_PKD_INTR_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_PKA_INTR_IRQn);
    NVIC_SetVector(BT_MASKED_PKA_INTR_IRQn,(uint32_t)bt_masked_pka_handler);
    NVIC_SetPriority(BT_MASKED_PKA_INTR_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_MASKED_AUX_TMR_INTR_IRQn);
    NVIC_SetVector(BT_MASKED_AUX_TMR_INTR_IRQn,(uint32_t)bt_masked_aux_tmr_handler);
    NVIC_SetPriority(BT_MASKED_AUX_TMR_INTR_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR0_IRQn);
    NVIC_SetVector(BT_ACCELERATOR_INTR0_IRQn,(uint32_t)bt_masked_accelerator_intr0_handler);
    NVIC_SetPriority(BT_ACCELERATOR_INTR0_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR1_IRQn);
    NVIC_SetVector(BT_ACCELERATOR_INTR1_IRQn,(uint32_t)bt_masked_accelerator_intr1_handler);
    NVIC_SetPriority(BT_ACCELERATOR_INTR1_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR2_IRQn);
    NVIC_SetVector(BT_ACCELERATOR_INTR2_IRQn,(uint32_t)bt_masked_accelerator_intr2_handler);
    NVIC_SetPriority(BT_ACCELERATOR_INTR2_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR3_IRQn);
    NVIC_SetVector(BT_ACCELERATOR_INTR3_IRQn,(uint32_t)bt_masked_accelerator_intr3_handler);
    NVIC_SetPriority(BT_ACCELERATOR_INTR3_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

    NVIC_DisableIRQ(BT_ACCELERATOR_INTR4_IRQn);
    NVIC_SetVector(BT_ACCELERATOR_INTR4_IRQn,(uint32_t)bt_masked_accelerator_intr4_handler);
    NVIC_SetPriority(BT_ACCELERATOR_INTR4_IRQn,RADIO_TICKER_USER_ID_WORKER_PRIO);

	sprd_bt_irq_enable();
}

void wifi_irq_enable()
{
	NVIC_EnableIRQ(GNSS2BTWIFI_IPI_IRQn);
}

