/*
 * @file
 * @brief PSK header
 */

/*
 * Copyright (c) 2019, Unisoc Communications Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _WIFIMGR_PSK_H_
#define _WIFIMGR_PSK_H_

#define WIFI_PMK_LEN		32
#define WIFI_PMK_ITER	4096

//#include <sys/types.h>

int pbkdf2_sha1(const char *passphrase, const char *ssid, int iterations,
		char *buf, size_t buflen);

#endif
