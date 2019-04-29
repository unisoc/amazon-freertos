/*
 * Amazon FreeRTOS Wi-Fi V1.0.0
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file aws_wifi.c
 * @brief Wi-Fi Interface.
 */

/* Socket and Wi-Fi interface includes. */
#include "FreeRTOS.h"

#include "aws_wifi.h"

/* Wi-Fi configuration includes. */
#include "aws_wifi_config.h"

#include "uwp_wifi_main.h"
#include "uwp_sys_wrapper.h"
#include "uwp_log.h"
#include "uwp_wifi_cmdevt.h"
#include "FreeRTOS_IP.h"

extern struct wifi_priv uwp_wifi_priv;
extern struct scan_list uwp_scan_list;
extern struct scanResult uwp_scanResult;
void* uwp_netif_up;//The sem of netifup for waiting flag of netif
static SemaphoreHandle_t xWiFiSem;
static const TickType_t xSemaphoreWaitTicks = pdMS_TO_TICKS( wificonfigMAX_SEMAPHORE_WAIT_TIME_MS );
//static WIFIDeviceMode_t prvcurDeviceMode;

#define CHECK_VALID_SSID_LEN(x) \
        ((x) > 0 && (x) <=  wificonfigMAX_SSID_LEN)
#define CHECK_VALID_PASSPHRASE_LEN(x) \
        ((x) > 0 && (x) <= wificonfigMAX_PASSPHRASE_LEN)
#define CHECK_VALID_HOSTNAME_LEN(x) \
        ((x) > 0 && (x) <= 253)

