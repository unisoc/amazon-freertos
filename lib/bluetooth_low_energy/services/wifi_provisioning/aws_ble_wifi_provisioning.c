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

#include <aws_ble.h>
#include <string.h>

#include "private/aws_ble_service_internals.h"
#include "aws_ble_wifi_provisioning.h"
#include "semphr.h"

#define ATTR_DATA( svc, attrType )      ( ( svc )->pxGattService->pxCharacteristics[ attrType ].xAttributeData )
#define ATTR_HANDLE( svc, attrType )    ( ( ATTR_DATA( svc, attrType ) ).xHandle )
#define ATTR_UUID( svc, attrType )      ( ( ATTR_DATA( svc, attrType ) ).xUuid )

#define IS_VALID_SERIALIZER_RET( ret, pxSerializerBuf )                                \
    (  ( ret == AWS_IOT_SERIALIZER_SUCCESS ) ||                                        \
          (  ( !pxSerializerBuf ) && ( ret == AWS_IOT_SERIALIZER_BUFFER_TOO_SMALL ) ) )
/*---------------------------------------------------------------------------------------------------------*/

static WifiProvService_t xWifiProvService = { 0 };

#define STORAGE_INDEX( priority )    ( xWifiProvService.usNumNetworks - priority - 1 )
#define NETWORK_INFO_DEFAULT_PARAMS        { .xStatus = eWiFiSuccess, .cRSSI = wifiProvINVALID_NETWORK_RSSI, .ucConnected = 0, .sSavedIdx = wifiProvINVALID_NETWORK_INDEX }
/*---------------------------------------------------------------------------------------------------------*/

/*
 * @brief Callback registered for BLE write and read events received for each characteristic.
 */
static void vCharacteristicCallback( BLEAttribute_t * pxAttribute,
                                     BLEAttributeEvent_t * pxEventParam );

/*
 * @brief Callback registered for client characteristic configuration descriptors.
 */
static void vClientCharCfgDescrCallback( BLEAttribute_t * pxAttribute,
                                         BLEAttributeEvent_t * pxEventParam );

/*
 * @brief Callback registered for receiving notification when the BLE service is started.
 */
static void vServiceStartedCb( BTStatus_t xStatus,
                               BLEService_t * pxService );

/*
 * @brief Callback registered for receiving notification when the GATT service is stopped.
 */
static void vServiceStoppedCb( BTStatus_t xStatus,
                               BLEService_t * pxService );

/*
 * @brief Callback registered for receiving notification when the GATT service is deleted.
 */
static void vServiceDeletedCb( BTStatus_t xStatus,
                               uint16_t usAttributeDataHandle );

/*
 * @brief Function used to create and initialize BLE service.
 */
static BaseType_t prxInitGATTService( void );

/*
 * @brief Helper function to clear a WiFi provisioning event.
 */
static void prvClearEvent( WifiProvEvent_t xEvent );

/*
 * @brief Helper function to set an WiFi provisioning event.
 */
static void prvSetEvent( WifiProvEvent_t xEvent );

/*
 * @brief Helper function to wait for a WiFi provisioning event.
 */
static BaseType_t prvWaitForEvent( WifiProvEvent_t xEvent,
                                          TickType_t xTimeout );

static BaseType_t prxDeserializeListNetworkRequest( uint8_t * pucData, size_t xLength, ListNetworkRequest_t* pxListNetworkRequest );

/*
 * @brief Parses List Network request params and creates task to list networks.
 */
static BaseType_t prxHandleListNetworkRequest( uint8_t * pucData,
                                               size_t xLength );


static BaseType_t prxDeserializeAddNetworkRequest( uint8_t * pucData, size_t xLength, AddNetworkRequest_t* pxAddNetworkRequest );
/*
 * @brief Parses Save Network request params and creates task to save the new network.
 */
static BaseType_t prxHandleSaveNetworkRequest( uint8_t * pucData,
                                               size_t xLength );


static BaseType_t prxDeserializeEditNetworkRequest( uint8_t * pucData, size_t xLength, EditNetworkRequest_t* pxEditNetworkRequest );

/*
 * @brief Parses Edit Network request params and creates task to edit network priority.
 */
static BaseType_t prxHandleEditNetworkRequest( uint8_t * pucData,
                                               size_t xLength );

static BaseType_t prxDeserializeDeleteNetworkRequest( uint8_t * pucData, size_t xLength, DeleteNetworkRequest_t* pxDeleteNetworkRequest );

/*
 * @brief Parses Delete Network request params and creates task to delete a WiFi networ.
 */
static BaseType_t prxHandleDeleteNetworkRequest( uint8_t * pucData,
                                                 size_t xLength );

/*
 * @brief Gets the GATT characteristic for a given attribute handle.
 */
static WifiProvCharacteristic_t prxGetCharFromHandle( uint16_t xHandle );


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


static AwsIotSerializerError_t prxSerializeNetwork( WifiNetworkInfo_t *pxNetworkInfo, uint8_t *pucBuffer, size_t *pxLength );

static void prvSendSavedNetwork( WIFINetworkProfile_t *pxSavedNetwork, uint16_t usIdx );

static void prvSendScanNetwork( WIFIScanResult_t *pxScanNetwork );

static AwsIotSerializerError_t prxSerializeStatusResponse( WIFIReturnCode_t xStatus, uint8_t* pucBuffer, size_t* pxLength );

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
 * @brief  The task runs in background and tries to connect to one of the saved networks in a loop. It loops until
 * a connection is successful.
 */
static void prvConnectAPTask( void * pvParams );

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
void prvSendStatusResponse( WifiProvCharacteristic_t xChar, WIFIReturnCode_t xStatus );


void prvSendResponse( WifiProvCharacteristic_t xCharacteristic, uint8_t* pucData, size_t xLen );
/* -------------------------------------------------------*/

static void vServiceStartedCb( BTStatus_t xStatus,
                               BLEService_t * pxService )
{
    if( xStatus == eBTStatusSuccess )
    {
        prvSetEvent( eWIFIPROVStarted );
    }
    else
    {
        prvSetEvent( eWIFIPROVFailed );
    }
}

/*-----------------------------------------------------------*/

static void vServiceStoppedCb( BTStatus_t xStatus,
                               BLEService_t * pxService )
{
    if( xStatus == eBTStatusSuccess )
    {
        prvSetEvent( eWIFIPROVStopped );
    }
    else
    {
        prvSetEvent( eWIFIPROVFailed );
    }
}

