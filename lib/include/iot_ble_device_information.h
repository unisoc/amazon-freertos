/*
 * Amazon FreeRTOS
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * @file iot_ble_device_information.h
 * @brief GATT service which exposes Amazon FreeRTOS device information
 */
#ifndef IOT_BLE_DEVICE_INFORMATION_H_
#define IOT_BLE_DEVICE_INFORMATION_H_

#include "iot_ble.h"
#include "iot_ble_config.h"


/**
 * @brief Service, characteristic and descriptor UUIDS for Device information Service
 */
#define IOT_BLE_DEVICE_INFO_CHAR_UUID_BASE          IOT_BLE_DEVICE_INFO_SERVICE_UUID
#define IOT_BLE_DEVICE_INFO_VERSION_UUID       { 0x01, 0xFF, IOT_BLE_DEVICE_INFO_SERVICE_UUID_MASK }
#define IOT_BLE_DEVICE_INFO_BROKER_ENDPOINT_UUID    { 0x02, 0xFF, IOT_BLE_DEVICE_INFO_SERVICE_UUID_MASK }
#define IOT_BLE_DEVICE_INFO_CHAR_MTU_UUID           { 0x03, 0xFF, IOT_BLE_DEVICE_INFO_SERVICE_UUID_MASK } 
#define IOT_BLE_DEVICE_INFO_CLIENT_CHAR_CFG_UUID    0x2902 

/**
 * @brief Number of characteristics, descriptors and included services for Device Information Service
 */
#define IOT_BLE_DEVICE_INFO_MAX_CHARS               3
#define IOT_BLE_DEVICE_INFO_MAX_DESCRS              1
#define IOT_BLE_DEVICE_INFO_MAX_INC_SVCS            0

/**
 * @brief Characteristics for Device Inforamtion Service.
 */
typedef enum
{
    IOT_BLE_DEVICE_INFO_VERSION_CHAR = 0,         /**< IOT_BLE_DEVICE_INFO_VERSION_CHAR Exposes the services version for the device */
    IOT_BLE_DEVICE_INFO_MQTT_BROKER_END_POINT_CHAR, /**< IOT_BLE_DEVICE_INFO_MQTT_BROKER_END_POINT_CHAR Exposes the IOT broker endpoint with which the device is provisioned */
    IOT_BLE_DEVICE_INFO_MTU_CHAR,                /**< IOT_BLE_DEVICE_INFO_MTU_CHAR Expose the BLE MTU for the device */
} IotBleDeviceInfoCharacteristic_t;

/**
 * @brief Descriptors for Device Information Service
 */
typedef enum
{
    IOT_BLE_DEVICE_INFO_MTU_CHARDescr = 0, /**< IOT_BLE_DEVICE_INFO_MTU_CHARDescr Client Characteristic configuration descriptor for MTU characteristic */
} IotBleDeviceInfoDescriptor_t;

/**
 *
 * JSON tokens used within the messages exchanged between GATT client and Server.
 *
 */

#define IOT_BLE_DEVICE_INFO_INT_MAX_WIDTH                  ( 6 )
#define IOT_BLE_DEVICE_INFO_MTU_WIDTH            ( IOT_BLE_DEVICE_INFO_INT_MAX_WIDTH )
#define IOT_BLE_DEVICE_INFO_VERSION_WIDTH        ( IOT_BLE_DEVICE_INFO_INT_MAX_WIDTH )
#define IOT_BLE_DEVICE_INFO_MTU                  "mtu"
#define IOT_BLE_DEVICE_INFO_VERSION              "version"
#define IOT_BLE_DEVICE_INFO_BROKER_ENDPOIINT     "brokerEndpoint"

#define IOT_BLE_DEVICE_INFO_JSON_STR( x )    IOT_BLE_DEVICE_INFO_STR( x )
#define IOT_BLE_DEVICE_INFO_STR( x )         # x

/**
 * JSON format for serializing the response payloads
 */
#define IOT_BLE_DEVICE_INFO_MTU_MSG_FORMAT    \
    "{"                             \
    IOT_BLE_DEVICE_INFO_JSON_STR( IOT_BLE_DEVICE_INFO_MTU ) ":%d" \
                              "}"
#define IOT_BLE_DEVICE_INFO_MTU_MSG_LEN               ( sizeof( IOT_BLE_DEVICE_INFO_MTU_MSG_FORMAT ) + IOT_BLE_DEVICE_INFO_MTU_WIDTH )

#define deviceInfoVERSION_MSG_FORMAT                \
    "{"                                             \
    IOT_BLE_DEVICE_INFO_JSON_STR( IOT_BLE_DEVICE_INFO_VERSION      ) ":\"%.*s\""  \
                                     "}"
#define deviceInfoVERSION_MSG_LEN      ( sizeof( deviceInfoVERSION_MSG_FORMAT ) )

#define deviceInfoBROKERENDPOINT_MSG_FORMAT            \
    "{"                                                \
    IOT_BLE_DEVICE_INFO_JSON_STR( IOT_BLE_DEVICE_INFO_BROKER_ENDPOIINT ) ":\"%.*s\"" \
                                           "}"
#define deviceInfoBROKERENDPOINT_MSG_LEN    ( sizeof( deviceInfoBROKERENDPOINT_MSG_FORMAT ) )

/**
 * @brief Structure used for Device Information Service
 */
typedef struct DeviceInfoService
{
    BTService_t * pxBLEService;
    uint16_t usCCFGVal[ IOT_BLE_DEVICE_INFO_MAX_DESCRS ];
    uint16_t usBLEConnId;
    uint16_t usBLEMtu;
} DeviceInfoService_t;

/**
 * @Brief Creates and starts Amazon FreeRTOS device information service
 *
 * @return pdTRUE if the service is initialized successfully, pdFALSE otherwise
 */
extern BaseType_t AFRDeviceInfoSvc_Init( void );


#endif /* IOT_BLE_DEVICE_INFORMATION_H_ */
