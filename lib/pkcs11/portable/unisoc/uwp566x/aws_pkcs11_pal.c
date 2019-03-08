/*
 * Amazon FreeRTOS PKCS #11 PAL V1.0.0
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
 * @file aws_pkcs11_pal.c
 * @brief Device specific helpers for PKCS11 Interface.
 */

/* Amazon FreeRTOS Includes. */
#include "aws_pkcs11.h"
#include "FreeRTOS.h"

/* C runtime includes. */
#include <stdio.h>
#include <string.h>

/* UWP FLASH driver */
#include "hal_ramfunc.h"

/*-----------------------------------------------------------*/

extern void vLoggingPRINT(const char *pcFormat, ... );

/*-----------------------------------------------------------*/

#define UWP_FLASH_SECTOR_SIZE        (0x1000)
#define UWP_FLASH_BASE               (0x02000000)
#define UWP_FLASH_FILE_ENTRY_OFFSET  (0x30000)
#define UWP_FLASH_FILE_SIZE          UWP_FLASH_SECTOR_SIZE
#define UWP_FLASH_FILE_OFFSET        (UWP_FLASH_FILE_ENTRY_OFFSET + UWP_FLASH_FILE_SIZE)

/*-----------------------------------------------------------*/

#define pkcs11palFILE_NAME_CLIENT_CERTIFICATE    "FreeRTOS_P11_Certificate.dat"
#define pkcs11palFILE_NAME_KEY                   "FreeRTOS_P11_Key.dat"
#define pkcs11palFILE_CODE_SIGN_PUBLIC_KEY       "FreeRTOS_P11_CodeSignKey.dat"

/*-----------------------------------------------------------*/

enum eObjectHandles
{
    eInvalidHandle = 0, /* According to PKCS #11 spec, 0 is never a valid object handle. */
    eAwsDevicePrivateKey = 1,
    eAwsDevicePublicKey,
    eAwsDeviceCertificate,
    eAwsCodeSigningKey
};

/*-----------------------------------------------------------*/

typedef struct {
    char pcLabel[64];
    char pcFileName[64];
	CK_OBJECT_HANDLE xHandle;
	CK_BBOOL xIsPrivate;
    uint32_t ulFileAddrOffset;
	uint32_t ulFileLen;
} xObjectFileEntry;

/*-----------------------------------------------------------*/

static xObjectFileEntry xObjectFileDict[] = {
    {
        pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
        pkcs11palFILE_NAME_CLIENT_CERTIFICATE,
        eAwsDeviceCertificate,
        CK_FALSE,
		UWP_FLASH_FILE_OFFSET,              /* offset   192K     */
		UWP_FLASH_FILE_SIZE                 /*      4K           */
    },
    {
        pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS,
        pkcs11palFILE_NAME_KEY,
        eAwsDevicePrivateKey,
        CK_TRUE,
		UWP_FLASH_FILE_OFFSET + UWP_FLASH_FILE_SIZE,
        0x1000
    },
    {
        pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS,
        pkcs11palFILE_NAME_KEY,
        eAwsDevicePublicKey,
        CK_FALSE,
		UWP_FLASH_FILE_OFFSET + UWP_FLASH_FILE_SIZE * 2,
		UWP_FLASH_FILE_SIZE
    },
    {
        pkcs11configLABEL_CODE_VERIFICATION_KEY,
        pkcs11palFILE_CODE_SIGN_PUBLIC_KEY,
        eAwsCodeSigningKey,
        CK_FALSE,
		UWP_FLASH_FILE_OFFSET + UWP_FLASH_FILE_SIZE * 3,
		UWP_FLASH_FILE_SIZE
    }
};

/*-----------------------------------------------------------*/

/*
*
*@bref  Find a file entry according to label.
*
*/

static void prvLabelToFileEntryHandle( uint8_t * pcLabel,
                               xObjectFileEntry **ppxFileEntry,
                               CK_OBJECT_HANDLE_PTR pHandle )
{
    if( pcLabel != NULL )
    {
        /* Translate from the PKCS#11 label to local storage file name. */
        for(int i = 0; i < (sizeof(xObjectFileDict) / sizeof(xObjectFileEntry)); i ++){
            if( 0 == strcmp((const char *)pcLabel, (const char *)xObjectFileDict[i].pcLabel) ){
                *ppxFileEntry = &(xObjectFileDict[i]);
				*pHandle = xObjectFileDict[i].xHandle;
				return;
			}
		}
    }
	
    *ppxFileEntry = NULL;
	*pHandle = eInvalidHandle;
}

/*
*
*@bref  Find a file entry according to handle.
*
*/

static void prvHandleToFileEntry( CK_OBJECT_HANDLE xHandle,
                               xObjectFileEntry **ppxFileEntry )
{
    if( xHandle != eInvalidHandle )
    {
        /* Translate from the PKCS#11 label to local storage file name. */
        for(int i = 0; i < (sizeof(xObjectFileDict) / sizeof(xObjectFileEntry)); i ++){
            if( xObjectFileDict[i].xHandle == xHandle ){
                *ppxFileEntry = &(xObjectFileDict[i]);
				return;
			}
		}
    }

    *ppxFileEntry = NULL;
}