/*-----------------------------------------------------------*/

static void vServiceDeletedCb( BTStatus_t xStatus,
                               uint16_t usAttributeDataHandle )
{
    if( xStatus == eBTStatusSuccess )
    {
        prvSetEvent( eWIFIPROVDeleted );
    }
    else
    {
        prvSetEvent( eWIFIPROVFailed );
    }
}

/*-----------------------------------------------------------*/

BaseType_t prxInitGATTService( void )
{
    BTStatus_t xStatus;
    BTUuid_t xUUID =
    {
        .uu.uu128 = wifiProvSVC_UUID_BASE,
        .ucType   = eBTuuidType128
    };

    BTUuid_t xClientCharCfgUUID =
    {
        .uu.uu16 = wifiProvCLIENT_CHAR_CFG_UUID,
        .ucType  = eBTuuidType16
    };

    struct BLEService * pxGattService = NULL;
    size_t xNumDescrsPerChar[ wifiProvNUM_CHARS ] = { 1, 1, 1, 1 };

    xStatus = BLE_CreateService( &xWifiProvService.pxGattService, wifiProvNUM_CHARS, wifiProvNUM_DESCRS, xNumDescrsPerChar, wifiProvNum_INCL_SERVICES );

    if( xStatus == eBTStatusSuccess )
    {
        pxGattService = xWifiProvService.pxGattService;
        configASSERT( pxGattService->xNbCharacteristics == wifiProvNUM_CHARS );
        configASSERT( pxGattService->xNbDescriptors == wifiProvNUM_DESCRS );
        configASSERT( pxGattService->xNbIncludedServices == wifiProvNum_INCL_SERVICES );

        xUUID.uu.uu16 = wifiProvSVC_UUID;
        pxGattService->xAttributeData.xUuid = xUUID;

        pxGattService->pxDescriptors[ eListNetworkCharCCFGDescr ].xAttributeData.xUuid = xClientCharCfgUUID;
        pxGattService->pxDescriptors[ eListNetworkCharCCFGDescr ].xAttributeData.pucData = NULL;
        pxGattService->pxDescriptors[ eListNetworkCharCCFGDescr ].xAttributeData.xSize = 0;
        pxGattService->pxDescriptors[ eListNetworkCharCCFGDescr ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxDescriptors[ eListNetworkCharCCFGDescr ].pxAttributeEventCallback = vClientCharCfgDescrCallback;

        xUUID.uu.uu16 = wifiProvLIST_NETWORK_CHAR_UUID;
        pxGattService->pxCharacteristics[ eListNetworkChar ].xAttributeData.xUuid = xUUID;
        pxGattService->pxCharacteristics[ eListNetworkChar ].xAttributeData.pucData = NULL;
        pxGattService->pxCharacteristics[ eListNetworkChar ].xAttributeData.xSize = 0;
        pxGattService->pxCharacteristics[ eListNetworkChar ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxCharacteristics[ eListNetworkChar ].xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify );
        pxGattService->pxCharacteristics[ eListNetworkChar ].pxAttributeEventCallback = vCharacteristicCallback;
        pxGattService->pxCharacteristics[ eListNetworkChar ].xNbDescriptors = 1;
        pxGattService->pxCharacteristics[ eListNetworkChar ].pxDescriptors[ 0 ] = &pxGattService->pxDescriptors[ eListNetworkCharCCFGDescr ];

        pxGattService->pxDescriptors[ eSaveNetworkCharCCFGDescr ].xAttributeData.xUuid = xClientCharCfgUUID;
        pxGattService->pxDescriptors[ eSaveNetworkCharCCFGDescr ].xAttributeData.pucData = NULL;
        pxGattService->pxDescriptors[ eSaveNetworkCharCCFGDescr ].xAttributeData.xSize = 0;
        pxGattService->pxDescriptors[ eSaveNetworkCharCCFGDescr ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxDescriptors[ eSaveNetworkCharCCFGDescr ].pxAttributeEventCallback = vClientCharCfgDescrCallback;

        xUUID.uu.uu16 = wifiProvSAVE_NETWORK_CHAR_UUID;
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].xAttributeData.xUuid = xUUID;
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].xAttributeData.pucData = NULL;
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].xAttributeData.xSize = 0;
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify );
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].pxAttributeEventCallback = vCharacteristicCallback;
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].xNbDescriptors = 1;
        pxGattService->pxCharacteristics[ eSaveNetworkChar ].pxDescriptors[ 0 ] = &pxGattService->pxDescriptors[ eSaveNetworkCharCCFGDescr ];

        pxGattService->pxDescriptors[ eEditNetworkCharCCFGDescr ].xAttributeData.xUuid = xClientCharCfgUUID;
        pxGattService->pxDescriptors[ eEditNetworkCharCCFGDescr ].xAttributeData.pucData = NULL;
        pxGattService->pxDescriptors[ eEditNetworkCharCCFGDescr ].xAttributeData.xSize = 0;
        pxGattService->pxDescriptors[ eEditNetworkCharCCFGDescr ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxDescriptors[ eEditNetworkCharCCFGDescr ].pxAttributeEventCallback = vClientCharCfgDescrCallback;

        xUUID.uu.uu16 = wifiProvEDIT_NETWORK_CHAR_UUID;
        pxGattService->pxCharacteristics[ eEditNetworkChar ].xAttributeData.xUuid = xUUID;
        pxGattService->pxCharacteristics[ eEditNetworkChar ].xAttributeData.pucData = NULL;
        pxGattService->pxCharacteristics[ eEditNetworkChar ].xAttributeData.xSize = 0;
        pxGattService->pxCharacteristics[ eEditNetworkChar ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxCharacteristics[ eEditNetworkChar ].xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify );
        pxGattService->pxCharacteristics[ eEditNetworkChar ].pxAttributeEventCallback = vCharacteristicCallback;
        pxGattService->pxCharacteristics[ eEditNetworkChar ].xNbDescriptors = 1;
        pxGattService->pxCharacteristics[ eEditNetworkChar ].pxDescriptors[ 0 ] = &pxGattService->pxDescriptors[ eEditNetworkCharCCFGDescr ];

        pxGattService->pxDescriptors[ eDeleteNetworkCharCCFGDescr ].xAttributeData.xUuid = xClientCharCfgUUID;
        pxGattService->pxDescriptors[ eDeleteNetworkCharCCFGDescr ].xAttributeData.pucData = NULL;
        pxGattService->pxDescriptors[ eDeleteNetworkCharCCFGDescr ].xAttributeData.xSize = 0;
        pxGattService->pxDescriptors[ eDeleteNetworkCharCCFGDescr ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxDescriptors[ eDeleteNetworkCharCCFGDescr ].pxAttributeEventCallback = vClientCharCfgDescrCallback;

        xUUID.uu.uu16 = wifiProvDELETE_NETWORK_CHAR_UUID;
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].xAttributeData.xUuid = xUUID;
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].xAttributeData.pucData = NULL;
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].xAttributeData.xSize = 0;
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].xPermissions = ( bleconfigCHAR_READ_PERM | bleconfigCHAR_WRITE_PERM );
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].xProperties = ( eBTPropRead | eBTPropWrite | eBTPropNotify );
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].pxAttributeEventCallback = vCharacteristicCallback;
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].xNbDescriptors = 1;
        pxGattService->pxCharacteristics[ eDeleteNetworkChar ].pxDescriptors[ 0 ] = &pxGattService->pxDescriptors[ eDeleteNetworkCharCCFGDescr ];

        pxGattService->xServiceType = eBTServiceTypePrimary;

        xStatus = BLE_AddService( pxGattService );
    }

    return wifiProvIS_SUCCESS( xStatus );
}

