/*
 * Copyright (c) 2018 Texas Instruments, Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief General WiFi Definitions
 */

#ifndef MBED_UWP_WIFI_H
#define MBED_UWP_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

enum wifi_security_type {
    WIFI_SECURITY_TYPE_OPEN = 1,
    WIFI_SECURITY_TYPE_PSK,
    WIFI_SECURITY_OTHERS,
};

#define NET_LINK_ADDR_MAX_LENGTH 6

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PSK_MAX_LEN 64

#define WIFI_CHANNEL_MAX 14
#define WIFI_CHANNEL_ANY 255

#define WIFI_BAND_2_4G	(1)
#define WIFI_BAND_5G	(2)


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_NET_WIFI_H_ */
