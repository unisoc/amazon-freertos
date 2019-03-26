/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_UWP_WIFI_CMDEVT_H
#define MBED_UWP_WIFI_CMDEVT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uwp_sys_wrapper.h"
#include "uwp_wifi_main.h"

#define MAX_SSID_LEN (33) /* SSID end with 0 */
#define MAX_KEY_LEN (128) /* FIXME: Max size 64 */
#define MAX_AP_KEY_LEN (64)


enum cmd_type {
	WIFI_CMD_BEGIN = 0x00,
	/* Common command. */
	WIFI_CMD_GET_CP_INFO,   /* Get cp version number. */
	WIFI_CMD_DOWNLOAD_INI,
	WIFI_CMD_OPEN,          /* Open a wifi mode, ap or sta. */
	WIFI_CMD_CLOSE,         /* Stop wifi mode. */
	WIFI_CMD_NPI_MSG,

	/* Sta command. */
	WIFI_CMD_SCAN,
	WIFI_CMD_CONNECT,
	WIFI_CMD_DISCONNECT,
	WIFI_CMD_GET_STATION,   /* Get sta's RSSI and tx rate. */
	/* Ap command. */
	WIFI_CMD_START_AP,      /* Start ap mode. */
	WIFI_CMD_DEL_STATION,
	WIFI_CMD_SET_BLACKLIST,
	WIFI_CMD_SET_WHITELIST,

	WIFI_CMD_SET_IP = 0x0F, /* Set IP address. */

	WIFI_CMD_MAX,
};

enum event_type {
	WIFI_EVENT_BEGIN = 0x80,
	/* Sta event. */
	WIFI_EVENT_CONNECT,
	WIFI_EVENT_DISCONNECT,
	WIFI_EVENT_SCAN_RESULT,
	WIFI_EVENT_SCAN_DONE,
	/* Ap event. */
	WIFI_EVENT_NEW_STATION,
	WIFI_EVENT_MAX,
};

