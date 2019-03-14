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
#include <stdlib.h>

/* UWP FLASH driver */
#include "hal_ramfunc.h"

#include "task.h"

//#define USE_OFFLOAD_SSL
#ifdef USE_OFFLOAD_SSL
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#endif

/*-----------------------------------------------------------*/

extern void vLoggingPrint( const char * pcMessage );
extern void vLoggingPrintf(const char *pcFormat, ... );

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
    uint8_t ucUpdateFromFalsh; /* indicate whether read content from flash after power up */
    char pcLabel[64];
    char pcFileName[64];
	CK_OBJECT_HANDLE xHandle;
	CK_BBOOL xIsPrivate;
    uint32_t ulFileAddrOffset;
	uint32_t ulFileLen;
} xObjectFileEntry;

/*-----------------------------------------------------------*/

static xObjectFileEntry xObjectFileDict[4] = {
    {
    	0,
        pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
        pkcs11palFILE_NAME_CLIENT_CERTIFICATE,
        eAwsDeviceCertificate,
        CK_FALSE,
		UWP_FLASH_FILE_OFFSET,
		UWP_FLASH_FILE_SIZE
    },
    {
    	0,
        pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS,
        pkcs11palFILE_NAME_KEY,
        eAwsDevicePrivateKey,
        CK_TRUE,
		UWP_FLASH_FILE_OFFSET + UWP_FLASH_FILE_SIZE,
		UWP_FLASH_FILE_SIZE
    },
    {
    	0,
        pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS,
        pkcs11palFILE_NAME_KEY,
        eAwsDevicePublicKey,
        CK_FALSE,
		UWP_FLASH_FILE_OFFSET + UWP_FLASH_FILE_SIZE * 2,
		UWP_FLASH_FILE_SIZE
    },
    {
    	0,
        pkcs11configLABEL_CODE_VERIFICATION_KEY,
        pkcs11palFILE_CODE_SIGN_PUBLIC_KEY,
        eAwsCodeSigningKey,
        CK_FALSE,
		UWP_FLASH_FILE_OFFSET + UWP_FLASH_FILE_SIZE * 3,
		UWP_FLASH_FILE_SIZE
    }
};

/*  alloc flash memory for xObjectFileDict  */
static xObjectFileEntry xObjectFileDictFlash[4] __attribute__ ( (section(".CERTIFICATE_ENTRY")) );

/*-----------------------------------------------------------*/

#ifdef USE_OFFLOAD_SSL

 #define BEGIN_EC_PRIVATE_KEY     "-----BEGIN EC PRIVATE KEY-----\n"
 #define END_EC_PRIVATE_KEY       "-----END EC PRIVATE KEY-----\n"
 #define BEGIN_RSA_PRIVATE_KEY    "-----BEGIN RSA PRIVATE KEY-----\n"
 #define END_RSA_PRIVATE_KEY      "-----END RSA PRIVATE KEY-----\n"
 #define BEGIN_CERTIFICATE        "-----BEGIN CERTIFICATE-----\n"
 #define END_CERTIFICATE          "-----END CERTIFICATE-----\n"
 #define NUM_CHAR_PER_PEM_LINE    64

