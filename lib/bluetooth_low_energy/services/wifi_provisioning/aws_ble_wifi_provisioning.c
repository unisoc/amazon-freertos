/*
 * Amazon FreeRTOS
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
 * @file aws_ble_wifi_provisioning.c
 * @brief BLE Gatt service for WiFi provisioning
 */

#include <string.h>


#ifdef AWS_IOT_CONFIG_FILE
    #include AWS_IOT_CONFIG_FILE
#endif

#include "iot_ble_config.h"
#include "iot_ble_wifi_provisioning.h"
#include "aws_iot_serializer.h"

#define ATTR_HANDLE( svc, ch_idx )        ( ( svc )->pusHandlesBuffer[ch_idx] )
#define ATTR_UUID( svc, ch_idx )          ( ( svc )->pxBLEAttributes[ch_idx].xCharacteristic.xUuid )

#define IS_VALID_SERIALIZER_RET( ret, pxSerializerBuf )                                \
    (  ( ret == AWS_IOT_SERIALIZER_SUCCESS ) ||                                        \
          (  ( !pxSerializerBuf ) && ( ret == AWS_IOT_SERIALIZER_BUFFER_TOO_SMALL ) ) )
/*---------------------------------------------------------------------------------------------------------*/

static IotBleWifiProvService_t xWifiProvService = { 0 };

#define STORAGE_INDEX( priority )    ( xWifiProvService.numNetworks - priority - 1 )
#define NETWORK_INFO_DEFAULT_PARAMS        { .status = eWiFiSuccess, .RSSI = IOT_BLE_WIFI_PROV_INVALID_NETWORK_RSSI, .connected = 0, .savedIdx = IOT_BLE_WIFI_PROV_INVALID_NETWORK_INDEX }
/*---------------------------------------------------------------------------------------------------------*/
/**
 * @brief UUID for Device Information Service.
 *
 * This UUID is used in advertisement for the companion apps to discover and connect to the device.
 */
#define IOT_BLE_WIFI_PROV_LIST_NETWORK_CHAR_UUID_TYPE \
{ \
    .uu.uu128 = IOT_BLE_WIFI_PROV_LIST_NETWORK_CHAR_UUID,\
    .ucType   = eBTuuidType128\
}

#define IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR_UUID_TYPE \
{ \
    .uu.uu128 = IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR_UUID,\
    .ucType   = eBTuuidType128\
}

#define IOT_BLE_WIFI_PROV_EDIT_NETWORK_CHAR_UUID_TYPE \
{ \
    .uu.uu128 = IOT_BLE_WIFI_PROV_EDIT_NETWORK_CHAR_UUID,\
    .ucType   = eBTuuidType128\
}

#define IOT_BLE_WIFI_PROV_DELETE_NETWORK_CHAR_UUID_TYPE \
{ \
    .uu.uu128 = IOT_BLE_WIFI_PROV_DELETE_NETWORK_CHAR_UUID,\
    .ucType   = eBTuuidType128\
}

#define IOT_BLE_WIFI_PROV_CLIENT_CHAR_CFG_UUID_TYPE \
{ \
    .uu.uu16 = IOT_BLE_WIFI_PROV_CLIENT_CHAR_CFG_UUID,\
    .ucType   = eBTuuidType16\
}


#define IOT_BLE_WIFI_PROV_SVC_UUID_TYPE \
{ \
    .uu.uu128 = IOT_BLE_WIFI_PROV_SVC_UUID,\
    .ucType   = eBTuuidType128\
}

static uint16_t usHandlesBuffer[IOT_BLE_WIFI_PROV_NB_ATTRIBUTES];

static const BTAttribute_t pxAttributeTable[] = {
     {
         .xServiceUUID =  IOT_BLE_WIFI_PROV_SVC_UUID_TYPE
     },
    {
         .xAttributeType = eBTDbCharacteristic,
         .xCharacteristic =
         {
              .xUuid = IOT_BLE_WIFI_PROV_LIST_NETWORK_CHAR_UUID_TYPE,
              .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  ),
              .xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify )
          }
     },
     {
         .xAttributeType = eBTDbDescriptor,
         .xCharacteristicDescr =
         {
             .xUuid = IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR_UUID_TYPE,
             .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  )
          }
     },
	{
		 .xAttributeType = eBTDbCharacteristic,
		 .xCharacteristic =
		 {
			  .xUuid = IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR_UUID_TYPE,
			  .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  ),
			  .xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify )
		  }
	 },
	 {
		 .xAttributeType = eBTDbDescriptor,
		 .xCharacteristicDescr =
		 {
			 .xUuid = IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR_UUID_TYPE,
			 .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  )
		  }
	 },
	{
		 .xAttributeType = eBTDbCharacteristic,
		 .xCharacteristic =
		 {
			  .xUuid = IOT_BLE_WIFI_PROV_EDIT_NETWORK_CHAR_UUID_TYPE,
			  .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  ),
			  .xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify )
		  }
	 },
	 {
		 .xAttributeType = eBTDbDescriptor,
		 .xCharacteristicDescr =
		 {
			 .xUuid = IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR_UUID_TYPE,
			 .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  )
		  }
	 },
	{
		 .xAttributeType = eBTDbCharacteristic,
		 .xCharacteristic =
		 {
			  .xUuid = IOT_BLE_WIFI_PROV_DELETE_NETWORK_CHAR_UUID_TYPE,
			  .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  ),
			  .xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify )
		  }
	 },
	 {
		 .xAttributeType = eBTDbDescriptor,
		 .xCharacteristicDescr =
		 {
			 .xUuid = IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR_UUID_TYPE,
			 .xPermissions = ( IOT_BLE_CHAR_READ_PERM | IOT_BLE_CHAR_WRITE_PERM  )
		  }
	 }
};

static const BTService_t xWIFIProvisionningService =
{
  .xNumberOfAttributes = IOT_BLE_WIFI_PROV_NB_ATTRIBUTES,
  .ucInstId = 0,
  .xType = eBTServiceTypePrimary,
  .pusHandlesBuffer = usHandlesBuffer,
  .pxBLEAttributes = (BTAttribute_t *)pxAttributeTable
};
/*
 * @brief Callback registered for BLE write and read events received for each characteristic.
 */
static void vCharacteristicCallback( IotBleAttributeEvent_t * pEventParam );

/*
 * @brief Callback registered for client characteristic configuration descriptors.
 */