/*-----------------------------------------------------------*/

static WifiProvCharacteristic_t prxGetCharFromHandle( uint16_t usHandle )
{
    uint8_t ucCharId;
    BLEService_t * pxService = xWifiProvService.pxGattService;

    for( ucCharId = 0; ucCharId < eMaxChars; ucCharId++ )
    {
        if( pxService->pxCharacteristics[ ucCharId ].xAttributeData.xHandle == usHandle )
        {
            break;
        }
    }

    return ( WifiProvCharacteristic_t ) ucCharId;
}

/*-----------------------------------------------------------*/

void vCharacteristicCallback( BLEAttribute_t * pxAttribute,
                              BLEAttributeEvent_t * pxEventParam )
{
    BLEWriteEventParams_t * pxWriteParam;
    BLEReadEventParams_t * pxReadParam;
    BaseType_t xResult = pdFAIL;

    BLEAttributeData_t xAttrData = { 0 };
    BLEEventResponse_t xResp =
    {
        .xEventStatus    = eBTStatusFail,
        .xRspErrorStatus = eBTRspErrorNone
    };
    WifiProvCharacteristic_t xChar;

    xResp.pxAttrData = &xAttrData;

    if( pxEventParam->xEventType == eBLEWrite )
    {
        pxWriteParam = pxEventParam->pxParamWrite;
        xWifiProvService.usBLEConnId = pxWriteParam->usConnId;

        xChar = prxGetCharFromHandle( pxWriteParam->pxAttribute->pxCharacteristic->xAttributeData.xHandle );

        switch( xChar )
        {
            case eListNetworkChar:
                xResult = prxHandleListNetworkRequest( pxWriteParam->pucValue, pxWriteParam->xLength );
                break;

            case eSaveNetworkChar:
                xResult = prxHandleSaveNetworkRequest( pxWriteParam->pucValue, pxWriteParam->xLength );
                break;

            case eEditNetworkChar:
                xResult = prxHandleEditNetworkRequest( pxWriteParam->pucValue, pxWriteParam->xLength );
                break;

            case eDeleteNetworkChar:
                xResult = prxHandleDeleteNetworkRequest( pxWriteParam->pucValue, pxWriteParam->xLength );
                break;

            case eMaxChars:
            default:
                xResult = pdFAIL;
                break;
        }

        if( xResult == pdPASS )
        {
            xResp.xEventStatus = eBTStatusSuccess;
        }

        xResp.pxAttrData->xHandle = pxWriteParam->pxAttribute->pxCharacteristic->xAttributeData.xHandle;
        xResp.pxAttrData->pucData = pxWriteParam->pucValue;
        xResp.pxAttrData->xSize = pxWriteParam->xLength;
        xResp.xAttrDataOffset = pxWriteParam->usOffset;
        BLE_SendResponse( &xResp, pxWriteParam->usConnId, pxWriteParam->ulTransId );
    }
    else if( pxEventParam->xEventType == eBLERead )
    {
        pxReadParam = pxEventParam->pxParamRead;
        xResp.pxAttrData->xHandle = pxReadParam->pxAttribute->pxCharacteristic->xAttributeData.xHandle;
        xResp.pxAttrData->pucData = NULL;
        xResp.pxAttrData->xSize = 0;
        xResp.xAttrDataOffset = 0;
        xResp.xEventStatus = eBTStatusSuccess;
        BLE_SendResponse( &xResp, pxReadParam->usConnId, pxReadParam->ulTransId );
    }
}

/*-----------------------------------------------------------*/

static void vClientCharCfgDescrCallback( BLEAttribute_t * pxAttribute,
                                         BLEAttributeEvent_t * pxEventParam )
{
    BLEWriteEventParams_t * pxWriteParam;
    BLEReadEventParams_t * pxReadParam;

    BLEAttributeData_t xAttrData = { 0 };
    BLEEventResponse_t xResp =
    {
        .xEventStatus    = eBTStatusSuccess,
        .xRspErrorStatus = eBTRspErrorNone
    };

    xResp.pxAttrData = &xAttrData;

    if( pxEventParam->xEventType == eBLEWrite )
    {
        pxWriteParam = pxEventParam->pxParamWrite;

        if( pxWriteParam->xLength == 2 )
        {
            xWifiProvService.usNotifyClientEnabled = ( pxWriteParam->pucValue[ 1 ] << 8 ) | pxWriteParam->pucValue[ 0 ];
            xWifiProvService.usBLEConnId = pxWriteParam->usConnId;
            xResp.pxAttrData->xHandle = pxWriteParam->pxAttribute->pxCharacteristic->xAttributeData.xHandle;
            xResp.pxAttrData->pucData = pxWriteParam->pucValue;
            xResp.pxAttrData->xSize = pxWriteParam->xLength;
            xResp.xAttrDataOffset = pxWriteParam->usOffset;
        }

        BLE_SendResponse( &xResp, pxWriteParam->usConnId, pxWriteParam->ulTransId );
    }
    else if( pxEventParam->xEventType == eBLERead )
    {
        pxReadParam = pxEventParam->pxParamRead;
        xResp.pxAttrData->xHandle = pxReadParam->pxAttribute->pxCharacteristic->xAttributeData.xHandle;
        xResp.pxAttrData->pucData = ( uint8_t * ) &xWifiProvService.usNotifyClientEnabled;
        xResp.pxAttrData->xSize = 2;
        xResp.xAttrDataOffset = 0;
        xResp.xEventStatus = eBTStatusSuccess;
        BLE_SendResponse( &xResp, pxReadParam->usConnId, pxReadParam->ulTransId );
    }
}

