/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 

#include <string.h>

#include "uwp_wifi_txrx.h"
#include "uwp_wifi_ipc.h"
#include "uwp_wifi_cmdevt.h"
#include "sipc.h"
#include "uwp_sys_wrapper.h"
#include "mbed_retarget.h"
#include "uwp_buf_mgmt.h"

//#define WIFI_LOG_DBG
//#define WIFI_DUMP
#include "uwp_log.h"

#define RX_DATA_SIZE (2000)
#define RX_CMDEVT_SIZE (512)
#define RX_STACK_SIZE (1024)

#define wifi_buf_slist_init(list) sys_slist_init(list)
#define wifi_buf_slist_append(list, buf) net_buf_slist_put(list, buf)
#define wifi_buf_slist_get(list) net_buf_slist_get(list)
#define wifi_buf_slist_remove(list, node) sys_slist_find_and_remove(list, node)

#define wifi_snode_t sys_snode_t
#define wifi_slist_t sys_slist_t

static void *event_sem = NULL;
static void *data_sem = NULL;
static void *rx_buf_mutex = NULL;
void * rx_cmdevt_thread_data;
void * rx_data_thread_data;

static u8_t rx_data_buf[RX_DATA_SIZE];
static u8_t rx_cmdevt_buf[RX_CMDEVT_SIZE];
extern void *packet_rx_queue;


int wifi_rx_complete_handle(struct wifi_priv *priv, void *data, int len)
{
    struct rxc *rx_complete_buf = (struct rxc *)data;
    struct rxc_ddr_addr_trans_t *rxc_addr = &rx_complete_buf->rxc_addr;
    struct rx_msdu_desc *rx_msdu = NULL;
    u32_t payload = 0;
    u8_t *debug_buf;

    int ctx_id = 0;
    int i = 0;
    u32_t data_len;
    int ret;

    for (i = 0; i < rxc_addr->num; i++) {
        memcpy(&payload, rxc_addr->addr_addr[i], 4);

        WIFI_ASSERT(payload > SPRD_CP_DRAM_BEGIN
            && payload < SPRD_CP_DRAM_END,
            "Invalid buffer address: %p", (void *)payload);

        SPRD_CP_TO_AP_ADDR(payload);

        WIFI_ASSERT(payload > SPRD_AP_DRAM_BEGIN
            && payload < SPRD_AP_DRAM_END,
            "Invalid pkt_buf address: %p", (void *)payload);

        rx_msdu = (struct rx_msdu_desc *)(payload + sizeof(struct rx_mh_desc));
        ctx_id = rx_msdu->ctx_id;
        data_len = rx_msdu->msdu_len + rx_msdu->msdu_offset;
        WIFI_ASSERT(data_len > 0 && data_len < 1800,
            "Invalid data len: %d", data_len);

extern BaseType_t wlanif_input(void *netif, void *buffer, uint16_t len);
		wlanif_input(NULL, (void *)((uint32_t)(payload + rx_msdu->msdu_offset)), rx_msdu->msdu_len);
        uwp_pkt_buf_free((void *)(payload));
    }


    /* Allocate new empty buffer to cp. */
    LOG_DBG("notify cp:%d\r\n",i);
    if(wifi_rx_free_num > i) {
        LOG_DBG("wifi_rx_free_num(%d) > i(%d)!!!\r\n",wifi_rx_free_num,i);
        i = wifi_rx_free_num;
    }
    ret = wifi_tx_empty_buf(i);

    return ret;
}