/*
* @brief Converts DER objects to PEM objects.
*
* \note Only elliptic curve and RSA private keys are supported.
*
* \param[in]   pDerBuffer      A pointer the DER object.
* \param[in]   xDerLength      The length of the DER object (bytes)
* \param[out]  ppcPemBuffer    A pointer to the buffer that will be allocated
*                              for the PEM object.  This function performs
*                              a memory allocation for this buffer, and the
*                              caller MUST free the buffer.
* \param[out]  pPemLength      Length of the PEM object
* \param[in]   xObjectType     Type of object being converted to PEM.
*                              Valid values are CKO_PRIVATE_KEY and
*                              CKO_CERTIFICATE.
*/
 static CK_RV prvDerToPem( uint8_t * pDerBuffer,
                           size_t xDerLength,
                           char ** ppcPemBuffer,
                           size_t * pPemLength,
                           CK_OBJECT_CLASS xObjectType )
 {
     CK_RV xReturn = CKR_OK;
     size_t xTotalPemLength = 0;
     mbedtls_pk_context pk;
     mbedtls_pk_type_t xKeyType;
     char * pcHeader;
     char * pcFooter;
     unsigned char * pemBodyBuffer;
     uint8_t * pFinalBufferPlaceholder;
     int ulLengthOfContentsCopiedSoFar = 0;
     int ulBytesInLine = 0;
     int ulBytesRemaining;

     if( xObjectType == CKO_PRIVATE_KEY )
     {
         mbedtls_pk_init( &pk );
         /* Parse key. */
         xReturn = mbedtls_pk_parse_key( &pk, pDerBuffer, xDerLength, NULL, 0 );

         if( xReturn != 0 )
         {
             xReturn = CKR_ATTRIBUTE_VALUE_INVALID;
         }

         /* Get key algorithm. */
         xKeyType = mbedtls_pk_get_type( &pk );

         switch( xKeyType )
         {
             case ( MBEDTLS_PK_RSA ):
             case ( MBEDTLS_PK_RSA_ALT ):
                 pcHeader = BEGIN_RSA_PRIVATE_KEY;
                 pcFooter = END_RSA_PRIVATE_KEY;
                 xTotalPemLength = strlen( BEGIN_RSA_PRIVATE_KEY ) + strlen( END_RSA_PRIVATE_KEY );
                 break;

             case ( MBEDTLS_PK_ECKEY ):
             case ( MBEDTLS_PK_ECKEY_DH ):
             case ( MBEDTLS_PK_ECDSA ):
                 pcHeader = BEGIN_EC_PRIVATE_KEY;
                 pcFooter = END_EC_PRIVATE_KEY;
                 xTotalPemLength = strlen( BEGIN_EC_PRIVATE_KEY ) + strlen( END_EC_PRIVATE_KEY );
                 break;

             default:
                 xReturn = CKR_ATTRIBUTE_VALUE_INVALID;
                 break;
         }

         mbedtls_pk_free( &pk );
     }
     else /* Certificate object. */
     {
         pcHeader = BEGIN_CERTIFICATE;
         pcFooter = END_CERTIFICATE;
         xTotalPemLength = strlen( BEGIN_CERTIFICATE ) + strlen( END_CERTIFICATE );
     }

     if( xReturn == CKR_OK )
     {
         /* A PEM object has a header, body, and footer. */

         /*
          * ------- BEGIN SOMETHING --------\n
          * BODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODY\n
          * BODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODY\n
          * BODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODYBODY\n
          * ....
          * ------- END SOMETHING ---------\n
          */

         /* Determine the length of the Base 64 encoded body. */
         xReturn = mbedtls_base64_encode( NULL, 0, pPemLength, pDerBuffer, xDerLength );

         if( xReturn != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL )
         {
             xReturn = CKR_ATTRIBUTE_VALUE_INVALID;
         }
         else
         {
             xReturn = 0;
         }

         if( xReturn == 0 )
         {
             /* Allocate memory for the PEM contents (excluding header, footer, newlines). */
             pemBodyBuffer = pvPortMalloc( *pPemLength );

             if( pemBodyBuffer == NULL )
             {
                 xReturn = CKR_DEVICE_MEMORY;
             }
         }

         if( xReturn == 0 )
         {
             /* Convert the body contents from DER to PEM. */
             xReturn = mbedtls_base64_encode( pemBodyBuffer,
                                              *pPemLength,
                                              pPemLength,
                                              pDerBuffer,
                                              xDerLength );
         }

         if( xReturn == 0 )
         {
             /* Calculate the length required for the entire PEM object. */
             xTotalPemLength += *pPemLength;
             /* Add in space for the newlines. */
             xTotalPemLength += ( *pPemLength ) / NUM_CHAR_PER_PEM_LINE;

             if( ( *pPemLength ) % NUM_CHAR_PER_PEM_LINE != 0 )
             {
                 xTotalPemLength += 1;
             }

             /* Allocate space for the full PEM certificate, including header, footer, and newlines.
              * This space must be freed by the application. */
             *ppcPemBuffer = pvPortMalloc( xTotalPemLength );

             if( *ppcPemBuffer == NULL )
             {
                 xReturn = CKR_DEVICE_MEMORY;
             }
         }

         if( xReturn == 0 )
         {
             /* Copy the header. */
             pFinalBufferPlaceholder = ( uint8_t * ) *ppcPemBuffer;
             memcpy( pFinalBufferPlaceholder, pcHeader, strlen( pcHeader ) );
             pFinalBufferPlaceholder += strlen( pcHeader );

             /* Copy the Base64 encoded contents into the final buffer 64 bytes at a time, adding newlines */
             while( ulLengthOfContentsCopiedSoFar < *pPemLength )
             {
                 ulBytesRemaining = *pPemLength - ulLengthOfContentsCopiedSoFar;
                 ulBytesInLine = ( ulBytesRemaining > NUM_CHAR_PER_PEM_LINE ) ? NUM_CHAR_PER_PEM_LINE : ulBytesRemaining;
                 memcpy( pFinalBufferPlaceholder,
                         pemBodyBuffer + ulLengthOfContentsCopiedSoFar,
                         ulBytesInLine );
                 pFinalBufferPlaceholder += ulBytesInLine;
                 *pFinalBufferPlaceholder = '\n';
                 pFinalBufferPlaceholder++;

                 ulLengthOfContentsCopiedSoFar += ulBytesInLine;
             }
         }

         if( pemBodyBuffer != NULL )
         {
             vPortFree( pemBodyBuffer );
         }

         /* Copy the footer. */
         memcpy( pFinalBufferPlaceholder, pcFooter, strlen( pcFooter ) );

         /* Update the total length of the PEM object returned by the function. */
         *pPemLength = xTotalPemLength;
     }

     return xReturn;
 }