/*-----------------------------------------------------------*/

static uint16_t prvGetNumSavedNetworks( void )
{
    uint16_t usNumNetworks = 0, usIdx;
    WIFIReturnCode_t xWifiRet;
    WIFINetworkProfile_t xProfile;

    for( usIdx = 0; usIdx < wifiProvMAX_SAVED_NETWORKS; usIdx++ )
    {
        xWifiRet = WIFI_NetworkGet( &xProfile, usIdx );

        if( xWifiRet == eWiFiSuccess )
        {
            usNumNetworks++;
        }
        else
        {
            break;
        }
    }

    return usNumNetworks;
}

static BaseType_t prxDeserializeListNetworkRequest( uint8_t * pucData, size_t xLength, ListNetworkRequest_t* pxListNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = bleMESSAGE_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

    if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
            ( xDecoderObj.type != AWS_IOT_SERIALIZER_CONTAINER_MAP ) )
    {
        configPRINTF(( "Failed to initialize the decoder, error = %d, object type = %d\n", xRet, xDecoderObj.type ));
        xResult = pdFALSE;
    }

    if( xResult == pdTRUE )
    {
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvMAX_NETWORKS_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get max Networks parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt < 0 )
                    || ( xValue.value.signedInt > bleconfigMAX_NETWORK ) )
            {
                configPRINTF(( "WARN: Max Networks (%d) exceeds configured Max networks (%d). Caping max networks to %d\n",
                        xValue.value.signedInt,
                        bleconfigMAX_NETWORK,
                        bleconfigMAX_NETWORK ));
                pxListNetworkRequest->sMaxNetworks = bleconfigMAX_NETWORK;
            }
            else
            {
                pxListNetworkRequest->sMaxNetworks = xValue.value.signedInt;
            }
        }
    }

    if( xResult == pdTRUE )
    {
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvSCAN_TIMEOUT_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get timeout parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {

            pxListNetworkRequest->sTimeoutMs = xValue.value.signedInt;
        }
    }

    bleMESSAGE_DECODER.destroy( &xDecoderObj );
    return xResult;
}


/*-----------------------------------------------------------*/

static BaseType_t prxHandleListNetworkRequest( uint8_t * pucData,
                                               size_t xLength )
{
    BaseType_t xStatus = pdFALSE;
    ListNetworkRequest_t * pxParams = pvPortMalloc( sizeof( ListNetworkRequest_t ));
    if( pxParams != NULL )
    {
        xStatus = prxDeserializeListNetworkRequest( pucData, xLength, pxParams );
        if( xStatus == pdTRUE )
        {
            xStatus = xTaskCreate(
                    prvListNetworkTask,
                    "WifiProvListNetwork",
                    configMINIMAL_STACK_SIZE * 6,
                    pxParams,
                    wifiProvLIST_NETWORK_TASK_PRIORITY,
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

    return xStatus;
}

static BaseType_t prxDeserializeAddNetworkRequest( uint8_t * pucData, size_t xLength, AddNetworkRequest_t* pxAddNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = bleMESSAGE_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

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
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvSSID_KEY, &xValue );
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
                strncpy( pxAddNetworkRequest->xNetwork.cSSID, ( const char * ) xValue.value.pString, wificonfigMAX_SSID_LEN );
                pxAddNetworkRequest->xNetwork.ucSSIDLength = ( xValue.value.stringLength + 1 );
            }
        }
    }

    if( xResult == pdTRUE )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_BYTE_STRING;
        xValue.value.pString = NULL;
        xValue.value.stringLength = 0;
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvBSSID_KEY, &xValue );
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
                memcpy( pxAddNetworkRequest->xNetwork.ucBSSID, xValue.value.pString, wificonfigMAX_BSSID_LEN );
            }
        }
    }

    if( xResult == pdTRUE )
    {
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvKEY_MGMT_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get WIFI security parameter, error = %d, value type = %d\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            pxAddNetworkRequest->xNetwork.xSecurity = xValue.value.signedInt;
        }
    }

    if( xResult == pdTRUE )
    {
        xValue.value.pString = NULL;
          xValue.value.stringLength = 0;
          xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvKEY_MGMT_KEY, &xValue );
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
                  strncpy( pxAddNetworkRequest->xNetwork.cPassword, ( const char * ) xValue.value.pString, wificonfigMAX_PASSPHRASE_LEN );
                  pxAddNetworkRequest->xNetwork.ucPasswordLength = ( uint16_t )( xValue.value.stringLength + 1 );
              }
          }
    }

    if( xResult == pdTRUE )
    {
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvINDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= wifiProvINVALID_NETWORK_INDEX )
                    && ( xValue.value.signedInt < wifiProvMAX_SAVED_NETWORKS  ) )
            {
                pxAddNetworkRequest->sSavedIdx = xValue.value.signedInt;
            }
            else
            {
                configPRINTF(( "Network index parameter ( %d ) is out of range.\n", xValue.value.signedInt ));
                xResult = pdFALSE;
            }
        }
    }

    bleMESSAGE_DECODER.destroy( &xDecoderObj );

    return xResult;
}

/*-----------------------------------------------------------*/