static void vClientCharCfgDescrCallback( IotBleAttributeEvent_t * pEventParam );

static BaseType_t prxDeserializeListNetworkRequest( uint8_t * pucData, size_t xLength, IotBleListNetworkRequest_t* pListNetworkRequest );

/*
 * @brief Parses List Network request params and creates task to list networks.
 */
static BaseType_t prxHandleListNetworkRequest( uint8_t * pucData,
                                               size_t xLength );


static BaseType_t prxDeserializeAddNetworkRequest( uint8_t * pucData, size_t xLength, IotBleAddNetworkRequest_t* pAddNetworkRequest );
/*
 * @brief Parses Save Network request params and creates task to save the new network.
 */
static BaseType_t prxHandleSaveNetworkRequest( uint8_t * pucData,
                                               size_t xLength );


static BaseType_t prxDeserializeEditNetworkRequest( uint8_t * pucData, size_t xLength, IotBleEditNetworkRequest_t* pEditNetworkRequest );

/*
 * @brief Parses Edit Network request params and creates task to edit network priority.
 */
static BaseType_t prxHandleEditNetworkRequest( uint8_t * pucData,
                                               size_t xLength );

static BaseType_t prxDeserializeDeleteNetworkRequest( uint8_t * pucData, size_t xLength, IotBleDeleteNetworkRequest_t* pDeleteNetworkRequest );

/*
 * @brief Parses Delete Network request params and creates task to delete a WiFi networ.
 */
static BaseType_t prxHandleDeleteNetworkRequest( uint8_t * pucData,
                                                 size_t xLength );

/*
 * @brief Gets the GATT characteristic for a given attribute handle.
 */
static IotBleWifiProvAttributes_t prxGetCharFromHandle( uint16_t xHandle );


WIFIReturnCode_t prvAppendNetwork( WIFINetworkProfile_t * pxProfile );

WIFIReturnCode_t prvInsertNetwork( uint16_t usIndex,
                                   WIFINetworkProfile_t * pxProfile );

WIFIReturnCode_t prvPopNetwork( uint16_t usIndex,
                                WIFINetworkProfile_t * pxProfile );

WIFIReturnCode_t prvMoveNetwork( uint16_t usCurrentIndex,
                                 uint16_t usNewIndex );

WIFIReturnCode_t prvGetSavedNetwork( uint16_t usIndex,
                                     WIFINetworkProfile_t * pxProfile );

WIFIReturnCode_t prvConnectNetwork( WIFINetworkProfile_t * pxProfile );

WIFIReturnCode_t prvConnectSavedNetwork( uint16_t usIndex );

WIFIReturnCode_t prvAddNewNetwork( WIFINetworkProfile_t * pxProfile );


static AwsIotSerializerError_t prxSerializeNetwork( IotBleWifiNetworkInfo_t *pNetworkInfo, uint8_t *pucBuffer, size_t *pxLength );

static void prvSendSavedNetwork( WIFINetworkProfile_t *pxSavedNetwork, uint16_t idx );

static void prvSendScanNetwork( WIFIScanResult_t *pScanNetwork );

static AwsIotSerializerError_t prxSerializeStatusResponse( WIFIReturnCode_t status, uint8_t* pucBuffer, size_t* pxLength );

/*
 * @brief  The task lists the saved network configurations in flash and also scans nearby networks.
 * It sends the profile information for each saved and scanned networks one at a time to the GATT client.
 * Maximum number of networks to scan is set in the List network request.
 */
static void prvListNetworkTask( void * pvParams );

/*
 * @brief  The task is used to save a new WiFi configuration.
 * It first connects to the network and if successful,saves the network onto flash with the highest priority.
 */
static void prvAddNetworkTask( void * pvParams );

/*
 * @brief  The task is used to reorder priorities of network profiles stored in flash.
 *  If the priority of existing connected network changes then it initiates a reconnection.
 */
void prvEditNetworkTask( void * pvParams );

/*
 * @brief  The task is used to delete a network configuration from the flash.
 * If the network is connected, it disconnects from the network and initiates a reconnection.
 */
void prvDeleteNetworkTask( void * pvParams );

/*
 * @brief Gets the number of saved networks from flash.
 */
static uint16_t prvGetNumSavedNetworks( void );

/*
 * @brief Sends a status response for the request.
 */
void prvSendStatusResponse( IotBleWifiProvAttributes_t xChar, WIFIReturnCode_t status );


void prvSendResponse( IotBleWifiProvAttributes_t xCharacteristic, uint8_t* pucData, size_t xLen );


/*-----------------------------------------------------------*/


static const IotBleAttributeEventCallback_t pxCallBackArray[IOT_BLE_WIFI_PROV_NB_ATTRIBUTES] =
{
  NULL,
  vCharacteristicCallback,
  vClientCharCfgDescrCallback,
  vCharacteristicCallback,
  vClientCharCfgDescrCallback,
  vCharacteristicCallback,
  vClientCharCfgDescrCallback,
  vCharacteristicCallback,
  vClientCharCfgDescrCallback
};

/*-----------------------------------------------------------*/

static IotBleWifiProvAttributes_t prxGetCharFromHandle( uint16_t usHandle )
{
    uint8_t ucCharId;

    for( ucCharId = 0; ucCharId < IOT_BLE_WIFI_PROV_NB_ATTRIBUTES; ucCharId++ )
    {
        if( usHandlesBuffer[ucCharId] == usHandle )
        {
            break;
        }
    }

    return ( IotBleWifiProvAttributes_t ) ucCharId;
}

/*-----------------------------------------------------------*/

BaseType_t IotBleWifiProv_Start( void )
{
    BaseType_t xRet = pdFALSE;

    if( xWifiProvService.init == pdTRUE )
    {
    	xWifiProvService.numNetworks = prvGetNumSavedNetworks();
    	xRet = pdTRUE;
    }

    return xRet;
}