#endif

/*
*
*@bref  get flash stroge address of xObjectFileEntryDict.
*
*/

static void * prvGetAbsolutexObjectFileDictAddr(){
    return (void *)xObjectFileDictFlash;
}

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
    		vLoggingPrint("malloc fiald\r\n");
            return eInvalidHandle;
    	}
    	memset(pvBufToWrite, 0, ulDataSize);
    	memcpy(pvBufToWrite, (const void *)(UWP_FLASH_BASE + pxFileEntry->ulFileAddrOffset), pxFileEntry->ulFileLen);
    	pxFileEntry->ulFileLen = ulDataSize;
    	memcpy(pvBufToWrite, pucData, ulDataSize);

    	if( flash_uwp_write_protection(false) != 0 ){
    		vLoggingPrint("flash not avaible\r\n");
    		return eInvalidHandle;
    	}

    	if( flash_uwp_erase(pxFileEntry->ulFileLen, UWP_FLASH_SECTOR_SIZE) != 0 ){
    		vLoggingPrint("flash file erase failed\r\n");
    		return eInvalidHandle;
    	}

    	if( flash_uwp_write(pxFileEntry->ulFileAddrOffset, pvBufToWrite, ulDataSize) != 0 ){
    		vLoggingPrint("flash file write failed\r\n");
    		flash_uwp_write_protection(true);
    		return eInvalidHandle;
    	}

    	if( flash_uwp_erase(UWP_FLASH_FILE_ENTRY_OFFSET, UWP_FLASH_SECTOR_SIZE) != 0 ){
    		vLoggingPrint("flash entry erase failed\r\n");
    		flash_uwp_write_protection(true);
    		return eInvalidHandle;
    	}

    	if( flash_uwp_write(UWP_FLASH_FILE_ENTRY_OFFSET, xObjectFileDict, sizeof(xObjectFileDict)) != 0 ){
    		vLoggingPrint("flash entry write failed\r\n");
    		flash_uwp_write_protection(true);
    		return eInvalidHandle;
    	}

    	if( flash_uwp_write_protection(true) != 0 ){
    		vLoggingPrint("flash protect failed\r\n");
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
    int copylen = len;
    int size;
    int entropy;
    unsigned seed = xTaskGetTickCount();

    size = sizeof( entropy );
    if(seed != 0)
        srand(seed);

    while( copylen > 0 )
    {
        entropy = rand();
        if( entropy == 0 )
        {
            vLoggingPrintf( "Error RAND is returning 0 at copylen 0x%x \n\r", copylen );
        }

        memcpy( output, &entropy, size < copylen ? size : copylen);
        output += size;
        copylen -= size;
    }

    *olen = len;
    return 0;
}

/*  test case */
void pkcs_self_test(void){
	void * pvObjectAddr = NULL;
	CK_ATTRIBUTE pxLabel = {
	    0,
		(void *)pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
		sizeof(pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS)
	};
	uint8_t *pucData = NULL;
	uint32_t ulDataSize;
	CK_BBOOL IsPrivate;

	vLoggingPrint("pkcs test\r\n");
	pvObjectAddr = prvGetAbsolutexObjectFileDictAddr();
	vLoggingPrintf("flash addr :0x%x\r\n", pvObjectAddr);

	if( PKCS11_PAL_FindObject((uint8_t *)pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
			                      sizeof(pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS)) != eAwsDeviceCertificate )
	{
		vLoggingPrint("test PKCS11_PAL_FindObject failed\r\n");
	}
	else
	{
		vLoggingPrint("test PKCS11_PAL_FindObject success\r\n");
	}

	if( PKCS11_PAL_SaveObject(&pxLabel,
			                     (uint8_t *)"123456789954123", sizeof("123456789954123")) != eAwsDeviceCertificate )
	{
		vLoggingPrint("test PKCS11_PAL_SaveObject failed\r\n");
	}
	else
	{
		vLoggingPrint("test PKCS11_PAL_SaveObject success\r\n");
	}

	if( PKCS11_PAL_GetObjectValue(eAwsDeviceCertificate, &pucData, &ulDataSize, &IsPrivate) != CKR_OK )
	{
		vLoggingPrint("test PKCS11_PAL_GetObjectValue failed\r\n");
	}
	else
	{
        vLoggingPrintf("cert:%s\r\n", (char *)pucData);
	}

}