//extern int uwpNetifStatus;//the netifup flag for get ip 

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_On( void )
{
    /* FIX ME. */
	int ret = eWiFiFailure;

	LOG_ERR("WIFI_On enter\r\n");


   if(uwp_wifi_initDone() && uwp_wifi_opened(WIFI_DEV_STA))
   {
		return eWiFiSuccess;
   }
   else
   {
	   if(sipc_init() != 0) {
		   LOG_ERR("sipc init failed");
		   return eWiFiFailure;
	   }

	   if(uwp_init(&uwp_wifi_priv, WIFI_MODE_STA)) {
		   LOG_ERR("uwp init failed");
		   return eWiFiFailure;
	   }
	   if(!uwp_wifi_opened(WIFI_DEV_STA)) {//if not opened
		   ret = uwp_mgmt_open(&uwp_wifi_priv);
			if(ret) {
				LOG_ERR("WIFI_On,open failed");
				return eWiFiFailure;
        	}
   		}
		static StaticSemaphore_t xSemaphoreBuffer;
		xWiFiSem = xSemaphoreCreateMutexStatic( &( xSemaphoreBuffer ) );

		UWP_SEM_INIT(uwp_netif_up, 1, 0);

   }
   LOG_ERR("WIFI_On done\r\n");
   return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
    /* FIX ME. */
	int ret = UWP_FAIL;
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    LOG_ERR("WIFI_Off\r\n");
    /* Try to acquire the semaphore. */
#if 0
    if( (uwpinitDone != pdFALSE) && (xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE) )
    {
    	ret = uwp_mgmt_close(&uwp_wifi_priv);
    	if(ret == UWP_OK)
    	{
			xRetVal = eWiFiSuccess;
			uwpinitDone = pdFALSE;
    	}
		/* Return the semaphore. */
    	xSemaphoreGive( xWiFiSem );
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_Off,rt=%d\r\n",xRetVal);
    return xRetVal;
#endif
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/
#define WAIT_DHCP_DELAY       pdMS_TO_TICKS( 6000 )
WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    /* FIX ME. */

	int ret = UWP_FAIL;
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    struct wifi_drv_scan_params scan_params;
    struct wifi_drv_connect_params connect_params;
//for thread-safe
	if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE ) {
		LOG_ERR("WIFI_ConnectAP,enter\r\n");

	if(uwp_wifi_isConnected(WIFI_DEV_STA)) {//if connected
        ret = uwp_mgmt_disconnect(&uwp_wifi_priv);
        FreeRTOS_NetworkDown();
		if( ret != eWiFiSuccess) {
			LOG_ERR("WIFI_ConnectAP,disconnect fail\r\n");
			xSemaphoreGive( xWiFiSem );//must give the sem
			return eWiFiFailure;
		}
	}

	if (pxNetworkParams == NULL || pxNetworkParams->pcSSID == NULL || pxNetworkParams->pcPassword == NULL) {
		LOG_ERR("invalid params!\r\n");
		xSemaphoreGive( xWiFiSem );//must give the sem
		return eWiFiFailure;
	}

	if (!CHECK_VALID_SSID_LEN(pxNetworkParams->ucSSIDLength) ||
		(pxNetworkParams->xSecurity != eWiFiSecurityOpen && !CHECK_VALID_PASSPHRASE_LEN(pxNetworkParams->ucPasswordLength))) {
		LOG_ERR("invalid params 2!\r\n");
		xSemaphoreGive( xWiFiSem );//must give the sem
		return eWiFiFailure;
	}

	//fixme: check if wifi on.
	if(!uwp_wifi_opened(WIFI_DEV_STA)) {
		LOG_ERR("wifi not open!\r\n");
		xSemaphoreGive( xWiFiSem );//must give the sem
		return eWiFiFailure;
    }


        //scan param
        scan_params.band = 0;
        scan_params.channel = 0;
#ifndef SCANP
        ret = uwp_mgmt_scan(&uwp_wifi_priv, &scan_params);
#else
        ret = uwp_mgmt_scan(&uwp_wifi_priv, &scan_params, NULL, 0);
#endif
        if(ret) {
        	LOG_ERR("scan failed\r\n");
        	xSemaphoreGive( xWiFiSem );//must give the sem
            return eWiFiFailure;
        }

        /* SSID is mandatory */
        connect_params.ssid_length = strlen(pxNetworkParams->pcSSID);
        if (!pxNetworkParams->pcSSID || !connect_params.ssid_length) {
            xSemaphoreGive( xWiFiSem );//must give the sem
            return eWiFiFailure;
        }
        connect_params.ssid = pxNetworkParams->pcSSID;

        /* BSSID is optional */

        /* Passphrase is only valid for WPA/WPA2-PSK */
        connect_params.psk_length = strlen(pxNetworkParams->pcPassword);
        //if (pxNetworkParams->pcPassword && !connect_params.psk_length)
        //	return -eWiFiFailure;

        connect_params.psk = pxNetworkParams->pcPassword;
        FreeRTOS_NetworkDown();//dhcp reset to 0 because process of scan waste 2s, dhcp timeout is 5*2^n
        ret = uwp_mgmt_connect(&uwp_wifi_priv, &connect_params);
        if(ret) {
            LOG_ERR("connect failed\r\n");
            FreeRTOS_NetworkDown();
            xSemaphoreGive( xWiFiSem );//must give the sem
            return eWiFiFailure;
        }
        else {
			ret = UWP_SEM_TAKE(uwp_netif_up, 20000);//forever block,20s is results of tests

			if(ret == pdFAIL) {
				LOG_ERR("getipfailed\r\n");
				ret = uwp_mgmt_disconnect(&uwp_wifi_priv);
				//timeout to get ip,eg.error passwd,cause status error,so process disconnect
				FreeRTOS_NetworkDown();
				if (ret == UWP_OK) {
					LOG_ERR("con_err,dis success\r\n");
				}
				xSemaphoreGive( xWiFiSem );
				return eWiFiFailure;
			}
			xRetVal = eWiFiSuccess;
			xSemaphoreGive( xWiFiSem );

        }
        //vTaskDelay( WAIT_DHCP_DELAY );
        //configPRINT_STRING("wifi connect done.\r\n");
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_connectAP,rt=%d\r\n",xRetVal);
    return xRetVal;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
    /* FIX ME. */
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    int ret = UWP_FAIL;

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
    LOG_ERR("WIFI_Disconnect");

	if(!uwp_wifi_isConnected(WIFI_DEV_STA))
	{
		LOG_ERR("WIFI_Disconnect,already");
		FreeRTOS_NetworkDown();//for dhcp reset
		xSemaphoreGive( xWiFiSem );
		return eWiFiSuccess;//
	}
        ret = uwp_mgmt_disconnect(&uwp_wifi_priv);
        if (ret == UWP_OK)
        {
        	xRetVal = eWiFiSuccess;
        	FreeRTOS_NetworkDown();//notify the upper
        }
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_Disconnect,rt=%d\r\n",xRetVal);
    return xRetVal;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Reset( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t * pxBuffer,
                            uint8_t ucNumNetworks )
{
    /* FIX ME. */
	int ret = UWP_FAIL;
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
    LOG_ERR("WIFI_Scan");
    if (pxBuffer == NULL) {
    	xSemaphoreGive( xWiFiSem );
        return xRetVal;
    }
        struct wifi_drv_scan_params scan_params;

        //scan param
        scan_params.band = 0;
        scan_params.channel = 0;
#ifndef SCANP
		ret = uwp_mgmt_scan(&uwp_wifi_priv, &scan_params);

		if(UWP_OK == ret)
		{
			struct event_scan_result *scan_result = (struct event_scan_result *)UWP_MEM_ALLOC(ucNumNetworks * sizeof(struct event_scan_result));
			if(scan_result == NULL)
			{
				LOG_ERR("mem alloc null");
				xSemaphoreGive( xWiFiSem );
				return xRetVal;
			}
			memset(scan_result, 0, ucNumNetworks * sizeof(struct event_scan_result));
			uwp_mgmt_get_scan_result(scan_result, ucNumNetworks);
           	for (int i = 0; i < ucNumNetworks; i++)
           	{
               	strlcpy(pxBuffer[i].cSSID, (const char *)scan_result[i].ssid, wificonfigMAX_SSID_LEN);
               	memcpy(pxBuffer[i].ucBSSID, scan_result[i].bssid, wificonfigMAX_BSSID_LEN);

               	pxBuffer[i].cRSSI = scan_result[i].rssi;
               	pxBuffer[i].cChannel = scan_result[i].channel;
           	}
           	UWP_MEM_FREE(scan_result);
           	xRetVal = eWiFiSuccess;
		}
#else
		struct event_scan_result *scan_result = (struct event_scan_result *)UWP_MEM_ALLOC(ucNumNetworks * sizeof(struct event_scan_result));
		if(scan_result == NULL)
		{
			LOG_ERR("mem alloc null");
			xSemaphoreGive( xWiFiSem );
			return xRetVal;
		}
		else
		{
			ret = uwp_mgmt_scan(&uwp_wifi_priv, &scan_params, scan_result, ucNumNetworks);
    		if (ret == UWP_OK) {
               	for (int i = 0; i < uwp_scanResult.nresults; i++) {
                   	strlcpy(pxBuffer[i].cSSID, (const char *)scan_result[i].ssid,
                   			wificonfigMAX_SSID_LEN);
                   	memcpy(pxBuffer[i].ucBSSID, scan_result[i].bssid,
                   			wificonfigMAX_BSSID_LEN);

                   	pxBuffer[i].cRSSI = scan_result[i].rssi;
                   	pxBuffer[i].cChannel = scan_result[i].channel;
               	}
               	UWP_MEM_FREE(scan_result);
               	xRetVal = eWiFiSuccess;
            }

		}
#endif
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
	}
    else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_scan,rt=%d\r\n",xRetVal);
    return xRetVal;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t * pxDeviceMode )
{
    /* FIX ME. */
    UWP_WIFI_MODE_T mode;
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {

        if (pxDeviceMode == NULL)
        {
            xSemaphoreGive( xWiFiSem );
            return eWiFiFailure;
        }

        uwp_wifi_get_mode(&mode);
        {
            if (mode == WIFI_MODE_STA)
            {
                *pxDeviceMode = eWiFiModeStation;
            } else if (mode == WIFI_MODE_AP)
            {
                *pxDeviceMode = eWiFiModeAP;
            }
            xRetVal = eWiFiSuccess;
        }
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }
    return xRetVal;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t * pusIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t * pxNetworkProfile,
                                  uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/
TaskHandle_t waiting_task;
static QueueHandle_t Uwp_PingReplyQueue = NULL;
#define WIFI_PING_PKT_SIZE                     256
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus,
                                uint16_t usIdentifier )
{
    /*handle ping reply. */
    switch( eStatus )
    {
        case eSuccess:
            /* A valid ping reply has been received. */
            xQueueSend( Uwp_PingReplyQueue, &usIdentifier, pdMS_TO_TICKS( 10 ) );
            break;
        case eInvalidChecksum:
        case eInvalidData:
            /* A reply was received but it was not valid. */
            break;
    }
}

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    uint32_t ulIPAddress = 0;
    uint16_t usPingSeqNum = pdFAIL;
    uint16_t usPingReplySeqNum = pdFAIL;
    uint32_t ulIndex = 0;
    int ret = pdFAIL;

    if( ( NULL == pucIPAddr ) || ( 0 == usCount ) )
    {
        LOG_ERR("invaild param\r\n");
        return eWiFiFailure;
    }

    if(!uwp_wifi_isConnected(WIFI_DEV_STA))
    {
        LOG_ERR("invaild param\r\n");
        return eWiFiFailure;
    }

    Uwp_PingReplyQueue = xQueueCreate( 5, sizeof(uint16_t) );
    if(Uwp_PingReplyQueue == NULL) {
        LOG_ERR("queue creadted faild\r\n");
        return eWiFiFailure;
    }

    for( ulIndex = 0; ulIndex < usCount; ulIndex++ )
    {
        usPingSeqNum = FreeRTOS_SendPingRequest( (*((uint32_t *)pucIPAddr)), WIFI_PING_PKT_SIZE, pdMS_TO_TICKS( ulIntervalMS ) );
        LOG_DBG("\r\nSending Ping request %d\r\n", usPingSeqNum );

        if(usPingSeqNum == pdFAIL)
        {
            LOG_ERR( "\r\nSending Ping request failed\r\n" );
            ret = eWiFiFailure;
            break;
        }
        else
        {
            /* The ping was sent.  Wait for a reply.  */
            if( xQueueReceive( Uwp_PingReplyQueue,
                               &usPingReplySeqNum,
                               pdMS_TO_TICKS(1000) ) == pdPASS )
            {
                /* A ping reply was received.  Was it a reply to the ping just sent? */
                if( usPingSeqNum == usPingReplySeqNum )
                {
                    /* This was a reply to the request just sent. */
                    printk("Reply[%d] from %d.%d.%d.%d\r\n", ulIndex+1, pucIPAddr[0], pucIPAddr[1], pucIPAddr[2], pucIPAddr[3]);
                }
                else
                    LOG_ERR("ping reply seq num wrong\r\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(ulIntervalMS));
    }
    printk("\r\n");

    /* all ping were sent*/
    ret = eWiFiSuccess;
    vQueueDelete(Uwp_PingReplyQueue);
    return ret;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr )
{

    WIFIReturnCode_t xRetVal = eWiFiFailure;
    uint32_t IPAddr;


    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
        LOG_ERR("WIFI_GetIP");
        if (pucIPAddr == NULL) {
            xSemaphoreGive( xWiFiSem );
            return xRetVal;
        }
        IPAddr = FreeRTOS_GetIPAddress();
        LOG_ERR("ip=%x",IPAddr);
        if (IPAddr != 0UL)
        {
            *( ( uint32_t * ) pucIPAddr ) = IPAddr;
            xRetVal = eWiFiSuccess;
        }
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_GetIP,rt=%d\r\n",xRetVal);
    return xRetVal;
    //return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    int ret = UWP_FAIL;

    LOG_ERR("WIFI_GetMAC");
    if (pucMac == NULL) {
        return xRetVal;
    }

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
        ret = uwp_mgmt_getmac(pucMac);
        if(ret == UWP_OK)
        	xRetVal = eWiFiSuccess;
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_GetMAC,rt=%d\r\n",xRetVal);
    return xRetVal;

}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    /* FIX ME. */
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    uint32_t IPAddr;

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
        if (pcHost == NULL || pucIPAddr == NULL || !CHECK_VALID_HOSTNAME_LEN(strlen(pcHost))) {
            xSemaphoreGive( xWiFiSem );
            return xRetVal;
        }
        IPAddr = FreeRTOS_gethostbyname( pcHost );
        LOG_ERR("hostip=%x",IPAddr);
        if (IPAddr != 0UL)
        {
            *( ( uint32_t * ) pucIPAddr ) = IPAddr;
            xRetVal = eWiFiSuccess;
        }
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_GetHostIP,rt=%d\r\n",xRetVal);
    return xRetVal;
    //return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartAP( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StopAP( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void * pvOptionValue )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t * pxPMModeType,
                                 void * pvOptionValue )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

BaseType_t WIFI_IsConnected(void)
{
	/* FIX ME. */
    BaseType_t xRetVal = pdFALSE;

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
       if (uwp_wifi_isConnected(WIFI_DEV_STA) == true)
        {
            xRetVal = pdTRUE;
        }
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
	else
    {
        xRetVal = eWiFiTimeout;
    }
    LOG_ERR("WIFI_IsConnected,ret=%d\r\n",xRetVal);
    return xRetVal;
}