int wifi_tx_complete_handle(void *data, int len)
{
    struct txc *txc_complete_buf = (struct txc *)data;
    struct txc_addr_buff *txc_addr = &txc_complete_buf->txc_addr;
    u16_t payload_num;
    u32_t payload_addr;
    u32_t tx_pkt;
    u8_t* pbuf;
    int i;

    payload_num = txc_addr->number;
    for (i = 0; i < payload_num; i++) {
        /* We use only 4 byte addr. */
        memcpy(&payload_addr,
            txc_addr->data + (i * SPRDWL_PHYS_LEN), 4);

        payload_addr -= sizeof(struct tx_msdu_dscr);
        WIFI_ASSERT(payload_addr > SPRD_CP_DRAM_BEGIN
            && payload_addr < SPRD_CP_DRAM_END,
            "Invalid buffer address: %p", (void *)payload_addr);

        SPRD_CP_TO_AP_ADDR(payload_addr);
        tx_pkt = uwp_get_addr_from_payload(payload_addr);
        WIFI_ASSERT(payload_addr > SPRD_AP_DRAM_BEGIN
            && payload_addr < SPRD_AP_DRAM_END,
            "Invalid pkt address: %p", (void *)payload_addr);

        LOG_DBG("free TXBUFF:%p\r\n",payload_addr);
        //UWP_MEM_FREE(payload_addr);//free(payload_addr);
        pbuf = (u8_t *)tx_pkt;
        
        pbuf += sizeof(struct tx_msdu_dscr);
        extern void wlanif_free_network_buffer(uint32_t addr);
        wlanif_free_network_buffer((u32_t)pbuf);
    }

    return 0;
}

