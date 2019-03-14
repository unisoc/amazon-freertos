/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed_retarget.h"
#include "sblock.h"
#include "sipc.h"

//#define WIFI_LOG_DBG
//#define WIFI_DUMP
#include "uwp_log.h"
/*success ret0*/
int wifi_ipc_create_channel(int ch, void (*callback)(int ch))
{
	int ret = 0;

	if (!callback) {
		LOG_ERR("Invalid callback.");
		return -EINVAL;
	}

	switch (ch) {
	case SMSG_CH_WIFI_CTRL:
		ret = sblock_create(0, ch,
				CTRLPATH_TX_BLOCK_NUM,
				CTRLPATH_TX_BLOCK_SIZE,
				CTRLPATH_RX_BLOCK_NUM,
				CTRLPATH_RX_BLOCK_SIZE);
		break;
	case SMSG_CH_WIFI_DATA_NOR:
		ret = sblock_create(0, ch,
				DATAPATH_NOR_TX_BLOCK_NUM,
				DATAPATH_NOR_TX_BLOCK_SIZE,
				DATAPATH_NOR_RX_BLOCK_NUM,
				DATAPATH_NOR_RX_BLOCK_SIZE);
		break;
	case SMSG_CH_WIFI_DATA_SPEC:
		ret = sblock_create(0, ch,
				DATAPATH_SPEC_TX_BLOCK_NUM,
				DATAPATH_SPEC_TX_BLOCK_SIZE,
				DATAPATH_SPEC_RX_BLOCK_NUM,
				DATAPATH_SPEC_RX_BLOCK_SIZE);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret < 0) {
		LOG_ERR("Ipc create channel %d failed.", ch);
		return ret;
	}

	ret = sblock_register_callback(ch, callback);
	if (ret < 0) {
		LOG_ERR("Register ipc callback failed");
		return ret;
	}

	return ret;
}
/*success,ret0;fail,ret<0*/
int wifi_ipc_send(int ch, int prio, void *data, int len, int offset)
{
	int ret;
	struct sblock blk;

	ret = sblock_get(0, ch, &blk, 0);
	if (ret) {
		LOG_ERR("Get block error: ch=%d,ret=%d", ch, ret);
		return ret;
	}
	LOG_DBG("IPC Channel %d Send data:", ch);
	memcpy((char *)blk.addr + BLOCK_HEADROOM_SIZE + offset, data, len);

#if 0  
			u8_t *temp_addr = (u8_t *)blk.addr;
			int temp_len = len+BLOCK_HEADROOM_SIZE+offset;
			DUMP_DATA(temp_addr,temp_len);
#endif

	blk.length = len + offset;
	ret = sblock_send(0, ch, prio, &blk);

	return ret;
}

int wifi_ipc_recv(int ch, u8_t *data, int *len, int offset)
{
	int ret;
	struct sblock blk;

	ret = sblock_receive(0, ch, &blk, 0);
	if (ret) {
		//LOG_ERR("wifi_ipc_recv,sblock_receive=%d",ret);
		return ret;
	}

	memcpy(data, blk.addr, blk.length);
	*len = blk.length;

	LOG_DBG("IPC Channel %d Get data:", ch);

	sblock_release(0, ch, &blk);
	if (ret) {
		LOG_ERR("wifi_ipc_recv,sblock_release=%d",ret);
		return ret;
	}
	return ret;
}