/* These structures are shared by command and event. */
PACK_STRUCT_BEGIN
struct trans_hdr {
	u8_t type;      /* Which command or event will be transferred. */
	u8_t seq;       /* Event sequence. */
	u8_t response;  /* 1 for command response. 0 for cp event. */
	char status;    /* 0 for response success, other for fail. */
	u16_t len;      /* Message length. */
	char data[0];   /* The start of message data. */
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_download_ini {
	struct trans_hdr trans_header;
	u32_t sec_num;
	char data[0];
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_open {
	struct trans_hdr trans_header;
	char mode;
	char mac[ETH_ALEN];
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_get_cp_info {
	struct trans_hdr trans_header;
	u32_t version;
	char mac[ETH_ALEN];
    u8_t max_ap_assoc_sta_num;
    u8_t max_ap_blacklist_sta_num;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_stop {
	struct trans_hdr trans_header;
	char mode;
	char mac[ETH_ALEN];
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

/* Command struct for ap. */
PACK_STRUCT_BEGIN
struct cmd_start_ap {
	struct trans_hdr trans_header;
	u8_t ssid_len;
	u8_t ssid[MAX_SSID_LEN];
	u8_t password_len;
	char password[MAX_AP_KEY_LEN];
	u8_t channel;
	u8_t vht_chwidth;
	u8_t vht_chan_center_freq_seg0_idx;
	u8_t vht_chan_center_freq_seg1_idx;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END
	
PACK_STRUCT_BEGIN
struct cmd_del_sta {
    struct trans_hdr trans_header;
	/**
	 * If mac set to FF:FF:FF:FF:FF:FF,
	 * all station will be disconnected.
	 */
	u8_t mac[ETH_ALEN];
	u16_t reason_code;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END


/* Command struct for sta. */
PACK_STRUCT_BEGIN
struct cmd_scan {
	struct trans_hdr trans_header;
	u32_t channels_2g; /* One bit for one 2.4G channel. */
	u32_t flags;
	u16_t ssid_len; /* Hidden ssid length. */
	u8_t ssid[0]; /* FIXME: Invalid reservation. Hidden ssid.  */
	u16_t channels_5g_cnt; /* Number of 5G channels. */
	u16_t channels_5g[0]; /* 5G channels to be scanned. */
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_connect {
	struct trans_hdr trans_header;
	u32_t wpa_versions;
	u8_t bssid[ETH_ALEN];
	u8_t channel;
	u8_t auth_type;
	u8_t pairwise_cipher;
	u8_t group_cipher;
	u8_t key_mgmt;
	u8_t mfp_enable;
	u8_t psk_len;
	u8_t ssid_len;
	u8_t psk[MAX_KEY_LEN];
	u8_t ssid[MAX_SSID_LEN];
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_disconnect {
	struct trans_hdr trans_header;
	u8_t reason_code;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_set_ip {
	struct trans_hdr trans_header;
	/**
	 * IPV4 address
	 * TODO: For compatibility with ipv6,
	 * it might be greater than 4 bytes.
	 */
	u8_t ip[IPV4_LEN];
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_get_sta {
    struct trans_hdr trans_header;
    /**
     * Response value
     * Now needs signal only.
     */
    u8_t reserved[5];
    s8_t signal;
    u8_t reserved_1[6];
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct cmd_npi {
    struct trans_hdr trans_header;
    u8_t data[0];
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct event_scan_result {
    u8_t band;
    u8_t channel;
    s8_t rssi;
    u8_t encrypt_mode;
    char bssid[ETH_ALEN];
    char ssid[MAX_SSID_LEN];
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct event_scan_done {
/* #define WIFI_EVENT_SCAN_SUCC (0) */
/* #define WIFI_EVENT_SCAN_ERROR (1) */
	u8_t status;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct event_connect {
    u8_t status;
    u8_t bssid[ETH_ALEN];
    u8_t primary_chan_num;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct event_disconnect {
	u8_t reason_code;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct event_new_station {
	u8_t is_connect; /* 1 for connected, 0 for disconnected. */
	u8_t mac[ETH_ALEN];
	/* u16_t ie_len; */
	/* u8_t ie[0]; */
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END
typedef struct{
    struct event_scan_result res;
    struct list_head res_list;
}scan_result_info_t;
struct scan_list{
    char g_scan_num;
    struct list_head g_scan_list;
};
/* int wifi_cmd_load_ini(u8_t *pAd); */
/* int wifi_cmd_set_sta_connect_info(u8_t *pAd, char *ssid, char *key); */

int wifi_cmd_get_cp_info(struct wifi_priv *priv);
int wifi_cmd_open(struct wifi_device *wifi_dev);
int wifi_cmd_close(struct wifi_device *wifi_dev);
int wifi_cmd_scan(struct wifi_device *wifi_dev, struct wifi_drv_scan_params *params);
int wifi_cmd_connect(struct wifi_device *wifi_dev, struct wifi_drv_connect_params *params);
int wifi_cmd_disconnect(struct wifi_device *wifi_dev);
/*
int wifi_cmd_start_ap(struct wifi_device *wifi_dev,
		struct wifi_drv_start_ap_params *params);
int wifi_cmd_stop_ap(struct wifi_device *wifi_dev);
int wifi_cmd_npi_send(struct device *dev, int ictx_id, char *t_buf,
		u32_t t_len, char *r_buf, u32_t *r_len);
int wifi_cmd_npi_get_mac(struct device *dev, char *buf);
int wifi_cmd_set_ip(struct wifi_device *wifi_dev, u8_t *ip, u8_t len);
*/
int wifi_cmd_send(int u32cmd, char *data, int len, char *rbuf, int *rlen);
int wifi_cmd_load_ini(const u8_t *data, u32_t len, u8_t sec_num);
int wifi_cmdevt_process(struct wifi_priv *priv, char *data, int len);
int wifi_cmdevt_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UWP_WIFI_CMDEVT_H */