static BaseType_t prxHandleSaveNetworkRequest( uint8_t * pucData,
                                               size_t xLength )
{
    BaseType_t xStatus = pdFALSE;
    AddNetworkRequest_t *pxParams = pvPortMalloc( sizeof( AddNetworkRequest_t ));

    if( pxParams != NULL )
    {
        xStatus = prxDeserializeAddNetworkRequest( pucData, xLength, pxParams );

        if( xStatus == pdTRUE )
        {
            xStatus = xTaskCreate(
                    prvAddNetworkTask,
                    "WifiProvAddNetwork",
                    configMINIMAL_STACK_SIZE * 4,
                    pxParams,
                    wifiProvMODIFY_NETWORK_TASK_PRIORITY,
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

    return xStatus;
}

static BaseType_t prxDeserializeEditNetworkRequest( uint8_t * pucData, size_t xLength, EditNetworkRequest_t* pxEditNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = bleMESSAGE_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

    if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
            ( xDecoderObj.type != AWS_IOT_SERIALIZER_CONTAINER_MAP ) )
    {
        configPRINTF(( "Failed to initialize decoder, error = %d, object type = %d\n", xRet, xDecoderObj.type ));
        xResult = pdFALSE;
    }

    if( xResult == pdTRUE )
    {
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvINDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= 0 )
                    && ( xValue.value.signedInt < wifiProvMAX_SAVED_NETWORKS  ) )
            {
                pxEditNetworkRequest->sCurIdx = xValue.value.signedInt;
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
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvNEWINDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get new network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= 0 )
                    && ( xValue.value.signedInt < wifiProvMAX_SAVED_NETWORKS  ) )
            {
                pxEditNetworkRequest->sNewIdx = xValue.value.signedInt;
            }
            else
            {
                configPRINTF(( "New Network index parameter ( %d ) is out of range.\n", xValue.value.signedInt ));
                xResult = pdFALSE;
            }
        }
    }

    bleMESSAGE_DECODER.destroy( &xDecoderObj );

    return xResult;
}

/*-----------------------------------------------------------*/