void vCharacteristicCallback( IotBleAttributeEvent_t * pEventParam )
{
    IotBleWriteEventParams_t * pxWriteParam;
    IotBleReadEventParams_t * pxReadParam;
    BaseType_t xResult = pdFAIL;

    IotBleAttributeData_t xAttrData = { 0 };
    IotBleEventResponse_t xResp =
    {
        .eventStatus    = eBTStatusFail,
        .rspErrorStatus = eBTRspErrorNone
    };
    IotBleWifiProvAttributes_t xChar;

    xResp.pAttrData = &xAttrData;

    if( pEventParam->xEventType == eBLEWrite )
    {
        pxWriteParam = pEventParam->pParamWrite;
        xWifiProvService.BLEConnId = pxWriteParam->connId;

        xChar = prxGetCharFromHandle( pxWriteParam->attrHandle );

        switch( xChar )
        {
            case IOT_BLE_WIFI_PROV_LIST_NETWORK_CHAR:
                xResult = prxHandleListNetworkRequest( pxWriteParam->pValue, pxWriteParam->length );
                break;

            case IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR:
                xResult = prxHandleSaveNetworkRequest( pxWriteParam->pValue, pxWriteParam->length );
                break;

            case  IOT_BLE_WIFI_PROV_EDIT_NETWORK_CHAR:
                xResult = prxHandleEditNetworkRequest( pxWriteParam->pValue, pxWriteParam->length );
                break;

            case IOT_BLE_WIFI_PROV_DELETE_NETWORK_CHAR:
                xResult = prxHandleDeleteNetworkRequest( pxWriteParam->pValue, pxWriteParam->length );
                break;

            case IOT_BLE_WIFI_PROV_NB_ATTRIBUTES:
            default:
                xResult = pdFAIL;
                break;
        }

        if( xResult == pdPASS )
        {
            xResp.eventStatus = eBTStatusSuccess;
        }

        xResp.pAttrData->handle = pxWriteParam->attrHandle;
        xResp.pAttrData->pData = pxWriteParam->pValue;
        xResp.pAttrData->size = pxWriteParam->length;
        xResp.attrDataOffset = pxWriteParam->offset;
        IotBle_SendResponse( &xResp, pxWriteParam->connId, pxWriteParam->transId );
    }
    else if( pEventParam->xEventType == eBLERead )
    {
        pxReadParam = pEventParam->pParamRead;
        xResp.pAttrData->handle = pxReadParam->attrHandle;
        xResp.pAttrData->pData = NULL;
        xResp.pAttrData->size = 0;
        xResp.attrDataOffset = 0;
        xResp.eventStatus = eBTStatusSuccess;
        IotBle_SendResponse( &xResp, pxReadParam->connId, pxReadParam->transId );
    }
}

/*-----------------------------------------------------------*/

static void vClientCharCfgDescrCallback( IotBleAttributeEvent_t * pEventParam )
{
    IotBleWriteEventParams_t * pxWriteParam;
    IotBleReadEventParams_t * pxReadParam;

    IotBleAttributeData_t xAttrData = { 0 };
    IotBleEventResponse_t xResp =
    {
        .eventStatus    = eBTStatusSuccess,
        .rspErrorStatus = eBTRspErrorNone
    };

    xResp.pAttrData = &xAttrData;

    if( pEventParam->xEventType == eBLEWrite )
    {
        pxWriteParam = pEventParam->pParamWrite;

        if( pxWriteParam->length == 2 )
        {
            xWifiProvService.notifyClientEnabled = ( pxWriteParam->pValue[ 1 ] << 8 ) | pxWriteParam->pValue[ 0 ];
            xWifiProvService.BLEConnId = pxWriteParam->connId;
            xResp.pAttrData->handle = pxWriteParam->attrHandle;
            xResp.pAttrData->pData = pxWriteParam->pValue;
            xResp.pAttrData->size = pxWriteParam->length;
            xResp.attrDataOffset = pxWriteParam->offset;
        }

        IotBle_SendResponse( &xResp, pxWriteParam->connId, pxWriteParam->transId );
    }
    else if( pEventParam->xEventType == eBLERead )
    {
        pxReadParam = pEventParam->pParamRead;
        xResp.pAttrData->handle = pxReadParam->attrHandle;
        xResp.pAttrData->pData = ( uint8_t * ) &xWifiProvService.notifyClientEnabled;
        xResp.pAttrData->size = 2;
        xResp.attrDataOffset = 0;
        xResp.eventStatus = eBTStatusSuccess;
        IotBle_SendResponse( &xResp, pxReadParam->connId, pxReadParam->transId );
    }
}

/*-----------------------------------------------------------*/

static uint16_t prvGetNumSavedNetworks( void )
{
    uint16_t idx;
    WIFIReturnCode_t xWifiRet;
    WIFINetworkProfile_t xProfile;

    for( idx = 0; idx < IOT_BLE_WIFI_PROV_MAX_SAVED_NETWORKS; idx++ )
    {
        xWifiRet = WIFI_NetworkGet( &xProfile, idx );

        if( xWifiRet != eWiFiSuccess )
        {
            break;
        }
    }

    return idx;
}

static BaseType_t prxDeserializeListNetworkRequest( uint8_t * pucData, size_t xLength, IotBleListNetworkRequest_t* pListNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = IOT_BLE_MESG_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

    if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
            ( xDecoderObj.type != AWS_IOT_SERIALIZER_CONTAINER_MAP ) )
    {
        configPRINTF(( "Failed to initialize the decoder, error = %d, object type = %d\n", xRet, xDecoderObj.type ));
        xResult = pdFALSE;
    }

    if( xResult == pdTRUE )
    {
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_MAX_NETWORKS_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get max Networks parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt <= 0 )
                    || ( xValue.value.signedInt  > IOT_BLE_MAX_NETWORK ) )
            {
                
                    configPRINTF(( "WARN: Max Networks (%d) exceeds configured Max networks (%d). Caping max networks to %d\n",
                        xValue.value.signedInt,
                        IOT_BLE_MAX_NETWORK,
                        IOT_BLE_MAX_NETWORK ));
                pListNetworkRequest->maxNetworks = IOT_BLE_MAX_NETWORK;
            }
            else
            {
                pListNetworkRequest->maxNetworks = xValue.value.signedInt;
            }
        }
    }

    if( xResult == pdTRUE )
    {
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_SCAN_TIMEOUT_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get timeout parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {

            pListNetworkRequest->timeoutMs = xValue.value.signedInt;
        }
    }

    IOT_BLE_MESG_DECODER.destroy( &xDecoderObj );
    return xResult;
}


/*-----------------------------------------------------------*/