int wifi_data_process(struct wifi_priv *priv, char *data, int len)
{
    struct sprdwl_common_hdr *common_hdr = (struct sprdwl_common_hdr *)data;

    switch (common_hdr->type) {
    case SPRDWL_TYPE_DATA_SPECIAL:
        if (common_hdr->direction_ind) { /* Rx data. */
            wifi_rx_complete_handle(priv, data, len);
        } else { /* Tx complete. */
            wifi_tx_complete_handle(data, len);
        }
        break;
    case SPRDWL_TYPE_DATA:
    case SPRDWL_TYPE_DATA_HIGH_SPEED:
        break;
    default:
        LOG_ERR("Unknown type :%d\n", common_hdr->type);
    }
    return 0;
}
/*success,ret0;fail,ret<0*/
static int wifi_tx_empty_buf_(int num)
{
	int i;
	void *pkt_buf = NULL;
	struct rx_empty_buff buf;
	int ret;
	u32_t data_ptr;

	memset(&buf, 0, sizeof(buf));

	for (i = 0; i < num; i++) {
		/* Reserve a data frag to receive the frame */
		pkt_buf = uwp_pkt_buf_get();
		if (!pkt_buf) {
			LOG_DBG("Could not allocate rx buf %d.", i);
			break;
		}

		data_ptr = (u32_t)pkt_buf;

		SPRD_AP_TO_CP_ADDR(data_ptr);
		memcpy(&(buf.addr[i][0]), &data_ptr, 4);
	}

	if (i == 0) {
		LOG_DBG("No more rx packet buffer.");
		return -EINVAL;
	}

	buf.type = EMPTY_DDR_BUFF;
	buf.num = i;
	buf.common.type = SPRDWL_TYPE_DATA_SPECIAL;
	buf.common.direction_ind = TRANS_FOR_RX_PATH;

	ret = wifi_ipc_send(SMSG_CH_WIFI_DATA_NOR, QUEUE_PRIO_NORMAL,
			(void *)&buf,
			i * SPRDWL_PHYS_LEN + 3,
			WIFI_DATA_NOR_MSG_OFFSET);
	if (ret < 0) {
		LOG_ERR("IPC send fail %d", ret);
		return ret;
	}

	return 0;
}
/*success,ret0;fail,ret<0*/
int wifi_tx_empty_buf(int num)
{
	int ret;
	int group;
	int rest;
	int i;

	if (num > MAX_RX_ADDR_NUM) {
		group = num / MAX_RX_ADDR_NUM;
		rest = num % MAX_RX_ADDR_NUM;
		for (i = 0; i < group; i++) {
			ret = wifi_tx_empty_buf_(MAX_RX_ADDR_NUM);
			if (ret) {
				return ret;
			}
		}
		if (rest) {
			ret = wifi_tx_empty_buf_(rest);
			if (ret) {
				return ret;
			}
		}
	} else {
		ret = wifi_tx_empty_buf_(num);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

#if 0
int wifi_release_rx_buf(void)
{
	struct net_buf *buf = NULL;

	k_mutex_lock(&rx_buf_mutex, K_FOREVER);
	while ((buf = wifi_buf_slist_get(&rx_buf_list))) {
		net_buf_unref(buf);
	}
	k_mutex_unlock(&rx_buf_mutex);

	LOG_DBG("Flush all rx buf");

	return 0;
}


int impl_empty_buf(int num){
    static int i = 0;
    struct rx_empty_buff buf;
    int ret;
    u32_t data_ptr;

    memset(&buf, 0, sizeof(buf));

    /* Reserve a data frag to receive the frame */
    void *pkt_buf = malloc(1600);
    if (!pkt_buf) {
        LOG_ERR("Could not allocate rx buf %d.", i);
        return -ENOMEM;
    }
    LOG_DBG("RXBUF%d:%p", i, pkt_buf);

    data_ptr = (u32_t)pkt_buf;

    SPRD_AP_TO_CP_ADDR(data_ptr);
    memcpy(&(buf.addr[0][0]), &data_ptr, 4);

    buf.type = EMPTY_DDR_BUFF;
    buf.num = 1;
    buf.common.type = SPRDWL_TYPE_DATA_SPECIAL;
    buf.common.direction_ind = TRANS_FOR_RX_PATH;

    ret = wifi_ipc_send(SMSG_CH_WIFI_DATA_NOR, QUEUE_PRIO_NORMAL,
            (void *)&buf,
            1 * SPRDWL_PHYS_LEN + 3,
            WIFI_DATA_NOR_MSG_OFFSET);
    if (ret < 0) {
        LOG_ERR("IPC send fail %d", ret);
        return ret;
    }

    i++;

    return 0;   
}
#endif
static void wifi_rx_event(int ch)
{
    if (ch == SMSG_CH_WIFI_CTRL) {
    	UWP_SEM_GIVE(event_sem);//k_sem_release(event_sem);
    }
}

static void wifi_rx_data(int ch)
{
    if (ch != SMSG_CH_WIFI_DATA_NOR) {
        LOG_ERR("Invalid data channel: %d.", ch);
    }

    UWP_SEM_GIVE(data_sem);//k_sem_release(data_sem);
}
/*success,ret0;fail,ret<0*/
int wifi_tx_cmd(void *data, int len)
{
    return wifi_ipc_send(SMSG_CH_WIFI_CTRL, QUEUE_PRIO_HIGH,
                data, len, WIFI_CTRL_MSG_OFFSET);
}

int wifi_tx_data(void *data, int len)
{
    //ARG_UNUSED(len);

    int ret;
    struct hw_addr_buff_t addr_buf;

    memset(&addr_buf, 0, sizeof(struct hw_addr_buff_t));
    addr_buf.common.interface = 0;
    addr_buf.common.type = SPRDWL_TYPE_DATA_SPECIAL;
    addr_buf.common.direction_ind = 0;
    addr_buf.common.buffer_type = 1;
    addr_buf.number = 1;
    addr_buf.offset = 7;
    addr_buf.tx_ctrl.pcie_mh_readcomp = 1;

    memset(addr_buf.pcie_addr, 0, SPRDWL_PHYS_LEN);
    memcpy(addr_buf.pcie_addr, &data, 4);  /* Copy addr to addr buf. */

    ret = wifi_ipc_send(SMSG_CH_WIFI_DATA_NOR, QUEUE_PRIO_NORMAL,
                (void *)&addr_buf,
                1 * SPRDWL_PHYS_LEN + sizeof(struct hw_addr_buff_t),
                WIFI_DATA_NOR_MSG_OFFSET);
    if (ret < 0) {
        LOG_ERR("IPC send fail %d", ret);
        return ret;
    }

    return 0;
}
static void rx_data_thread(const char*p1)
{
    int ret;
    struct wifi_priv *priv = (struct wifi_priv *)p1;
    u8_t *addr = rx_data_buf;
    int len;

    while (1) {
		ret = UWP_SEM_TAKE(data_sem, UWP_DELAY_FOREVER);//uwp_sem_acquire(data_sem, UWP_FOREVER);
	    if(ret == 0){
	        printk("event_sem mutex lock failed.");
	        //return; /* lock mutex failed */
	    }
        while (1) {
            memset(addr, 0, RX_DATA_SIZE);
            ret = wifi_ipc_recv(SMSG_CH_WIFI_DATA_NOR,
                    addr, &len, 0);
            if (ret == 0) {
                LOG_DBG("Receive data %p len %i",
                        addr, len);
                wifi_data_process(priv, addr, len);
            } else {
                break;
            }
            //portYIELD();
        }
    }
}
static void rx_cmdevt_thread(const char*p1)
{
    int ret;
    struct wifi_priv *priv = (struct wifi_priv *)p1;
    u8_t *addr = rx_cmdevt_buf;
    int len;

    while (1) {
        LOG_DBG("Wait for cmdevt.");
		ret = UWP_SEM_TAKE(event_sem, UWP_DELAY_FOREVER);//uwp_sem_acquire(event_sem, UWP_FOREVER);
	    if(ret == 0){
	        LOG_ERR("event_sem mutex lock failed.");
	        //return; /* lock mutex failed */
	    }
        while (1) {
            LOG_DBG("rx_cmdevt_thread.");
            memset(addr, 0, RX_CMDEVT_SIZE);
            ret = wifi_ipc_recv(SMSG_CH_WIFI_CTRL,
                    addr, &len, 0);
            if (ret == 0) {
                LOG_DBG("Receive cmd/evt %p len %i, status=%d",
                    addr, len,(((struct trans_hdr *)addr)->status));
                wifi_cmdevt_process(priv, addr, len);
            } else {
                break;
            }
        }
    }
}
/*success ret=0*/
int wifi_txrx_init(struct wifi_priv *priv)
{
    int ret = 0;
    LOG_DBG("wifi_txrx_init,enter\r\n");

    UWP_SEM_INIT(event_sem, 1, 0);//event_sem = uwp_sem_create(1, 0);
    UWP_SEM_INIT(data_sem, 15, 0);//data_sem = uwp_sem_create(15, 0);

    UWP_MUTEX_INIT(rx_buf_mutex);//rx_buf_mutex = uwp_mutex_create();

    //wifi_buf_slist_init(&rx_buf_list);
    ret = uwp_pkt_buf_init();
    if (ret < 0) {
        LOG_ERR("uwp_pkt_buf_init failed.");
        return ret;
    }
    ret = wifi_ipc_create_channel(SMSG_CH_WIFI_CTRL,
                      wifi_rx_event);
    if (ret < 0) {
        LOG_ERR("Create wifi control channel failed.");
        return ret;
    }

    ret = wifi_ipc_create_channel(SMSG_CH_WIFI_DATA_NOR,
                      wifi_rx_data);
    if (ret < 0) {
        LOG_ERR("Create wifi data channel failed.");
        return ret;
    }

    ret = wifi_ipc_create_channel(SMSG_CH_WIFI_DATA_SPEC,
                      wifi_rx_data);
    if (ret < 0) {
        LOG_ERR("Create wifi data channel failed.");
        return ret;
    }

    UWPTaskCreate(rx_cmdevt_thread, "rx_cmdevt_thread", RX_STACK_SIZE, (void *)priv, uwpPriorityNormal, &rx_cmdevt_thread_data);
    UWPTaskCreate(rx_data_thread, "rx_data_thread", RX_STACK_SIZE, (void *)priv, 6, &rx_data_thread_data);
    //uwp_thread_create("rx_cmdevt_thread", rx_cmdevt_thread, (void *)priv, NULL, RX_STACK_SIZE, uwpPriorityNormal);
    //uwp_thread_create("rx_data_thread", rx_data_thread, (void *)priv, NULL, RX_STACK_SIZE, uwpPriorityNormal);
    LOG_DBG("wifi_txrx_init,exit\r\n");
    return 0;
}
