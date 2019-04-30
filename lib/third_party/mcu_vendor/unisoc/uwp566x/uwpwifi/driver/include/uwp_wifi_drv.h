/*
 * @file
 * @brief WiFi manager callbacks for the WiFi driver
 */

/*
 * Copyright (c) 2018, Unisoc Communications Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_UWP_WIFI_DRV_H
#define MBED_UWP_WIFI_DRV_H

#ifdef __cplusplus
extern "C"{
#endif

#include "uwp_wifi.h"

struct wifi_drv_connect_params {
	char *ssid;
	char ssid_length; /* Max 32 */
	char *psk;
	char psk_length; /* Min 8 - Max 64 */
	unsigned char channel;
	enum wifi_security_type security;
};

struct wifi_drv_start_ap_params {
	char *ssid;
	char ssid_length; /* Max 32 */
	char *psk;
	char psk_length; /* Min 8 - Max 64 */
	unsigned char channel;
	enum wifi_security_type security;
};

struct wifi_drv_scan_params {
	unsigned char band;
	unsigned char channel;
    //char ssid[WIFI_SSID_MAX_LEN];
    //unsigned char ssid_len;
};

struct wifi_drv_scan_result {
	char bssid[NET_LINK_ADDR_MAX_LENGTH];
	char ssid[WIFI_SSID_MAX_LEN];
	char ssid_length;
	unsigned char channel;
	char rssi;
	enum wifi_security_type security;
};

typedef void (*scan_result_cb_t)(void *iface, int status,
				 struct wifi_drv_scan_result *entry);
typedef void (*connect_cb_t)(int netif);
typedef void (*disconnect_cb_t)(int netif);
typedef void (*new_station_t)(void *iface, int status, char *mac);

#if 0
struct wifi_drv_api{
	/**
	 * Mandatory to get in first position.
	 * A network device should indeed provide a pointer on such
	 * ethernet_api structure. So we make current structure pointer
	 * that can be casted to a net_if_api structure pointer.
	 */
	struct ethernet_api eth_api;

	int (*open)(struct device *dev);
	int (*close)(struct device *dev);
	int (*scan)(struct device *dev, struct wifi_drv_scan_params *params,
		    scan_result_cb_t cb);
	int (*connect)(struct device *dev,
		       struct wifi_drv_connect_params *params,
		       connect_cb_t conn_cb, disconnect_cb_t disc_cb);
	int (*disconnect)(struct device *dev, disconnect_cb_t cb);
	int (*get_station)(struct device *dev, u8_t *signal);
	int (*notify_ip)(struct device *dev, u8_t *ipaddr, u8_t len);
	int (*start_ap)(struct device *dev,
			struct wifi_drv_start_ap_params *params,
			new_station_t cb);
	int (*stop_ap)(struct device *dev);
	int (*del_station)(struct device *dev, u8_t *mac);
};
#endif

#ifdef __cplusplus
}
#endif

#endif