static BaseType_t prxHandleListNetworkRequest( uint8_t * pucData,
                                               size_t xLength )
{
    BaseType_t status = pdFALSE;
    IotBleListNetworkRequest_t * pxParams = pvPortMalloc( sizeof( IotBleListNetworkRequest_t ));
    if( pxParams != NULL )
    {
        status = prxDeserializeListNetworkRequest( pucData, xLength, pxParams );
        if( status == pdTRUE )
        {
            status = xTaskCreate(
                    prvListNetworkTask,
                    "WifiProvListNetwork",
                    configMINIMAL_STACK_SIZE * 6,
                    pxParams,
					IOT_BLE_WIFI_PROV_TASK_PRIORITY,
                    NULL );
        }
        else
        {
            vPortFree( pxParams );
        }
    }
    else
    {
        configPRINTF(( "Failed to allocate memory for List Network Request\n" ));
    }

    return status;
}

static BaseType_t prxDeserializeAddNetworkRequest( uint8_t * pucData, size_t xLength, IotBleAddNetworkRequest_t* pAddNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = IOT_BLE_MESG_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

    if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
            ( xDecoderObj.type != AWS_IOT_SERIALIZER_CONTAINER_MAP ) )
    {
        configPRINTF(( "Failed to initialize the decoder, error = %d, object type = %d\n", xRet, xDecoderObj.type ));
        xResult = pdFALSE;
    }

    if( xResult == pdTRUE )
    {
        xValue.value.pString = NULL;
        xValue.value.stringLength = 0;
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_SSID_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_TEXT_STRING ) )
        {
            configPRINTF(( "Failed to get SSID parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( xValue.value.stringLength >= wificonfigMAX_SSID_LEN )
            {
                configPRINTF(( "SSID, %.*s, exceeds maximum length %d\n",
                        xValue.value.stringLength,
                        ( const char * )xValue.value.pString,
                        wificonfigMAX_SSID_LEN ));
                xResult = pdFALSE;
            }
            else
            {
                strncpy( pAddNetworkRequest->network.cSSID, ( const char * ) xValue.value.pString, xValue.value.stringLength );
                pAddNetworkRequest->network.cSSID[ xValue.value.stringLength ] = '\0';
                pAddNetworkRequest->network.ucSSIDLength = ( xValue.value.stringLength + 1 );
            }
        }
    }

    if( xResult == pdTRUE )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_BYTE_STRING;
        xValue.value.pString = NULL;
        xValue.value.stringLength = 0;
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_BSSID_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_BYTE_STRING ) )
        {
            configPRINTF(( "Failed to get BSSID parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( xValue.value.stringLength != wificonfigMAX_BSSID_LEN )
            {
                configPRINTF(( "Parameter BSSID length (%d) does not match BSSID length %d\n",
                        xValue.value.stringLength,
                        wificonfigMAX_BSSID_LEN ));
                xResult = pdFALSE;
            }
            else
            {
                memcpy( pAddNetworkRequest->network.ucBSSID, xValue.value.pString, wificonfigMAX_BSSID_LEN );
            }
        }
    }

    if( xResult == pdTRUE )
    {
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_KEY_MGMT_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get WIFI xSecurity parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            pAddNetworkRequest->network.xSecurity = xValue.value.signedInt;
        }
    }

    if( xResult == pdTRUE )
    {
        xValue.value.pString = NULL;
          xValue.value.stringLength = 0;
          xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_PSK_KEY, &xValue );
          if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                  ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_TEXT_STRING ) )
          {
              configPRINTF(( "Failed to get password parameter, error = %d, value type = %d\n", xRet, xValue.type ));
              xResult = pdFALSE;
          }
          else
          {
              if( xValue.value.stringLength >= wificonfigMAX_PASSPHRASE_LEN )
              {
                  configPRINTF(( "SSID, %.*s, exceeds maximum length %d\n",
                          xValue.value.stringLength,
                          ( const char * )xValue.value.pString,
                          wificonfigMAX_SSID_LEN ));
                  xResult = pdFALSE;
              }
              else
              {
                  strncpy( pAddNetworkRequest->network.cPassword, ( const char * ) xValue.value.pString, xValue.value.stringLength );
                  pAddNetworkRequest->network.cPassword[ xValue.value.stringLength ] = '\0';
                  pAddNetworkRequest->network.ucPasswordLength = ( uint16_t )( xValue.value.stringLength + 1 );
              }
          }
    }

    if( xResult == pdTRUE )
    {
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_INDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= IOT_BLE_WIFI_PROV_INVALID_NETWORK_INDEX )
                    && ( xValue.value.signedInt < IOT_BLE_WIFI_PROV_MAX_SAVED_NETWORKS  ) )
            {
                pAddNetworkRequest->savedIdx = xValue.value.signedInt;
            }
            else
            {
                configPRINTF(( "Network index parameter ( %d ) is out of range.\n", xValue.value.signedInt ));
                xResult = pdFALSE;
            }
        }
    }

    IOT_BLE_MESG_DECODER.destroy( &xDecoderObj );

    return xResult;
}

/*-----------------------------------------------------------*/

static BaseType_t prxHandleSaveNetworkRequest( uint8_t * pucData,
                                               size_t xLength )
{
    BaseType_t status = pdFALSE;
    IotBleAddNetworkRequest_t *pxParams = pvPortMalloc( sizeof( IotBleAddNetworkRequest_t ));

    if( pxParams != NULL )
    {
        status = prxDeserializeAddNetworkRequest( pucData, xLength, pxParams );

        if( status == pdTRUE )
        {
            status = xTaskCreate(
                    prvAddNetworkTask,
                    "WifiProvAddNetwork",
                    configMINIMAL_STACK_SIZE * 4,
                    pxParams,
                    IOT_BLE_WIFI_PROV_MODIFY_NETWORK_TASK_PRIORITY,
                    NULL );
        }
        else
        {
            vPortFree( pxParams );
        }
    }
    else
    {
        configPRINTF(( "Failed to allocate memory for save network request\n " ));
    }

    return status;
}

