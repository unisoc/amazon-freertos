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
//#include "sdkconfig.h"
//#include "uwp566x_types.h"
#include "uwp_log.h"
//#include "uwp_err.h"

#include "aws_wifi.h"
//#include "wifi_main.h"

//#include "uwp5661_log.h"
//#include "uwp5661_wifi.h"
//#include "uwp5661_wifi_internal.h"
//#include "tcpip_adapter.h"

enum if_state_t {
    INTERFACE_DOWN = 0,
    INTERFACE_UP,
};

enum if_idx {
	INTERFACE_STA = 1,
	INTERFACE_AP,
};

volatile static uint32_t xInterfaceState = INTERFACE_DOWN;

BaseType_t xNetworkInterfaceInitialise( void )
{
    static BaseType_t xMACAdrInitialized = pdFALSE;
    uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ];

    if (xInterfaceState == INTERFACE_UP) {
        if (xMACAdrInitialized == pdFALSE) {
            WIFI_GetMAC(ucMACAddress);
            FreeRTOS_UpdateMACAddress(ucMACAddress);
            xMACAdrInitialized = pdTRUE;
        }
        return pdTRUE;
    }
    return pdFALSE;
}

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t *const pxNetworkBuffer, BaseType_t xReleaseAfterSend )
{
	NetworkBufferDescriptor_t * pxSendingBuffer;
	const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS( 250 );
    int ret = -1;

    if (pxNetworkBuffer == NULL || pxNetworkBuffer->pucEthernetBuffer == NULL || pxNetworkBuffer->xDataLength == 0) {
        LOG_ERR("Invalid params");
        return pdFALSE;
    }

    if (xReleaseAfterSend == pdFALSE) {
        // Duplicate Network descriptor and buffer
        pxSendingBuffer = pxGetNetworkBufferWithDescriptor(pxNetworkBuffer->xDataLength, xDescriptorWaitTime);
            if (pxSendingBuffer != NULL) {
                memcpy((uint8_t *)pxSendingBuffer, (uint8_t *)pxNetworkBuffer, sizeof(NetworkBufferDescriptor_t));
                memcpy(pxSendingBuffer->pucEthernetBuffer,
                        pxNetworkBuffer->pucEthernetBuffer,
                        pxSendingBuffer->xDataLength);

            } else {
                return pdFALSE;
            }
    } else {
        pxSendingBuffer = pxNetworkBuffer;
    }

    ret = uwp_iface_tx(INTERFACE_STA, pxSendingBuffer->pucEthernetBuffer,
        pxSendingBuffer->xDataLength);
    if (ret != UWP_OK) {
        LOG_ERR("Failed to tx buffer %p, len %d, err %d",
                pxSendingBuffer->pucEthernetBuffer,
                pxSendingBuffer->xDataLength, ret);
        vReleaseNetworkBufferAndDescriptor(pxSendingBuffer);
    }

    return ret == UWP_OK ? pdTRUE : pdFALSE;
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
	NetworkBufferDescriptor_t *pxNetworkBuffer =
			(NetworkBufferDescriptor_t *)(*(uint32_t *)(addr - ipconfigBUFFER_PADDING));

	vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);

	return;
}

BaseType_t wlanif_input(void *netif, void *buffer, uint16_t len)
{
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    IPStackEvent_t xRxEvent = { eNetworkRxEvent, NULL };
    const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS( 250 );

    if( eConsiderFrameForProcessing( buffer ) != eProcessBuffer ) {
        LOG_ERR("Dropping packet");
        return UWP_OK;
    }

    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(len, xDescriptorWaitTime);
    if (pxNetworkBuffer != NULL) {

	/* Set the packet size, in case a larger buffer was returned. */
	pxNetworkBuffer->xDataLength = len;

	/* Copy the packet data. */
        memcpy(pxNetworkBuffer->pucEthernetBuffer, buffer, len);
        xRxEvent.pvData = (void *) pxNetworkBuffer;

        if ( xSendEventStructToIPTask( &xRxEvent, xDescriptorWaitTime) == pdFAIL ) {
            LOG_ERR("Failed to enqueue packet to network stack %p, len %d", buffer, len);
            vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
            return UWP_FAIL;
        }
        return UWP_OK;
    } else {
        LOG_ERR("Failed to get buffer descriptor");
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

#if 0
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
driver will filter incoming packets and only pass the stack those packets it
considers need processing. */
#if( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0 )
#define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer ) eProcessBuffer
#else
#define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer ) eConsiderFrameForProcessing( ( pucEthernetBuffer ) )
#endif

BaseType_t xNetworkInterfaceInitialise( void )
{
    /* FIX ME. */
    return pdFALSE;
}

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend )
{
    /* FIX ME. */
    return pdFALSE;
}

#endif
