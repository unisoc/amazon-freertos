#ifndef __UWP_BUF_MGMT_H
#define __UWP_BUF_MGMT_H

#ifdef __cplusplus
extern "C"{
#endif

#include "uwp_sys_wrapper.h"
#include "hal_config_def.h"

extern int wifi_rx_employ_num;
extern int wifi_rx_free_num;

typedef struct{
    void *buf;
    struct list_head list;
}uwp_pkt_buf;

int uwp_pkt_buf_init(void);
void *uwp_pkt_buf_get(void);
int uwp_pkt_buf_free(void *buf);

#ifdef __cplusplus
}
#endif

#endif