static BaseType_t prxDeserializeEditNetworkRequest( uint8_t * pucData, size_t xLength, IotBleEditNetworkRequest_t* pEditNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = IOT_BLE_MESG_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

    if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
            ( xDecoderObj.type != AWS_IOT_SERIALIZER_CONTAINER_MAP ) )
    {
        configPRINTF(( "Failed to initialize decoder, error = %d, object type = %d\n", xRet, xDecoderObj.type ));
        xResult = pdFALSE;
    }

    if( xResult == pdTRUE )
    {
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_INDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= 0 )
                    && ( xValue.value.signedInt < IOT_BLE_WIFI_PROV_MAX_SAVED_NETWORKS  ) )
            {
                pEditNetworkRequest->curIdx = xValue.value.signedInt;
            }
            else
            {
                configPRINTF(( "Network index parameter ( %d ) is out of range.\n", xValue.value.signedInt ));
                xResult = pdFALSE;
            }
        }
    }

    if( xResult == pdTRUE )
    {
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_NEWINDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get new network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= 0 )
                    && ( xValue.value.signedInt < IOT_BLE_WIFI_PROV_MAX_SAVED_NETWORKS  ) )
            {
                pEditNetworkRequest->newIdx = xValue.value.signedInt;
            }
            else
            {
                configPRINTF(( "New Network index parameter ( %d ) is out of range.\n", xValue.value.signedInt ));
                xResult = pdFALSE;
            }
        }
    }

    IOT_BLE_MESG_DECODER.destroy( &xDecoderObj );

    return xResult;
}

/*-----------------------------------------------------------*/

static BaseType_t prxHandleEditNetworkRequest( uint8_t * pucData, size_t xLength )
{
    BaseType_t status = pdFALSE;
    IotBleEditNetworkRequest_t *pxParams = pvPortMalloc( sizeof( IotBleEditNetworkRequest_t ));

    if( pxParams != NULL )
    {
        status = prxDeserializeEditNetworkRequest( pucData, xLength, pxParams );

        if( status == pdTRUE )
        {
            status = xTaskCreate(
                    prvEditNetworkTask,
                    "WifiProvEditNetwork",
                    configMINIMAL_STACK_SIZE * 4,
                    pxParams,
                    IOT_BLE_WIFI_PROV_MODIFY_NETWORK_TASK_PRIORITY,
                    NULL );
        }
        else
        {
            vPortFree( pxParams );
        }
    }
    else
    {
        configPRINTF(( "Failed to allocate memory for edit network request\n " ));
    }

    return status;
}

static BaseType_t prxDeserializeDeleteNetworkRequest( uint8_t * pucData, size_t xLength, IotBleDeleteNetworkRequest_t* pDeleteNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = IOT_BLE_MESG_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

    if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
            ( xDecoderObj.type != AWS_IOT_SERIALIZER_CONTAINER_MAP ) )
    {
        configPRINTF(( "Failed to initialize decoder, error = %d, object type = %d\n", xRet, xDecoderObj.type ));
        xResult = pdFALSE;
    }

    if( xResult == pdTRUE )
    {
        xRet = IOT_BLE_MESG_DECODER.find( &xDecoderObj, IOT_BLE_WIFI_PROV_INDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= 0 )
                    && ( xValue.value.signedInt < IOT_BLE_WIFI_PROV_MAX_SAVED_NETWORKS  ) )
            {
                pDeleteNetworkRequest->idx = ( int16_t ) xValue.value.signedInt;
            }
            else
            {
                configPRINTF(( "Network index parameter ( %d ) is out of range.\n", xValue.value.signedInt ));
                xResult = pdFALSE;
            }
        }
    }

    IOT_BLE_MESG_DECODER.destroy( &xDecoderObj );

    return xResult;
}

/*-----------------------------------------------------------*/

static BaseType_t prxHandleDeleteNetworkRequest( uint8_t * pucData,
                                                 size_t xLength )
{
    BaseType_t status = pdFALSE;
    IotBleDeleteNetworkRequest_t *pxParams = pvPortMalloc( sizeof( IotBleDeleteNetworkRequest_t ));

    if( pxParams != NULL )
    {
        status = prxDeserializeDeleteNetworkRequest( pucData, xLength, pxParams );

        if( status == pdTRUE )
        {
            status = xTaskCreate(
                    prvDeleteNetworkTask,
                    "WifiProvDeleteNetwork",
                    configMINIMAL_STACK_SIZE * 4,
                    pxParams,
                    IOT_BLE_WIFI_PROV_MODIFY_NETWORK_TASK_PRIORITY,
                    NULL );
        }
        else
        {
            vPortFree( pxParams );
        }
    }
    else
    {
        configPRINTF(( "Failed to allocate memory for delete network request\n " ));
    }

    return status;
}


AwsIotSerializerError_t prxSerializeNetwork( IotBleWifiNetworkInfo_t *pNetworkInfo, uint8_t *pucBuffer, size_t *pxLength )
{
    AwsIotSerializerEncoderObject_t xContainer = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_STREAM;
    AwsIotSerializerEncoderObject_t xNetworkMap = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;
    AwsIotSerializerScalarData_t xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    size_t xLength = *pxLength;

    xRet = IOT_BLE_MESG_ENCODER.init( &xContainer, pucBuffer, xLength );
    if( xRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        xRet = IOT_BLE_MESG_ENCODER.openContainer( &xContainer, &xNetworkMap, IOT_BLE_WIFI_PROV_NUM_NETWORK_INFO_MESG_PARAMS );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pNetworkInfo->status;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_STATUS_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_TEXT_STRING;
        xValue.value.pString = ( uint8_t * ) pNetworkInfo->pSSID;
        xValue.value.stringLength = pNetworkInfo->SSIDLength;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_SSID_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_BYTE_STRING;
        xValue.value.pString = ( uint8_t * ) pNetworkInfo->pBSSID;
        xValue.value.stringLength = pNetworkInfo->BSSIDLength;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_BSSID_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pNetworkInfo->security;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_KEY_MGMT_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_BOOL;
        xValue.value.booleanValue = pNetworkInfo->hidden;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_HIDDEN_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pNetworkInfo->RSSI;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_RSSI_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pNetworkInfo->connected;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_CONNECTED_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pNetworkInfo->savedIdx;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xNetworkMap, IOT_BLE_WIFI_PROV_INDEX_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xRet = IOT_BLE_MESG_ENCODER.closeContainer( &xContainer, &xNetworkMap );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        if( pucBuffer == NULL )
        {
            *pxLength = IOT_BLE_MESG_ENCODER.getExtraBufferSizeNeeded( &xContainer );
        }
        else
        {
            *pxLength = IOT_BLE_MESG_ENCODER.getEncodedSize( &xContainer, pucBuffer );
        }

        IOT_BLE_MESG_ENCODER.destroy( &xContainer );
        xRet = AWS_IOT_SERIALIZER_SUCCESS;
    }

    return xRet;
}

