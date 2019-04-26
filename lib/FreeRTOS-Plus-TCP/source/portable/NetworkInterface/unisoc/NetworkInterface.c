/*
FreeRTOS+TCP V2.0.10
Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 http://aws.amazon.com/freertos
 http://www.FreeRTOS.org
*/

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_DNS.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#include "uwp_sys_wrapper.h"
#include "uwp_log.h"
#include "aws_wifi.h"
#include "uwp_wifi_main.h"
#include "uwp_wifi_txrx.h"

enum if_state_t {
    INTERFACE_DOWN = 0,
    INTERFACE_UP,
};

enum if_idx {
	INTERFACE_STA = 1,
	INTERFACE_AP,
};

static int more_space = sizeof(struct tx_msdu_dscr) + 4;//4byte to save buffer addr of struct tx_msdu_dscr
volatile static uint32_t xInterfaceState = INTERFACE_DOWN;
extern struct wifi_priv uwp_wifi_priv;

static inline void save_dscr_addr_before_buffer_addr(u32_t payload, void *addr)
{
	u32_t *pkt_ptr;

	pkt_ptr = (u32_t *)(payload - (sizeof(struct tx_msdu_dscr)+4+4));
	*pkt_ptr = (u32_t)addr;
}

static inline u32_t get_dscr_addr_from_buffer_addr(u32_t payload)
{
	u32_t *ptr;

	ptr = (u32_t *)(payload - (sizeof(struct tx_msdu_dscr)+4+4));

	return *ptr;
}

BaseType_t xNetworkInterfaceInitialise( void )
{
    static BaseType_t xMACAdrInitialized = pdFALSE;
    uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ];

    if(!uwp_wifi_priv.wifi_dev[0].opened)
        return pdFALSE;
    else {
            if (xMACAdrInitialized == pdFALSE) {
                WIFI_GetMAC(ucMACAddress);
                FreeRTOS_UpdateMACAddress(ucMACAddress);
                xMACAdrInitialized = pdTRUE;
            }
            return pdTRUE;
    }
    return pdTRUE;

}

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t *const pxNetworkBuffer, BaseType_t xReleaseAfterSend )
{
	const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS( 250 );
    int ret = -1;
    u8_t *data = NULL;
    u8_t *alloc_ptr = NULL;
    int len = 0;

    if (pxNetworkBuffer == NULL || pxNetworkBuffer->pucEthernetBuffer == NULL || pxNetworkBuffer->xDataLength == 0) {
        LOG_ERR("Invalid params");
        return pdFALSE;
    }

    len = pxNetworkBuffer->xDataLength;
    data = (u8_t *)pvPortMalloc(len + more_space);
    if (data == NULL) {
        printk("%s alloc buffer failed.\r\n", __func__);
        goto fail;
    }

    alloc_ptr = data;
    data += more_space;
    //save_dscr_addr_before_buffer_addr((u32_t)(data), (void *)alloc_ptr);

    memcpy(data, pxNetworkBuffer->pucEthernetBuffer, len);

    if (xReleaseAfterSend) { //driver should release the NetworkBufferDescriptor_t
        vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
    }
    ret = uwp_mgmt_tx(data, len);
    if (ret != UWP_OK) {
        LOG_DBG("Failed to tx buffer %p, len %d, err %d\r\n",
                pxNetworkBuffer->pucEthernetBuffer,
                pxNetworkBuffer->xDataLength, ret);
        vPortFree(alloc_ptr);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
    return ret == UWP_OK ? pdTRUE : pdFALSE;

fail:
    if (xReleaseAfterSend)
        vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
    return pdFALSE;
}

void vNetworkNotifyIFDown()
{
    IPStackEvent_t xRxEvent = { eNetworkDownEvent, NULL };
    xInterfaceState = INTERFACE_DOWN;
    xSendEventStructToIPTask( &xRxEvent, 0 );
}

void vNetworkNotifyIFUp()
{
    xInterfaceState = INTERFACE_UP;
}

void* wlanif_alloc_network_buffer(uint16_t len)
{
	NetworkBufferDescriptor_t *pxNetworkBuffer;
    const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS( 250 );

	pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(len, xDescriptorWaitTime);

	return pxNetworkBuffer;
}

void wlanif_free_network_buffer(uint32_t addr)
{
    void *ptr = (void*)addr - more_space;
    if (((u32_t)ptr < 0x00180000) || ((u32_t)ptr > 0x001e4000)) {
        printk("invalid buffer do not free:%p\r\n",ptr);
        return;
    }

    vPortFree(ptr);
}

BaseType_t wlanif_input(void *netif, void *buffer, uint16_t len)
{
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    IPStackEvent_t xRxEvent = { eNetworkRxEvent, NULL };
    const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS( 250 );

    if( eConsiderFrameForProcessing( buffer ) != eProcessBuffer ) {
        //printk("Dropping packet");
        return UWP_OK;
    }

    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(len, xDescriptorWaitTime);
    if (pxNetworkBuffer != NULL) {

	    /* Set the packet size, in case a larger buffer was returned. */
	    pxNetworkBuffer->xDataLength = len;

	    /* Copy the packet data. */
        memcpy(pxNetworkBuffer->pucEthernetBuffer, buffer, len);
        DUMP_DATA(pxNetworkBuffer->pucEthernetBuffer,len);

        xRxEvent.pvData = (void *) pxNetworkBuffer;
        if ( xSendEventStructToIPTask( &xRxEvent, xDescriptorWaitTime) == pdFAIL ) {
            printk("Failed to enqueue packet to network stack %p, len %d", buffer, len);
            vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
            return UWP_FAIL;
        }
        return UWP_OK;
    } else {
        printk("Failed to get buffer descriptor");
        return UWP_FAIL;
    }
}


void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    /* FIX ME. */
}

BaseType_t xGetPhyLinkStatus( void )
{
    /* FIX ME. */
    return pdFALSE;
}

