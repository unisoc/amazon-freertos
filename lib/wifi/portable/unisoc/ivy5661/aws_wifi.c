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

extern struct wifi_priv uwp_wifi_priv;

static SemaphoreHandle_t xWiFiSem;
static const TickType_t xSemaphoreWaitTicks = pdMS_TO_TICKS( wificonfigMAX_SEMAPHORE_WAIT_TIME_MS );
static WIFIDeviceMode_t prvcurDeviceMode;

#define CHECK_VALID_SSID_LEN(x) \
        ((x) > 0 && (x) <=  wificonfigMAX_SSID_LEN)
#define CHECK_VALID_PASSPHRASE_LEN(x) \
        ((x) > 0 && (x) <= wificonfigMAX_PASSPHRASE_LEN)

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_On( void )
{
    /* FIX ME. */
	int ret = eWiFiFailure;

   configPRINT_STRING("WIFI_On enter\r\n");

   if(sipc_init() != 0) {
       configPRINT_STRING("sipc init failed\r\n");
       return eWiFiFailure;
   }

   if(uwp_init(&uwp_wifi_priv, WIFI_MODE_STA)) {
        configPRINT_STRING("uwp init failed\r\n");
        return eWiFiFailure;
   }
   if(!uwp_wifi_priv.wifi_dev[WIFI_DEV_STA].opened) {
       ret = uwp_mgmt_open(&uwp_wifi_priv);
        if(ret) {
            configPRINT_STRING("WIFI_On,open failed,\r\n");
            return eWiFiFailure;
        }
   }

   static StaticSemaphore_t xSemaphoreBuffer;
   xWiFiSem = xSemaphoreCreateMutexStatic( &( xSemaphoreBuffer ) );
   configPRINT_STRING("WIFI_On done\r\n");
   return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
    /* FIX ME. */
	int ret = eWiFiFailure;

	//ret = uwp_mgmt_close(&uwp_wifi_priv);
	if(!ret)
		return eWiFiSuccess;
	return eWiFiFailure;
}
/*-----------------------------------------------------------*/
#define WAIT_DHCP_DELAY       pdMS_TO_TICKS( 6000 )
WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    /* FIX ME. */
    int ret = eWiFiFailure;
    struct wifi_drv_scan_params scan_params;
    struct wifi_drv_connect_params connect_params;

    if (pxNetworkParams == NULL || pxNetworkParams->pcSSID == NULL
            || (pxNetworkParams->xSecurity != eWiFiSecurityOpen && pxNetworkParams->pcPassword == NULL)) {
        printk("invalid params!\r\n");
        return eWiFiFailure;
    }

    if (!CHECK_VALID_SSID_LEN(pxNetworkParams->ucSSIDLength) ||
        (pxNetworkParams->xSecurity != eWiFiSecurityOpen && !CHECK_VALID_PASSPHRASE_LEN(pxNetworkParams->ucPasswordLength))) {
        printk("invalid params 2!\r\n");
        return eWiFiFailure;
    }

    //fixme: check if wifi on.
    if(!uwp_wifi_priv.wifi_dev[WIFI_DEV_STA].opened) {
        printk("wifi not open!\r\n");
        return eWiFiFailure;
    }
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE ) {

        //scan param
        scan_params.band = 0;
        scan_params.channel = 0;
        ret = uwp_mgmt_scan(&uwp_wifi_priv, &scan_params);
        if(ret) {
        configPRINT_STRING("scan failed\r\n");
            return eWiFiFailure;
        }

        /* SSID is mandatory */
        connect_params.ssid_length = strlen(pxNetworkParams->pcSSID);
        if (!pxNetworkParams->pcSSID || !connect_params.ssid_length)
            return -eWiFiFailure;
        connect_params.ssid = pxNetworkParams->pcSSID;

        /* BSSID is optional */

        /* Passphrase is only valid for WPA/WPA2-PSK */
        connect_params.psk_length = strlen(pxNetworkParams->pcPassword);
        //if (pxNetworkParams->pcPassword && !connect_params.psk_length)
        //	return -eWiFiFailure;

        connect_params.psk = pxNetworkParams->pcPassword;
        ret = uwp_mgmt_connect(&uwp_wifi_priv, &connect_params);
        if(ret) {
            configPRINT_STRING("connect failed\r\n");
            return eWiFiFailure;
        }
        xSemaphoreGive( xWiFiSem );
        //vTaskDelay( WAIT_DHCP_DELAY );
        configPRINT_STRING("wifi connect done.\r\n");
    	return eWiFiSuccess;
    }
    return eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
    /* FIX ME. */
	int ret = eWiFiFailure;

	//ret = uwp_mgmt_disconnect(&uwp_wifi_priv);
	if(!ret)
		return eWiFiSuccess;
	return eWiFiFailure;
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
	int ret = eWiFiFailure;
/*
	struct wifi_scan_params params = {
    		.band = 0,
    		.channel = 0,
    };

	ret = uwp_mgmt_scan(&uwp_wifi_priv, &params);
*/
	if(!ret)
		return eWiFiSuccess;
	return eWiFiFailure;
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
    return eWiFiNotSupported;
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

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr )
{

    WIFIReturnCode_t xRetVal;

    if (pucIPAddr == NULL) {
        return eWiFiFailure;
    }

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
        *( ( uint32_t * ) pucIPAddr ) = FreeRTOS_GetIPAddress();

        xRetVal = eWiFiSuccess;
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

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    WIFIReturnCode_t xRetVal;
    int ret = UWP_FAIL;

    if (pucMac == NULL) {
        return eWiFiFailure;
    }

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
        ret = uwp_mgmt_getmac(pucMac);
        xRetVal = (ret == UWP_OK) ? eWiFiSuccess : eWiFiFailure;
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
    else
    {
        xRetVal = eWiFiTimeout;
    }

    return xRetVal;
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    /* FIX ME. */
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    uint32_t IPAddr;

    if (pcHost == NULL || pucIPAddr == NULL) {
        return xRetVal;
    }

    /* Try to acquire the semaphore. */
    if( xSemaphoreTake( xWiFiSem, xSemaphoreWaitTicks ) == pdTRUE )
    {
        IPAddr = FreeRTOS_gethostbyname( pcHost );
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
        if (uwp_wifi_priv.wifi_dev[WIFI_DEV_STA].connected == true)
        {
            xRetVal = pdTRUE;
        }
        /* Return the semaphore. */
        xSemaphoreGive( xWiFiSem );
    }
    return xRetVal;
}