static AwsIotSerializerError_t prxSerializeStatusResponse( WIFIReturnCode_t status, uint8_t* pucBuffer, size_t* pxLength )
{
    AwsIotSerializerEncoderObject_t xContainer = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_STREAM;
    AwsIotSerializerEncoderObject_t xResponseMap = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;
    AwsIotSerializerScalarData_t xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    size_t xLength = *pxLength;

    xRet = IOT_BLE_MESG_ENCODER.init( &xContainer, pucBuffer, xLength );
    if( xRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        xRet = IOT_BLE_MESG_ENCODER.openContainer( &xContainer, &xResponseMap, IOT_BLE_WIFI_PROV_NUM_STATUS_MESG_PARAMS );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = status;
        xRet = IOT_BLE_MESG_ENCODER.appendKeyValue( &xResponseMap, IOT_BLE_WIFI_PROV_STATUS_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xRet = IOT_BLE_MESG_ENCODER.closeContainer( &xContainer, &xResponseMap );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        if( pucBuffer == NULL )
        {
            *pxLength = IOT_BLE_MESG_ENCODER.getExtraBufferSizeNeeded( &xContainer );
        }
        else
        {
            *pxLength = IOT_BLE_MESG_ENCODER.getEncodedSize( &xContainer, pucBuffer );
        }

        IOT_BLE_MESG_ENCODER.destroy( &xContainer );

        xRet = AWS_IOT_SERIALIZER_SUCCESS;
    }

    return xRet;
}


/*-----------------------------------------------------------*/

void prvSendStatusResponse( IotBleWifiProvAttributes_t xCharacteristic,
                            WIFIReturnCode_t status )
{

    uint8_t *pucBuffer = NULL;
    size_t xMesgLen;
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    xRet = prxSerializeStatusResponse( status, NULL, &xMesgLen );
    if( xRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        pucBuffer = pvPortMalloc( xMesgLen );
        if( pucBuffer != NULL )
        {
            xRet = prxSerializeStatusResponse( status, pucBuffer, &xMesgLen );
        }
        else
        {
            xRet = AWS_IOT_SERIALIZER_OUT_OF_MEMORY;
        }
    }

    if( xRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        prvSendResponse( xCharacteristic, pucBuffer, xMesgLen );
    }
    else
    {
        configPRINTF(( "Failed to serialize status response, error = %d\n", xRet ));
    }

    if( pucBuffer != NULL )
    {
        vPortFree( pucBuffer );
    }
}

void prvSendResponse( IotBleWifiProvAttributes_t xCharacteristic, uint8_t* pucData, size_t xLen )
{
	IotBleAttributeData_t xAttrData = { 0 };
	IotBleEventResponse_t xResp = { 0 };

	xAttrData.handle = ATTR_HANDLE( xWifiProvService.pGattService, xCharacteristic );
	xAttrData.uuid = ATTR_UUID( xWifiProvService.pGattService, xCharacteristic );
	xResp.attrDataOffset = 0;
	xResp.pAttrData = &xAttrData;
	xResp.rspErrorStatus = eBTRspErrorNone;

	xAttrData.pData = pucData;
	xAttrData.size = xLen;

	( void ) IotBle_SendIndication( &xResp, xWifiProvService.BLEConnId, false );
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t prvAppendNetwork( WIFINetworkProfile_t * pxProfile )
{
    WIFIReturnCode_t xRet;
    uint16_t idx;

    xRet = WIFI_NetworkAdd( pxProfile, &idx );

    if( xRet == eWiFiSuccess )
    {
        xWifiProvService.numNetworks++;
        xWifiProvService.connectedIdx++;
    }

    return xRet;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t prvPopNetwork( uint16_t usIndex,
                                WIFINetworkProfile_t * pxProfile )
{
    WIFIReturnCode_t xRet = eWiFiSuccess;

    if( pxProfile != NULL )
    {
        xRet = WIFI_NetworkGet( pxProfile, STORAGE_INDEX( usIndex ) );
    }

    if( xRet == eWiFiSuccess )
    {
        xRet = WIFI_NetworkDelete( STORAGE_INDEX( usIndex ) );
    }

    if( xRet == eWiFiSuccess )
    {
        xWifiProvService.numNetworks--;

        /* Shift the priority for connected network */
        if( usIndex < xWifiProvService.connectedIdx )
        {
            xWifiProvService.connectedIdx--;
        }
        else if( usIndex == xWifiProvService.connectedIdx )
        {
            xWifiProvService.connectedIdx = IOT_BLE_WIFI_PROV_INVALID_NETWORK_INDEX;
        }
    }

    return xRet;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t prvMoveNetwork( uint16_t usCurrentIndex,
                                 uint16_t usNewIndex )
{
    WIFIReturnCode_t xRet = eWiFiSuccess;
    WIFINetworkProfile_t xProfile;

    if( usCurrentIndex != usNewIndex )
    {
        xRet = prvPopNetwork( usCurrentIndex, &xProfile );

        if( xRet == eWiFiSuccess )
        {
            xRet = prvInsertNetwork( usNewIndex, &xProfile );
        }
    }

    return xRet;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t prvGetSavedNetwork( uint16_t usIndex,
                                     WIFINetworkProfile_t * pxProfile )
{
	return WIFI_NetworkGet( pxProfile, STORAGE_INDEX( usIndex ) );
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t prvConnectNetwork( WIFINetworkProfile_t * pxProfile )
{
    WIFIReturnCode_t xRet = eWiFiFailure;
    WIFINetworkParams_t xNetworkParams = { 0 };

    xNetworkParams.pcSSID = pxProfile->cSSID;
    xNetworkParams.ucSSIDLength = pxProfile->ucSSIDLength;
    xNetworkParams.pcPassword = pxProfile->cPassword;
    xNetworkParams.ucPasswordLength = pxProfile->ucPasswordLength;
    xNetworkParams.xSecurity = pxProfile->xSecurity;
    xRet = WIFI_ConnectAP( &xNetworkParams );
    return xRet;
}

WIFIReturnCode_t prvAddNewNetwork( WIFINetworkProfile_t * pxProfile )
{
    WIFIReturnCode_t xRet;

    xRet = prvConnectNetwork( pxProfile );
    if( xRet == eWiFiSuccess )
    {
    	xRet = prvAppendNetwork( pxProfile );
    	if( xRet == eWiFiSuccess )
    	{
    		xWifiProvService.connectedIdx = 0;
    	}
    }
    return xRet;
}


WIFIReturnCode_t prvConnectSavedNetwork( uint16_t usIndex )
{
    WIFIReturnCode_t xRet;
    WIFINetworkProfile_t xProfile;

    xRet = prvGetSavedNetwork( usIndex, &xProfile );
    if( xRet == eWiFiSuccess )
    {
    	xRet = prvConnectNetwork( &xProfile );
    	if( xRet == eWiFiSuccess )
    	{
    		xWifiProvService.connectedIdx = usIndex;
    	}
    }
    return xRet;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t prvInsertNetwork( uint16_t usIndex,
                                   WIFINetworkProfile_t * pxProfile )
{
    WIFIReturnCode_t xRet;
    WIFINetworkProfile_t xProfile;
    uint16_t usNumElementsToShift, x;

    /* All higher priority elements needs to be shifted */
    usNumElementsToShift = usIndex;

    xRet = prvAppendNetwork( pxProfile );

    if( xRet == eWiFiSuccess )
    {
        for( x = 0; x < usNumElementsToShift; x++ )
        {
            xRet = prvPopNetwork( usIndex, &xProfile );
            configASSERT( xRet == eWiFiSuccess );
            xRet = prvAppendNetwork( &xProfile );
            configASSERT( xRet == eWiFiSuccess );
        }
    }

    return xRet;
}

static void prvSendSavedNetwork( WIFINetworkProfile_t *pxSavedNetwork, uint16_t idx )
{
    IotBleWifiNetworkInfo_t xNetworkInfo = NETWORK_INFO_DEFAULT_PARAMS;
    uint8_t *pucMessage = NULL;
    size_t xMessageLen = 0;
    AwsIotSerializerError_t xSerializerRet;

    xNetworkInfo.pSSID = pxSavedNetwork->cSSID;
    xNetworkInfo.SSIDLength = pxSavedNetwork->ucSSIDLength;
    xNetworkInfo.pBSSID = pxSavedNetwork->ucBSSID;
    xNetworkInfo.BSSIDLength = wificonfigMAX_BSSID_LEN;
    xNetworkInfo.connected = ( xWifiProvService.connectedIdx == idx ) ? 1 : 0;
    xNetworkInfo.savedIdx = ( int32_t ) idx;

    xSerializerRet = prxSerializeNetwork( &xNetworkInfo, NULL, &xMessageLen );
    if( xSerializerRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        pucMessage = pvPortMalloc( xMessageLen );
        if( pucMessage != NULL )
        {
            xSerializerRet = prxSerializeNetwork( &xNetworkInfo, pucMessage, &xMessageLen );
        }
        else
        {
            xSerializerRet = AWS_IOT_SERIALIZER_OUT_OF_MEMORY;
        }
    }

    if( xSerializerRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        prvSendResponse( IOT_BLE_WIFI_PROV_LIST_NETWORK_CHAR, pucMessage, xMessageLen );
    }
    else
    {
        configPRINTF(( "Failed to send network profile, SSID:%*s\n",
                pxSavedNetwork->ucSSIDLength, pxSavedNetwork->cSSID ));
    }

    if( pucMessage != NULL )
    {
        vPortFree( pucMessage );
    }
}

static void prvSendScanNetwork( WIFIScanResult_t *pScanNetwork )
{
    IotBleWifiNetworkInfo_t xNetworkInfo = NETWORK_INFO_DEFAULT_PARAMS;
    uint8_t *pucMessage = NULL;
    size_t xMessageLen = 0;
    AwsIotSerializerError_t xSerializerRet;

    xNetworkInfo.pSSID = pScanNetwork->cSSID;
    xNetworkInfo.SSIDLength = strlen( pScanNetwork->cSSID );
    xNetworkInfo.pBSSID = pScanNetwork->ucBSSID;
    xNetworkInfo.BSSIDLength = wificonfigMAX_BSSID_LEN;
    xNetworkInfo.RSSI = pScanNetwork->cRSSI;
    xNetworkInfo.hidden = ( bool ) pScanNetwork->ucHidden;
    xNetworkInfo.security = pScanNetwork->xSecurity;

    xSerializerRet = prxSerializeNetwork( &xNetworkInfo, NULL, &xMessageLen );
    if( xSerializerRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        pucMessage = pvPortMalloc( xMessageLen );
        if( pucMessage != NULL )
        {
            xSerializerRet = prxSerializeNetwork( &xNetworkInfo, pucMessage, &xMessageLen );
        }
        else
        {
            xSerializerRet = AWS_IOT_SERIALIZER_OUT_OF_MEMORY;
        }
    }

    if( xSerializerRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        prvSendResponse( IOT_BLE_WIFI_PROV_LIST_NETWORK_CHAR, pucMessage, xMessageLen );
    }
    else
    {
        configPRINTF(( "Failed to send network profile, SSID:%s\n",
                pScanNetwork->cSSID ));
    }

    if( pucMessage != NULL )
    {
        vPortFree( pucMessage );
    }
}
/*-----------------------------------------------------------*/

void prvListNetworkTask( void * pvParams )
{
    IotBleListNetworkRequest_t * pxListNetworReq = ( IotBleListNetworkRequest_t * ) pvParams;
    WIFIScanResult_t xScanResults[ pxListNetworReq->maxNetworks ];
    WIFINetworkProfile_t xProfile;
    uint16_t idx;
    WIFIReturnCode_t xWifiRet;

    if( xSemaphoreTake( xWifiProvService.lock, portMAX_DELAY ) == pdPASS )
    {
    	for( idx = 0; idx < xWifiProvService.numNetworks; idx++ )
    	{
    	    xWifiRet = prvGetSavedNetwork( idx, &xProfile );
    		if( xWifiRet == eWiFiSuccess )
    		{
    		    prvSendSavedNetwork( &xProfile, idx );
    		}
    	}

    	memset( xScanResults, 0x00, sizeof( WIFIScanResult_t ) * pxListNetworReq->maxNetworks );
    	xWifiRet = WIFI_Scan( xScanResults, pxListNetworReq->maxNetworks );
    	if( xWifiRet == eWiFiSuccess )
    	{
    		for( idx = 0; idx < pxListNetworReq->maxNetworks; idx++ )
    		{
    			if( strlen( xScanResults[ idx ].cSSID ) > 0 )
    			{
    			    prvSendScanNetwork( &xScanResults[ idx ] );
    			}
    		}
    	}
    	else
    	{
    		prvSendStatusResponse( IOT_BLE_WIFI_PROV_LIST_NETWORK_CHAR, xWifiRet );
    	}


    	xSemaphoreGive( xWifiProvService.lock );
    }

    vPortFree( pxListNetworReq );
    vTaskDelete( NULL );
}

/*-----------------------------------------------------------*/

void prvAddNetworkTask( void * pvParams )
{
    WIFIReturnCode_t xRet = eWiFiFailure;
    IotBleAddNetworkRequest_t * pxAddNetworkReq = ( IotBleAddNetworkRequest_t * ) pvParams;

    if( xSemaphoreTake( xWifiProvService.lock, portMAX_DELAY ) == pdPASS )
    {
        if( pxAddNetworkReq->savedIdx != IOT_BLE_WIFI_PROV_INVALID_NETWORK_INDEX )
        {
        	xRet = prvConnectSavedNetwork( pxAddNetworkReq->savedIdx );
        }
        else
        {
        	xRet = prvAddNewNetwork( &pxAddNetworkReq->network );
        }
        xSemaphoreGive( xWifiProvService.lock );
    }

    prvSendStatusResponse( IOT_BLE_WIFI_PROV_SAVE_NETWORK_CHAR, xRet );
    vPortFree( pxAddNetworkReq );
    vTaskDelete( NULL );
}



/*-----------------------------------------------------------*/

void prvDeleteNetworkTask( void * pvParams )
{
    WIFIReturnCode_t xRet = eWiFiFailure;
    IotBleDeleteNetworkRequest_t * pxDeleteNetworkReq = ( IotBleDeleteNetworkRequest_t * ) pvParams;


    if( xSemaphoreTake( xWifiProvService.lock, portMAX_DELAY ) == pdPASS )
    {
        xRet = prvPopNetwork( pxDeleteNetworkReq->idx, NULL );

        if( xRet == eWiFiSuccess )
        {
            if( xWifiProvService.connectedIdx == IOT_BLE_WIFI_PROV_INVALID_NETWORK_INDEX )
            {
                ( void ) WIFI_Disconnect();
            }
        }
        xSemaphoreGive( xWifiProvService.lock );
    }

    prvSendStatusResponse( IOT_BLE_WIFI_PROV_DELETE_NETWORK_CHAR, xRet );
    vPortFree( pxDeleteNetworkReq );
    vTaskDelete( NULL );
}



/*-----------------------------------------------------------*/

void prvEditNetworkTask( void * pvParams )
{
    WIFIReturnCode_t xRet = eWiFiFailure;
    IotBleEditNetworkRequest_t * pxEditNetworkReq = ( IotBleEditNetworkRequest_t * ) pvParams;

    if( xSemaphoreTake( xWifiProvService.lock, portMAX_DELAY ) == pdPASS )
    {
        xRet = prvMoveNetwork( pxEditNetworkReq->curIdx, pxEditNetworkReq->newIdx );
        xSemaphoreGive( xWifiProvService.lock );
    }

    prvSendStatusResponse(  IOT_BLE_WIFI_PROV_EDIT_NETWORK_CHAR, xRet );
    vPortFree( pxEditNetworkReq );
    vTaskDelete( NULL );
}

/*-----------------------------------------------------------*/

BaseType_t IotBleWifiProv_Init( void )
{
	BTStatus_t status = eBTStatusSuccess;
	BTStatus_t error = pdFAIL;

    if( xWifiProvService.init == pdFALSE )
    {

		xWifiProvService.lock = xSemaphoreCreateMutex();

		if( xWifiProvService.lock != NULL )
		{
			xSemaphoreGive( xWifiProvService.lock );
		}
		else
		{
			status = eBTStatusFail;
		}

        if( status == eBTStatusSuccess )
        {
        	xWifiProvService.pGattService = ( BTService_t *)&xWIFIProvisionningService;
        	status = IotBle_CreateService( (BTService_t *)&xWIFIProvisionningService, (IotBleAttributeEventCallback_t *)pxCallBackArray );
        }
        if( status == eBTStatusSuccess )
        {
        	xWifiProvService.connectedIdx = IOT_BLE_WIFI_PROV_INVALID_NETWORK_INDEX;
        	xWifiProvService.init = pdTRUE;
        }
    }
    else
    {
    	status = eBTStatusFail;
    }

    if(status == eBTStatusSuccess)
    {
    	error = pdPASS;
    }

    return error;
}

/*-----------------------------------------------------------*/
uint16_t IotBleWifiProv_GetNumNetworks( void )
{
	return prvGetNumSavedNetworks();
}

BaseType_t IotBleWifiProv_Connect( uint16_t usNetworkIndex )
{
	WIFIReturnCode_t xWifiRet;
	BaseType_t xRet = pdFALSE;
	xWifiRet = prvConnectSavedNetwork( usNetworkIndex );
	if( xWifiRet == eWiFiSuccess )
	{
		xRet = pdTRUE;
	}
	return xRet;
}


BaseType_t IotBleWifiProv_EraseAllNetworks( void )
{

    BaseType_t xRet = pdTRUE;
    WIFIReturnCode_t xWiFiRet;

    if( xSemaphoreTake( xWifiProvService.lock, portMAX_DELAY ) == pdPASS )
    {
        while( xWifiProvService.numNetworks > 0 )
        {
            xWiFiRet = prvPopNetwork( 0, NULL );
            if( xWiFiRet != eWiFiSuccess )
            {
                configPRINTF(( "Failed to delete WIFI network, error = %d\n", xWiFiRet ));
                xRet = pdFALSE;
                break;
            }
        }
        xSemaphoreGive( xWifiProvService.lock );
    }
    else
    {
        xRet = pdFALSE;
    }

    return xRet;
}

/*-----------------------------------------------------------*/

BaseType_t IotBleWifiProv_Delete( void )
{
	BaseType_t xRet = pdFALSE;

    if( IotBle_DeleteService( xWifiProvService.pGattService ) == eBTStatusSuccess )
    {
    	xRet = pdTRUE;
    }
    if( xRet == pdTRUE )
    {

    	if( xWifiProvService.lock != NULL )
    	{
    		vSemaphoreDelete( xWifiProvService.lock );
    	}

    	memset( &xWifiProvService, 0x00, sizeof( IotBleWifiProvService_t ) );
    }

    return xRet;

}

/* Provide access to private members for testing. */
#ifdef AMAZON_FREERTOS_ENABLE_UNIT_TESTS
    #include "iot_ble_wifi_prov_test_access_define.h"
#endif