static BaseType_t prxHandleEditNetworkRequest( uint8_t * pucData, size_t xLength )
{
    BaseType_t xStatus = pdFALSE;
    EditNetworkRequest_t *pxParams = pvPortMalloc( sizeof( EditNetworkRequest_t ));

    if( pxParams != NULL )
    {
        xStatus = prxDeserializeEditNetworkRequest( pucData, xLength, pxParams );

        if( xStatus == pdTRUE )
        {
            xStatus = xTaskCreate(
                    prvEditNetworkTask,
                    "WifiProvEditNetwork",
                    configMINIMAL_STACK_SIZE * 4,
                    pxParams,
                    wifiProvMODIFY_NETWORK_TASK_PRIORITY,
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

    return xStatus;
}

static BaseType_t prxDeserializeDeleteNetworkRequest( uint8_t * pucData, size_t xLength, DeleteNetworkRequest_t* pxDeleteNetworkRequest )
{

    AwsIotSerializerDecoderObject_t xDecoderObj = { 0 }, xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    BaseType_t xResult = pdTRUE;

    xRet = bleMESSAGE_DECODER.init( &xDecoderObj, ( uint8_t * ) pucData, xLength );

    if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
            ( xDecoderObj.type != AWS_IOT_SERIALIZER_CONTAINER_MAP ) )
    {
        configPRINTF(( "Failed to initialize decoder, error = %d, object type = %d\n", xRet, xDecoderObj.type ));
        xResult = pdFALSE;
    }

    if( xResult == pdTRUE )
    {
        xRet = bleMESSAGE_DECODER.find( &xDecoderObj, wifiProvINDEX_KEY, &xValue );
        if( ( xRet != AWS_IOT_SERIALIZER_SUCCESS ) ||
                ( xValue.type != AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT ) )
        {
            configPRINTF(( "Failed to get network index parameter, error = %d, value type = %d.\n", xRet, xValue.type ));
            xResult = pdFALSE;
        }
        else
        {
            if( ( xValue.value.signedInt >= 0 )
                    && ( xValue.value.signedInt < wifiProvMAX_SAVED_NETWORKS  ) )
            {
                pxDeleteNetworkRequest->sIdx = ( int16_t ) xValue.value.signedInt;
            }
            else
            {
                configPRINTF(( "Network index parameter ( %d ) is out of range.\n", xValue.value.signedInt ));
                xResult = pdFALSE;
            }
        }
    }

    bleMESSAGE_DECODER.destroy( &xDecoderObj );

    return xResult;
}

/*-----------------------------------------------------------*/

static BaseType_t prxHandleDeleteNetworkRequest( uint8_t * pucData,
                                                 size_t xLength )
{
    BaseType_t xStatus = pdFALSE;
    DeleteNetworkRequest_t *pxParams = pvPortMalloc( sizeof( DeleteNetworkRequest_t ));

    if( pxParams != NULL )
    {
        xStatus = prxDeserializeDeleteNetworkRequest( pucData, xLength, pxParams );

        if( xStatus == pdTRUE )
        {
            xStatus = xTaskCreate(
                    prvDeleteNetworkTask,
                    "WifiProvDeleteNetwork",
                    configMINIMAL_STACK_SIZE * 4,
                    pxParams,
                    wifiProvMODIFY_NETWORK_TASK_PRIORITY,
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

    return xStatus;
}


AwsIotSerializerError_t prxSerializeNetwork( WifiNetworkInfo_t *pxNetworkInfo, uint8_t *pucBuffer, size_t *pxLength )
{
    AwsIotSerializerEncoderObject_t xContainer = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_STREAM;
    AwsIotSerializerEncoderObject_t xNetworkMap = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;
    AwsIotSerializerScalarData_t xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    size_t xLength = *pxLength;

    xRet = bleMESSAGE_ENCODER.init( &xContainer, pucBuffer, xLength );
    if( xRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        xRet = bleMESSAGE_ENCODER.openContainer( &xContainer, &xNetworkMap, wifiProvNUM_NETWORK_INFO_MESG_PARAMS );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pxNetworkInfo->xStatus;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiProvSTATUS_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_TEXT_STRING;
        xValue.value.pString = ( uint8_t * ) pxNetworkInfo->pcSSID;
        xValue.value.stringLength = pxNetworkInfo->xSSIDLength;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiProvSSID_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_BYTE_STRING;
        xValue.value.pString = ( uint8_t * ) pxNetworkInfo->pucBSSID;
        xValue.value.stringLength = pxNetworkInfo->xBSSIDLength;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiProvBSSID_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pxNetworkInfo->xSecurity;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiProvKEY_MGMT_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pxNetworkInfo->ucHidden;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiProvHIDDEN_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pxNetworkInfo->cRSSI;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiRSSI_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pxNetworkInfo->ucConnected;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiProvCONNECTED_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = pxNetworkInfo->sSavedIdx;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xNetworkMap, wifiProvINDEX_KEY, xValue );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xRet = bleMESSAGE_ENCODER.closeContainer( &xContainer, &xNetworkMap );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        if( pucBuffer == NULL )
        {
            *pxLength = bleMESSAGE_ENCODER.getExtraBufferSizeNeeded( &xContainer );
        }
        else
        {
            *pxLength = bleMESSAGE_ENCODER.getEncodedSize( &xContainer, pucBuffer );
        }

        bleMESSAGE_ENCODER.destroy( &xContainer );
        xRet = AWS_IOT_SERIALIZER_SUCCESS;
    }

    return xRet;
}

static AwsIotSerializerError_t prxSerializeStatusResponse( WIFIReturnCode_t xStatus, uint8_t* pucBuffer, size_t* pxLength )
{
    AwsIotSerializerEncoderObject_t xContainer = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_STREAM;
    AwsIotSerializerEncoderObject_t xResponseMap = AWS_IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;
    AwsIotSerializerScalarData_t xValue = { 0 };
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    size_t xLength = *pxLength;

    xRet = bleMESSAGE_ENCODER.init( &xContainer, pucBuffer, xLength );
    if( xRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        xRet = bleMESSAGE_ENCODER.openContainer( &xContainer, &xResponseMap, wifiProvNUM_STATUS_MESG_PARAMS );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xValue.type = AWS_IOT_SERIALIZER_SCALAR_SIGNED_INT;
        xValue.value.signedInt = xStatus;
        xRet = bleMESSAGE_ENCODER.appendKeyValue( &xResponseMap, wifiProvSTATUS_KEY, xValue );
    }
    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        xRet = bleMESSAGE_ENCODER.closeContainer( &xContainer, &xResponseMap );
    }

    if( IS_VALID_SERIALIZER_RET( xRet, pucBuffer ) )
    {
        if( pucBuffer == NULL )
        {
            *pxLength = bleMESSAGE_ENCODER.getExtraBufferSizeNeeded( &xContainer );
        }
        else
        {
            *pxLength = bleMESSAGE_ENCODER.getEncodedSize( &xContainer, pucBuffer );
        }

        bleMESSAGE_ENCODER.destroy( &xContainer );

        xRet = AWS_IOT_SERIALIZER_SUCCESS;
    }

    return xRet;
}


/*-----------------------------------------------------------*/

void prvSendStatusResponse( WifiProvCharacteristic_t xCharacteristic,
                            WIFIReturnCode_t xStatus )
{

    uint8_t *pucBuffer = NULL;
    size_t xMesgLen;
    AwsIotSerializerError_t xRet = AWS_IOT_SERIALIZER_SUCCESS;
    xRet = prxSerializeStatusResponse( xStatus, NULL, &xMesgLen );
    if( xRet == AWS_IOT_SERIALIZER_SUCCESS )
    {
        pucBuffer = pvPortMalloc( xMesgLen );
        if( pucBuffer != NULL )
        {
            xRet = prxSerializeStatusResponse( xStatus, pucBuffer, &xMesgLen );
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

void prvSendResponse( WifiProvCharacteristic_t xCharacteristic, uint8_t* pucData, size_t xLen )
{
	BLEAttributeData_t xAttrData = { 0 };
	BLEEventResponse_t xResp = { 0 };

	xAttrData.xHandle = ATTR_HANDLE( &xWifiProvService, xCharacteristic );
	xAttrData.xUuid = ATTR_UUID( &xWifiProvService, xCharacteristic );
	xResp.xAttrDataOffset = 0;
	xResp.pxAttrData = &xAttrData;
	xResp.xRspErrorStatus = eBTRspErrorNone;

	xAttrData.pucData = pucData;
	xAttrData.xSize = xLen;

	( void ) BLE_SendIndication( &xResp, xWifiProvService.usBLEConnId, false );
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t prvAppendNetwork( WIFINetworkProfile_t * pxProfile )
{
    WIFIReturnCode_t xRet;
    uint16_t usIdx;

    xRet = WIFI_NetworkAdd( pxProfile, &usIdx );

    if( xRet == eWiFiSuccess )
    {
        xWifiProvService.usNumNetworks++;
        xWifiProvService.sConnectedIdx++;
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
        xWifiProvService.usNumNetworks--;

        /* Shift the priority for connected network */
        if( usIndex < xWifiProvService.sConnectedIdx )
        {
            xWifiProvService.sConnectedIdx--;
        }
        else if( usIndex == xWifiProvService.sConnectedIdx )
        {
            xWifiProvService.sConnectedIdx = wifiProvINVALID_NETWORK_INDEX;
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
    		xWifiProvService.sConnectedIdx = 0;
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
    		xWifiProvService.sConnectedIdx = usIndex;
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

static void prvSendSavedNetwork( WIFINetworkProfile_t *pxSavedNetwork, uint16_t usIdx )
{
    WifiNetworkInfo_t xNetworkInfo = NETWORK_INFO_DEFAULT_PARAMS;
    uint8_t *pucMessage = NULL;
    size_t xMessageLen = 0;
    AwsIotSerializerError_t xSerializerRet;

    xNetworkInfo.pcSSID = pxSavedNetwork->cSSID;
    xNetworkInfo.xSSIDLength = pxSavedNetwork->ucSSIDLength;
    xNetworkInfo.pucBSSID = pxSavedNetwork->ucBSSID;
    xNetworkInfo.xBSSIDLength = wificonfigMAX_BSSID_LEN;
    xNetworkInfo.ucConnected = ( xWifiProvService.sConnectedIdx == usIdx ) ? 1 : 0;
    xNetworkInfo.sSavedIdx = ( int32_t ) usIdx;

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
        prvSendResponse( eListNetworkChar, pucMessage, xMessageLen );
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

static void prvSendScanNetwork( WIFIScanResult_t *pxScanNetwork )
{
    WifiNetworkInfo_t xNetworkInfo = NETWORK_INFO_DEFAULT_PARAMS;
    uint8_t *pucMessage = NULL;
    size_t xMessageLen = 0;
    AwsIotSerializerError_t xSerializerRet;

    xNetworkInfo.pcSSID = pxScanNetwork->cSSID;
    xNetworkInfo.xSSIDLength = strlen( pxScanNetwork->cSSID );
    xNetworkInfo.pucBSSID = pxScanNetwork->ucBSSID;
    xNetworkInfo.xBSSIDLength = wificonfigMAX_BSSID_LEN;
    xNetworkInfo.cRSSI = pxScanNetwork->cRSSI;
    xNetworkInfo.ucHidden = pxScanNetwork->ucHidden;
    xNetworkInfo.xSecurity = pxScanNetwork->xSecurity;

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
        prvSendResponse( eListNetworkChar, pucMessage, xMessageLen );
    }
    else
    {
        configPRINTF(( "Failed to send network profile, SSID:%s\n",
                pxScanNetwork->cSSID ));
    }

    if( pucMessage != NULL )
    {
        vPortFree( pucMessage );
    }
}
/*-----------------------------------------------------------*/

void prvListNetworkTask( void * pvParams )
{
    ListNetworkRequest_t * pxListNetworReq = ( ListNetworkRequest_t * ) pvParams;
    WIFIScanResult_t xScanResults[ pxListNetworReq->sMaxNetworks ];
    WIFINetworkProfile_t xProfile;
    uint16_t usIdx;
    WIFIReturnCode_t xWifiRet;

    if( xSemaphoreTake( xWifiProvService.xLock, portMAX_DELAY ) == pdPASS )
    {
    	for( usIdx = 0; usIdx < xWifiProvService.usNumNetworks; usIdx++ )
    	{
    	    xWifiRet = prvGetSavedNetwork( usIdx, &xProfile );
    		if( xWifiRet == eWiFiSuccess )
    		{
    		    prvSendSavedNetwork( &xProfile, usIdx );
    		}
    	}

    	memset( xScanResults, 0x00, sizeof( WIFIScanResult_t ) * pxListNetworReq->sMaxNetworks );
    	xWifiRet = WIFI_Scan( xScanResults, pxListNetworReq->sMaxNetworks );
    	if( xWifiRet == eWiFiSuccess )
    	{
    		for( usIdx = 0; usIdx < pxListNetworReq->sMaxNetworks; usIdx++ )
    		{
    			if( strlen( xScanResults[ usIdx ].cSSID ) > 0 )
    			{
    			    prvSendScanNetwork( &xScanResults[ usIdx ] );
    			}
    		}
    	}
    	else
    	{
    		prvSendStatusResponse( eListNetworkChar, xWifiRet );
    	}

    	xSemaphoreGive( xWifiProvService.xLock );
    }

    vPortFree( pxListNetworReq );
    vTaskDelete( NULL );
}

/*-----------------------------------------------------------*/

void prvAddNetworkTask( void * pvParams )
{
    WIFIReturnCode_t xRet = eWiFiFailure;
    AddNetworkRequest_t * pxAddNetworkReq = ( AddNetworkRequest_t * ) pvParams;

    prvClearEvent( eWIFIPROVConnect );
    if( xSemaphoreTake( xWifiProvService.xLock, portMAX_DELAY ) == pdPASS )
    {
        if( pxAddNetworkReq->sSavedIdx != wifiProvINVALID_NETWORK_INDEX )
        {
        	xRet = prvConnectSavedNetwork( pxAddNetworkReq->sSavedIdx );
        }
        else
        {
        	xRet = prvAddNewNetwork( &pxAddNetworkReq->xNetwork );
        }
        xSemaphoreGive( xWifiProvService.xLock );
    }

    if( xRet == eWiFiSuccess )
    {
        prvSetEvent( eWIFIPROVConnected );
        xWifiProvService.usNextConnectIdx = xWifiProvService.sConnectedIdx;
    }
    else
    {
    	prvSetEvent( eWIFIPROVConnect );
    }

    prvSendStatusResponse( eSaveNetworkChar, xRet );
    vPortFree( pxAddNetworkReq );
    vTaskDelete( NULL );
}



/*-----------------------------------------------------------*/

void prvDeleteNetworkTask( void * pvParams )
{
    WIFIReturnCode_t xRet = eWiFiFailure;
    DeleteNetworkRequest_t * pxDeleteNetworkReq = ( DeleteNetworkRequest_t * ) pvParams;


    if( xSemaphoreTake( xWifiProvService.xLock, portMAX_DELAY ) == pdPASS )
    {
        xRet = prvPopNetwork( pxDeleteNetworkReq->sIdx, NULL );

        if( xRet == eWiFiSuccess )
        {
            if( xWifiProvService.sConnectedIdx == wifiProvINVALID_NETWORK_INDEX )
            {
                ( void ) WIFI_Disconnect();
                /* Reset the next connection index */
                xWifiProvService.usNextConnectIdx = 0;
                prvSetEvent( eWIFIPROVConnect );
            }
        }
        xSemaphoreGive( xWifiProvService.xLock );
    }

    prvSendStatusResponse( eDeleteNetworkChar, xRet );
    vPortFree( pxDeleteNetworkReq );
    vTaskDelete( NULL );
}



/*-----------------------------------------------------------*/

void prvEditNetworkTask( void * pvParams )
{
    WIFIReturnCode_t xRet = eWiFiFailure;
    EditNetworkRequest_t * pxEditNetworkReq = ( EditNetworkRequest_t * ) pvParams;

    if( xSemaphoreTake( xWifiProvService.xLock, portMAX_DELAY ) == pdPASS )
    {
        xRet = prvMoveNetwork( pxEditNetworkReq->sCurIdx, pxEditNetworkReq->sNewIdx );
        if( xRet == eWiFiSuccess )
        {
            /* Reset the next connect index */
            xWifiProvService.usNextConnectIdx = 0;
        }
        xSemaphoreGive( xWifiProvService.xLock );
    }

    prvSendStatusResponse( eEditNetworkChar, xRet );
    vPortFree( pxEditNetworkReq );
    vTaskDelete( NULL );
}


/*-----------------------------------------------------------*/

void prvConnectAPTask( void * pvParams )
{
    WIFIReturnCode_t xConnectionStatus;
    TickType_t xDelay = pdMS_TO_TICKS( wifiProvSAVED_NETWORKS_CONNECTION_INTERVAL_MS );

    ( void ) pvParams;

    for( ; ; )
    {
        if( prvWaitForEvent( eWIFIPROVConnect, portMAX_DELAY ) == pdTRUE )
        {
        	xConnectionStatus = eWiFiFailure;
            if( xSemaphoreTake( xWifiProvService.xLock, portMAX_DELAY ) == pdPASS )
            {
            	if( ( xWifiProvService.sConnectedIdx == wifiProvINVALID_NETWORK_INDEX )
            			&& ( xWifiProvService.usNumNetworks > 0 ) )
            	{
            		xConnectionStatus = prvConnectSavedNetwork( xWifiProvService.usNextConnectIdx );
            		if( xConnectionStatus == eWiFiSuccess )
            		{
            			prvSetEvent( eWIFIPROVConnected );
            			xWifiProvService.usNextConnectIdx = xWifiProvService.sConnectedIdx;
            		}
            		else
            		{
            			xWifiProvService.usNextConnectIdx = ( xWifiProvService.usNextConnectIdx + 1 ) % xWifiProvService.usNumNetworks;
            		}
            	}
                xSemaphoreGive( xWifiProvService.xLock );


                if( xConnectionStatus != eWiFiSuccess )
                {
                	vTaskDelay( xDelay );
                }

            }

        }
    }
}

/*-----------------------------------------------------------*/

static void prvClearEvent( WifiProvEvent_t xEvent )
{
    ( void ) xEventGroupClearBits( xWifiProvService.xEventGroup, xEvent );
}

/*-----------------------------------------------------------*/

static void prvSetEvent( WifiProvEvent_t xEvent )
{
    ( void ) xEventGroupClearBits( xWifiProvService.xEventGroup, ALL_EVENTS );
    ( void ) xEventGroupSetBits( xWifiProvService.xEventGroup, xEvent );
}

/*-----------------------------------------------------------*/

static BaseType_t prvWaitForEvent( WifiProvEvent_t xEvent,
                                          TickType_t xTimeout )
{
    EventBits_t xSetBits = xEventGroupWaitBits( xWifiProvService.xEventGroup, ( xEvent | eWIFIPROVFailed ), pdFALSE, pdFALSE, xTimeout );

    return( !( xSetBits & eWIFIPROVFailed ) && ( xSetBits & xEvent ) );
}

/*-----------------------------------------------------------*/

BaseType_t WIFI_PROVISION_Init( void )
{
    BaseType_t xStatus = pdTRUE;

    if( xWifiProvService.xInit == pdFALSE )
    {
        xWifiProvService.xEventGroup = xEventGroupCreate();

        if( xWifiProvService.xEventGroup != NULL )
        {
        	( void ) xEventGroupClearBits( xWifiProvService.xEventGroup, ALL_EVENTS );
        }
        else
        {
            xStatus = pdFALSE;
        }

        if( xStatus == pdTRUE )
        {
            xWifiProvService.xLock = xSemaphoreCreateMutex();

            if( xWifiProvService.xLock != NULL )
            {
                xSemaphoreGive( xWifiProvService.xLock );
            }
            else
            {
                xStatus = pdFALSE;
            }
        }


        if( xStatus == pdTRUE )
        {
            xStatus = prxInitGATTService();
        }

        if( xStatus == pdTRUE )
        {
            xWifiProvService.xInit = pdTRUE;
            xStatus = xTaskCreate(
                prvConnectAPTask,
                "WiFiProvConnect",
                configMINIMAL_STACK_SIZE * 4,
                NULL,
				wifiProvCONNECT_AP_TASK_PRIORITY,
                &xWifiProvService.xConnectTask );
        }
    }
    else
    {
        xStatus = pdFALSE;
    }

    return xStatus;
}

/*-----------------------------------------------------------*/

BaseType_t WIFI_PROVISION_Start( void )
{
    BaseType_t xRet;

    /* Initialize number of wifi networks */
    xWifiProvService.usNumNetworks = prvGetNumSavedNetworks();
    xWifiProvService.sConnectedIdx = wifiProvINVALID_NETWORK_INDEX;
    xWifiProvService.usNextConnectIdx = 0;

    if( BLE_StartService( xWifiProvService.pxGattService, vServiceStartedCb ) == eBTStatusSuccess )
    {
        xRet = prvWaitForEvent( eWIFIPROVStarted, portMAX_DELAY );
    }
    else
    {
        xRet = pdFALSE;
    }

    if( xWifiProvService.usNumNetworks > 0 )
    {
        prvSetEvent( eWIFIPROVConnect );
    }

    return xRet;
}

/*-----------------------------------------------------------*/

BaseType_t WIFI_PROVISION_IsConnected( TickType_t xWaitTicks )
{
    BaseType_t xIsConnected = WIFI_IsConnected();

    if( xIsConnected == pdFALSE )
    {
        xIsConnected = prvWaitForEvent( eWIFIPROVConnected, xWaitTicks );
    }

    return xIsConnected;
}

/*-----------------------------------------------------------*/

BaseType_t WIFI_PROVISION_GetConnectedNetwork( WIFINetworkProfile_t * pxNetwork )
{
    BaseType_t xRet = pdFALSE;

    if( ( WIFI_IsConnected() == pdTRUE ) && ( pxNetwork != NULL ) )
    {
        if( prvGetSavedNetwork( xWifiProvService.sConnectedIdx, pxNetwork ) == eWiFiSuccess )
        {
            xRet = pdTRUE;
        }
    }

    return xRet;
}

/*-----------------------------------------------------------*/

BaseType_t WIFI_PROVISION_Stop( void )
{
    BLE_StopService( xWifiProvService.pxGattService, vServiceStoppedCb );
    return prvWaitForEvent( eWIFIPROVStopped, portMAX_DELAY );
}

/*-----------------------------------------------------------*/

BaseType_t WIFI_PROVISION_Delete( void )
{
	BaseType_t xRet = pdFALSE;

    if( BLE_DeleteService( xWifiProvService.pxGattService, vServiceDeletedCb ) == eBTStatusSuccess )
    {
    	xRet = prvWaitForEvent( eWIFIPROVDeleted, portMAX_DELAY );
    }
    if( xRet == pdTRUE )
    {
    	if( xWifiProvService.xConnectTask != NULL )
    	{
    		vTaskDelete( xWifiProvService.xConnectTask );
    	}

    	if( xWifiProvService.xLock != NULL )
    	{
    		vSemaphoreDelete( xWifiProvService.xLock );
    	}

    	if( xWifiProvService.xEventGroup != NULL )
    	{
    		vEventGroupDelete( xWifiProvService.xEventGroup );
    	}

    	memset( &xWifiProvService, 0x00, sizeof( WifiProvService_t ) );
    }

    return xRet;

}

/* Provide access to private members for testing. */
#ifdef AMAZON_FREERTOS_ENABLE_UNIT_TESTS
    #include "aws_ble_wifi_prov_test_access_define.h"
#endif
