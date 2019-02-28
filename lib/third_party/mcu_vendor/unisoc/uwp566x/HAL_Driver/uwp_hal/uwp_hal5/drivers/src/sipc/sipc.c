/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <string.h>

#include "sipc.h"
#include "uwp_log.h"

extern int smsg_init(u32_t dst, u32_t smsg_base);
int sipc_init(void)
{
	int ret;

	LOG_INF("sipc init start.");

	ret = smsg_init(IPC_DST, IPC_RING_ADDR);
	if (ret) {
		LOG_ERR("sipc init failed.");
		return ret;
	}

	LOG_INF("sipc init success.");

	return 0;
}

