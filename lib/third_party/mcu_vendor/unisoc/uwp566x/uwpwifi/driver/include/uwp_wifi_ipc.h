/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __UWP_WIFI_IPC_H
#define __UWP_WIFI_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

int wifi_ipc_send(int ch, int prio, void *data, int len, int offset);
int wifi_ipc_recv(int ch, u8_t *data, int *len, int offset);
int wifi_ipc_create_channel(int ch, void (*callback)(int ch));

#ifdef __cplusplus
}
#endif

#endif /* UWP_WIFI_IPC_H */