/**
* @brief Writes a file to local storage.
*
* Port-specific file write for crytographic information.
*
* @param[in] pxLabel       Label of the object to be saved.
* @param[in] pucData       Data buffer to be written to file
* @param[in] ulDataSize    Size (in bytes) of data to be saved.
*
* @return The file handle of the object that was stored.
*/
CK_OBJECT_HANDLE PKCS11_PAL_SaveObject( CK_ATTRIBUTE_PTR pxLabel,
    uint8_t * pucData,
    uint32_t ulDataSize )
{
    CK_OBJECT_HANDLE xHandle = eInvalidHandle;
	xObjectFileEntry * pxFileEntry = NULL;

	prvLabelToFileEntryHandle( pxLabel->pValue,
                              &pxFileEntry,
                              &xHandle );

    if( pxFileEntry != NULL )
    {
    	void *pvBufToWrite = pvPortMalloc(UWP_FLASH_SECTOR_SIZE);
    	if( pvBufToWrite == NULL ){
            vLoggingPRINT("malloc fiald\r\n");
            return eInvalidHandle;
    	}
    	memset(pvBufToWrite, 0, ulDataSize);
    	memcpy(pvBufToWrite, (const void *)(UWP_FLASH_BASE + pxFileEntry->ulFileAddrOffset), pxFileEntry->ulFileLen);
    	pxFileEntry->ulFileLen = ulDataSize;
    	memcpy(pvBufToWrite, pucData, ulDataSize);

    	if( flash_uwp_write_protection(false) != 0 ){
    		vLoggingPRINT("flash not avaible\r\n");
    		return eInvalidHandle;
    	}

    	if( flash_uwp_erase(pxFileEntry->ulFileLen, UWP_FLASH_SECTOR_SIZE) != 0 ){
    		vLoggingPRINT("flash file erase failed\r\n");
    		return eInvalidHandle;
    	}

    	if( flash_uwp_write(pxFileEntry->ulFileAddrOffset, pvBufToWrite, ulDataSize) != 0 ){
    		vLoggingPRINT("flash file write failed\r\n");
    		return eInvalidHandle;
    	}

    	if( flash_uwp_erase(UWP_FLASH_FILE_ENTRY_OFFSET, UWP_FLASH_SECTOR_SIZE) != 0 ){
    		vLoggingPRINT("flash entry erase failed\r\n");
    		return eInvalidHandle;
    	}

    	if( flash_uwp_write(UWP_FLASH_FILE_ENTRY_OFFSET, xObjectFileDict, sizeof(xObjectFileDict)) != 0 ){
    		vLoggingPRINT("flash entry write failed\r\n");
    		return eInvalidHandle;
    	}

    	vPortFree(pvBufToWrite);
    }

    return xHandle;
}

/**
* @brief Translates a PKCS #11 label into an object handle.
*
* Port-specific object handle retrieval.
*
*
* @param[in] pLabel         Pointer to the label of the object
*                           who's handle should be found.
* @param[in] usLength       The length of the label, in bytes.
*
* @return The object handle if operation was successful.
* Returns eInvalidHandle if unsuccessful.
*/
CK_OBJECT_HANDLE PKCS11_PAL_FindObject( uint8_t * pLabel,
    uint8_t usLength )
{
	(void) usLength;
	xObjectFileEntry * pxFileEntry = NULL;
    CK_OBJECT_HANDLE xHandle = eInvalidHandle;
    prvLabelToFileEntryHandle(pLabel, &pxFileEntry, &xHandle);
    return xHandle;
}

/**
* @brief Gets the value of an object in storage, by handle.
*
* Port-specific file access for cryptographic information.
*
* This call dynamically allocates the buffer which object value
* data is copied into.  PKCS11_PAL_GetObjectValueCleanup()
* should be called after each use to free the dynamically allocated
* buffer.
*
* @sa PKCS11_PAL_GetObjectValueCleanup
*
* @param[in] pcFileName    The name of the file to be read.
* @param[out] ppucData     Pointer to buffer for file data.
* @param[out] pulDataSize  Size (in bytes) of data located in file.
* @param[out] pIsPrivate   Boolean indicating if value is private (CK_TRUE)
*                          or exportable (CK_FALSE)
*
* @return CKR_OK if operation was successful.  CKR_KEY_HANDLE_INVALID if
* no such object handle was found, CKR_DEVICE_MEMORY if memory for
* buffer could not be allocated, CKR_FUNCTION_FAILED for device driver
* error.
*/
CK_RV PKCS11_PAL_GetObjectValue( CK_OBJECT_HANDLE xHandle,
    uint8_t ** ppucData,
    uint32_t * pulDataSize,
    CK_BBOOL * pIsPrivate )
{
    xObjectFileEntry *pxFileEntry = NULL;

	prvHandleToFileEntry( xHandle, &pxFileEntry);
	if( pxFileEntry == NULL)
		return CKR_KEY_HANDLE_INVALID;

	*ppucData = (uint8_t *)(pxFileEntry->ulFileAddrOffset + UWP_FLASH_BASE);
	*pulDataSize = (uint32_t) pxFileEntry->ulFileLen;
	*pIsPrivate = pxFileEntry->xIsPrivate;
    return CKR_OK;
}


/**
* @brief Cleanup after PKCS11_GetObjectValue().
*
* @param[in] pucData       The buffer to free.
*                          (*ppucData from PKCS11_PAL_GetObjectValue())
* @param[in] ulDataSize    The length of the buffer to free.
*                          (*pulDataSize from PKCS11_PAL_GetObjectValue())
*/
void PKCS11_PAL_GetObjectValueCleanup( uint8_t * pucData,
    uint32_t ulDataSize )
{
	(void) pucData;
	(void) ulDataSize;
}

/*-----------------------------------------------------------*/

int mbedtls_hardware_poll( void * data,
                           unsigned char * output,
                           size_t len,
                           size_t * olen )
{
    /* FIX ME. */
    return 0;
}
