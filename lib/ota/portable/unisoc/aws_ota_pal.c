
/* C libraries */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* FreeRTOS header */
#include "FreeRTOS.h"

/* aws ota header */
#include "aws_ota_agent.h"
#include "aws_ota_pal.h"
#include "aws_crypto.h"
#include "aws_pkcs11.h"
#include "aws_ota_codesigner_certificate.h"

/* uwp Flash Driver */
#include "hal_ramfunc.h"

/*************************** struct *****************************/
typedef enum{
    PartitionInvalid = 0,
	PartitionAppRunning,
	PartitionAppOta,
	PartitionModemOta
}PartitionState_t;

typedef struct
{
    uint32_t ulPartitionAddrOffset;
    uint32_t ulLastWriteOffset;
    uint32_t ulPartitionLength;
    uint32_t ulFileLength;
    bool     bImageValid;
    PartitionState_t eState;
    uint8_t  ucPartition;
} uwp_ota_flash_partition_t;

typedef struct
{
    const OTA_FileContext_t * pxCurOTAFileCtx;
    uwp_ota_flash_partition_t partition;
    uint32_t data_write_len;
    bool valid_image;
} uwp_ota_context_t;

/*********************** variable *****************************/
static uwp_ota_context_t xOTACtx;

/********************alloc falsh memory************************/
static uwp_ota_flash_partition_t xPartitionFlash[3] __attribute__ ( (section(".UWP_OTA_INFO")) );

/*******************convenient function***************************/
static inline BaseType_t prvContextValidate( OTA_FileContext_t * C )
{
    return ( (C != NULL) && (C->pucFile == (uint8_t *)&xOTACtx) && (C == xOTACtx.pxCurOTAFileCtx) );
}

/***********************PARAMETERS*********************************/
#define UWP_FLASH_BASE            (0x02000000)
#define UWP_OTA_UPDATE_IMAGE_SIZE (512*1024)
#define UWP_OTA_UPDATA_IMAGE_ADDR (0x002C0000)
#define UWP_FLASH_SECTOR_SIZE     (0x1000)

/********************* function protype ***************************/
OTA_Err_t prvPAL_CheckFileSignature( OTA_FileContext_t * const C );
static CK_RV prvGetCertificate( const char * pcLabelName,
                                uint8_t ** ppucData,
                                uint32_t * pulDataSize );

/*
 * @bref get a partition for app ota use
 */
static int prvGetOTAPartition( uwp_ota_flash_partition_t * pxPartition ){

    for(int i = 0; i < 2; i++) {
        if( xPartitionFlash[i].eState == PartitionAppOta ) {
            memcpy( (void *)pxPartition, (void *)&xPartitionFlash[i], sizeof(uwp_ota_flash_partition_t));
            return 0;
        }
    }

    return -1;
}

/*
 * @bref erase and wirte flash according to flash offset
 * erase a sector and write a sector
 */
static int prvFlashEraseAndWrite(uint8_t *pcData, uint32_t ulAddrOffset, uint32_t ulWriteLen) {

    /* check flash avaliable */
    if( (ulAddrOffset < 0x0) || (ulAddrOffset > 0x400000) ) {
        return -100;
    }

    uint32_t ulAddrWrite = ulAddrOffset;
    uint32_t ulAddrErase;
    uint32_t ulDataToWriteLen = 0;
    uint32_t ulDataLen = ulWriteLen;
    uint32_t ulDataWritePos = 0;
    uint8_t *pucDataWriting = pcData;
    void *   pvWriteBuf = NULL;
    int      iResult = 0;

    pvWriteBuf = pvPortMalloc(UWP_FLASH_SECTOR_SIZE);

    if(pvWriteBuf == NULL){
        return -101;
    }

    do {
        ulAddrErase = ulAddrWrite / UWP_FLASH_SECTOR_SIZE * UWP_FLASH_SECTOR_SIZE;
        ulDataWritePos = ulAddrWrite - ulAddrErase;
        ulDataToWriteLen = (ulDataLen <= (UWP_FLASH_SECTOR_SIZE - ulDataWritePos)) ? ulDataLen : (UWP_FLASH_SECTOR_SIZE - ulDataWritePos);
        memcpy( pvWriteBuf, (void *)ulAddrErase, UWP_FLASH_SECTOR_SIZE);
        memcpy( pvWriteBuf + ulDataWritePos , (void *)pucDataWriting, ulDataToWriteLen);

        if( flash_uwp_write_protection(false) != 0 ){
            vLoggingPrint("flash not avaliable\r\n");
            iResult = -102;
        }

        if( flash_uwp_erase(ulAddrErase, UWP_FLASH_SECTOR_SIZE) != 0 ){
            flash_uwp_write_protection(true);
            vLoggingPrint("flash file erase failed\r\n");
            iResult = -103;
        }

        if( flash_uwp_write(ulAddrErase, pvWriteBuf, UWP_FLASH_SECTOR_SIZE) != 0 ){
            flash_uwp_write_protection(true);
            vLoggingPrint("flash file write failed\r\n");
            iResult = -104;
        }

        if( flash_uwp_write_protection(true) != 0 ){
            vLoggingPrint("flash protect failed\r\n");
            iResult = -105;
        }

         if(iResult != 0){
            vPortFree(pvWriteBuf);
            return iResult;
        }

        ulAddrWrite += ulDataToWriteLen;
        pucDataWriting += ulDataToWriteLen;
        ulDataLen -= ulDataToWriteLen;

    } while (ulDataLen > 0);

    vPortFree(pvWriteBuf);
    return 0;
}

/*
 * @bref update partition information
 */
static int prvUpdatePartitionInfo( uwp_ota_flash_partition_t * pxPartition ){
    return prvFlashEraseAndWrite( (uint8_t *)pxPartition, (uint32_t)&xPartitionFlash[xOTACtx.partition.ucPartition] - UWP_FLASH_BASE,
                                      sizeof(uwp_ota_flash_partition_t));
}

/*
 * @bref erase a partition
 * partition is always 4K aligned
 */
static int prvErasePartition( uwp_ota_flash_partition_t * pxPartition ){

    uint32_t ulEraseAddrOffset = pxPartition->ulPartitionAddrOffset;

    for(ulEraseAddrOffset = 0; ulEraseAddrOffset + UWP_FLASH_SECTOR_SIZE <= pxPartition->ulPartitionLength;
                                      ulEraseAddrOffset += UWP_FLASH_SECTOR_SIZE){
        if(ulEraseAddrOffset % 0x1000 != 0){
            vLoggingPrint("partition addr should be always 4k aligned\r\n");
            return -300;
        }

        if( flash_uwp_write_protection(false) != 0 ){
            vLoggingPrint("flash not avaliable\r\n");
            return -301;
        }

        if( flash_uwp_erase(ulEraseAddrOffset + pxPartition->ulPartitionAddrOffset, UWP_FLASH_SECTOR_SIZE) != 0 ){
            flash_uwp_write_protection(true);
            vLoggingPrint("flash file erase failed\r\n");
            return -302;
        }

        if( flash_uwp_write_protection(true) != 0 ){
            vLoggingPrint("flash protect failed\r\n");
            return -303;
        }
    }

    return 0;
}

/*
 * @bref swap partition information
 * excute before return bootloader
 */
static int prvSwapPartitionInfo(void){

    if( xOTACtx.partition.bImageValid == false ){
        vLoggingPrint("no valid ota image exits\r\n");
        return -200;
    }

    uwp_ota_flash_partition_t xPartitionTemp[3];
    memcpy((void *)xPartitionTemp, (void *)xPartitionFlash, sizeof(xPartitionTemp));
    for(int i = 0; (i != xOTACtx.partition.ucPartition) && (i < 3); i++){
        if( xPartitionTemp[i].eState == PartitionAppRunning ){
            xPartitionTemp[i].eState = PartitionAppOta;
            xPartitionTemp[xOTACtx.partition.ucPartition].eState = PartitionAppRunning;
            return prvFlashEraseAndWrite( (uint8_t *)xPartitionTemp, (uint32_t)xPartitionFlash, sizeof(xPartitionTemp));
        }
    }

    vLoggingPrint("can't identify running image\r\n");
    return -201;
}

/* Abort receiving the specified OTA update by closing the file. */
OTA_Err_t prvPAL_Abort( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_Abort" );

    OTA_Err_t ota_ret = kOTA_Err_FileAbort;

    if( C != NULL ){

        if( prvContextValidate( C ) == pdTRUE ) {
            xOTACtx.pxCurOTAFileCtx = NULL;
            C->pucFile = NULL;
            xOTACtx.partition.bImageValid = false;
            xOTACtx.partition.ulFileLength = 0;
            xOTACtx.partition.ulLastWriteOffset = 0;
            OTA_LOG_L1( "[%s] OK\r\n", OTA_METHOD_NAME );
            ota_ret = kOTA_Err_None;
        }
        else if( C && ( C->pucFile == NULL ) ) {
            OTA_LOG_L1( "[%s] Invalid contex\r\n", OTA_METHOD_NAME );
            ota_ret = kOTA_Err_None;
        }
    }
    else {
        OTA_LOG_L1( "[%s] NULL contex\r\n", OTA_METHOD_NAME );
        ota_ret = kOTA_Err_None;
    }

    return ota_ret;
}

/* Attempt to create a new receive file for the file chunks as they come in. */
OTA_Err_t prvPAL_CreateFileForRx( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_CreateFileForRx" );

    int iResult = 0;

    if( (C != NULL) && (C->ulFileSize <= UWP_OTA_UPDATE_IMAGE_SIZE) ) {
        if( xOTACtx.pxCurOTAFileCtx == NULL ){
            C->pucFile = (uint8_t *)&xOTACtx;
            xOTACtx.pxCurOTAFileCtx = C;
            iResult = prvGetOTAPartition(&(xOTACtx.partition));
            if( iResult != 0 )
                return kOTA_Err_RxFileCreateFailed;
            xOTACtx.partition.bImageValid = false;
            xOTACtx.partition.ulFileLength = 0;
            xOTACtx.partition.ulLastWriteOffset = 0;
            OTA_LOG_L1( "[%s] OK\r\n", OTA_METHOD_NAME );
            return kOTA_Err_None;
        }
        else{
            OTA_LOG_L1( "[%s] ERR another OTA exits\r\n", OTA_METHOD_NAME );
            return kOTA_Err_RxFileCreateFailed;
        }
    }
    else {
        OTA_LOG_L1( "[%s] invalid context\r\n", OTA_METHOD_NAME );
        return kOTA_Err_RxFileCreateFailed;
    }
}

/* Close the specified file. This shall authenticate the file if it is marked as secure. */
OTA_Err_t prvPAL_CloseFile( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_CloseFile" );

    OTA_Err_t eResult = kOTA_Err_None;

    if( prvContextValidate( C ) == pdTRUE )
    {
        if( C->pxSignature != NULL )
        {
            /* Verify the file signature, close the file and return the signature verification result. */
            eResult = prvPAL_CheckFileSignature( C );
        }
        else
        {
            OTA_LOG_L1( "[%s] ERROR - NULL OTA Signature structure.\r\n", OTA_METHOD_NAME );
            eResult = kOTA_Err_SignatureCheckFailed;
        }

        if( eResult == kOTA_Err_None )
        {
            xOTACtx.partition.bImageValid = true;
            /* Close the file. */
            C->pucFile = NULL;
            xOTACtx.pxCurOTAFileCtx = NULL;
            //OTA_LOG_L1( "[%s] %s signature verification passed.\r\n", OTA_METHOD_NAME, pcOTA_JSON_FileSignatureKey );
            OTA_LOG_L1( "[%s] signature verification passed.\r\n", OTA_METHOD_NAME );
        }
        else
        {
            /*OTA_LOG_L1( "[%s] ERROR - Failed to pass %s signature verification: %d.\r\n", OTA_METHOD_NAME,
                        pcOTA_JSON_FileSignatureKey, eResult );*/

            OTA_LOG_L1( "[%s] ERROR - Failed to pass signature verification: %d.\r\n", OTA_METHOD_NAME, eResult );

            /* If we fail to verify the file signature that means the image is not valid. We need to set the image state to aborted. */
            prvPAL_SetPlatformImageState( eOTA_ImageState_Aborted );

        }
    }
    else /* Invalid OTA Context. */
    {
        /* FIXME: Invalid error code for a null file context and file handle. */
        OTA_LOG_L1( "[%s] ERROR - Invalid context.\r\n", OTA_METHOD_NAME );
        eResult = kOTA_Err_FileClose;
    }

    return eResult;
}

/* Write a block of data to the specified file. */
int16_t prvPAL_WriteBlock( OTA_FileContext_t * const C,
                           uint32_t ulOffset,
                           uint8_t * const pacData,
                           uint32_t ulBlockSize )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_WriteBlock" );

    int32_t lResult = 0;

    if( prvContextValidate( C ) == pdTRUE )
    {
    	//OTA_LOG_L1( "[%s]  %x %d %d\r\n", OTA_METHOD_NAME, xOTACtx.partition.ulLastWriteOffset, ulOffset, ulBlockSize );

        if( ((xOTACtx.partition.ulLastWriteOffset + ulBlockSize) <= xOTACtx.partition.ulPartitionLength)
             && (xOTACtx.partition.ulLastWriteOffset == ulOffset) )
        {
            lResult = prvFlashEraseAndWrite( pacData, xOTACtx.partition.ulPartitionAddrOffset + ulOffset, ulBlockSize );

            if( lResult != 0 )
            {
                OTA_LOG_L1( "[%s] ERROR - fwrite failed:%d\r\n", OTA_METHOD_NAME, lResult );
                return -1;
            }

            xOTACtx.partition.ulFileLength += ulBlockSize;
            xOTACtx.partition.ulLastWriteOffset += ulBlockSize;
            lResult = ulBlockSize;
        }
        else
        {
            OTA_LOG_L1( "[%s] ERROR - fseek failed\r\n", OTA_METHOD_NAME );
            return -1;
        }
    }
    else /* Invalid context or file pointer provided. */
    {
        OTA_LOG_L1( "[%s] ERROR - Invalid context.\r\n", OTA_METHOD_NAME );
        lResult = -1; /*TODO: Need a negative error code from the PAL here. */
    }

    return ( int16_t ) lResult;
}

/* Activates or launches the new firmware image. */
OTA_Err_t prvPAL_ActivateNewImage( void )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_ActivateNewImage" );

    prvPAL_ResetDevice();

    OTA_LOG_L1( "[%s] ERROR - you should nerver see this.\r\n", OTA_METHOD_NAME );

    for(;;); // wait reset

    return kOTA_Err_None;
}

/* Platform specific handling of the last transferred OTA file.
 * Commit the image if the state == eOTA_ImageState_Accepted.
 */
OTA_Err_t prvPAL_SetPlatformImageState( OTA_ImageState_t eState )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_SetPlatformImageState" );

    OTA_Err_t eResult = kOTA_Err_Uninitialized;
    int iResult = 0;

    /* do not distinguish self test mode */
    if( eState == eOTA_ImageState_Accepted )
    {
        /* check the image valid or not*/
        if( xOTACtx.partition.bImageValid != true)
        	return kOTA_Err_CommitFailed;

        iResult = prvUpdatePartitionInfo(&xOTACtx.partition);
        if( iResult == 0 )
        {
            OTA_LOG_L1( "[%s] Accepted and committed final image.\r\n", OTA_METHOD_NAME );
            eResult = kOTA_Err_None;
        }
        else
        {
            OTA_LOG_L1( "[%s] Accepted final image but commit failed (%d).\r\n", OTA_METHOD_NAME, iResult);
            eResult = ( uint32_t ) kOTA_Err_CommitFailed;
        }
    }
    else if( eState == eOTA_ImageState_Rejected )
    {
        /* Mark the image as invalid */
        xOTACtx.partition.bImageValid = false;
        iResult = prvUpdatePartitionInfo(&xOTACtx.partition);
        if( iResult == 0)
        {
            OTA_LOG_L1( "[%s] Rejected image.\r\n", OTA_METHOD_NAME );
            eResult = kOTA_Err_None;
        }
        else
        {
            OTA_LOG_L1( "[%s] Failed updating the flags.(%d).\r\n", OTA_METHOD_NAME, iResult );
            eResult = kOTA_Err_CommitFailed;
        }
    }
    else if( eState == eOTA_ImageState_Aborted )
    {
        /* Mark the image as invalid */
        xOTACtx.partition.bImageValid = false;
        iResult = prvUpdatePartitionInfo(&xOTACtx.partition);
        if( iResult == 0 )
        {
            OTA_LOG_L1( "[%s] Aborted image.\r\n", OTA_METHOD_NAME );
            eResult = kOTA_Err_None;
        }
        else
        {
            OTA_LOG_L1( "[%s] Failed updating the flags.(%d).\r\n", OTA_METHOD_NAME, iResult );
            eResult = kOTA_Err_CommitFailed;
        }
    }
    else if( eState == eOTA_ImageState_Testing )
    {
            eResult = kOTA_Err_None;
    }
    else
    {
            OTA_LOG_L1( "[%s] Unknown state received %d.\r\n", OTA_METHOD_NAME, ( int32_t ) eState );
            eResult = kOTA_Err_BadImageState;
    }

    return eResult;
}

/* Get the state of the currently running image. */
OTA_PAL_ImageState_t prvPAL_GetPlatformImageState(void)
{

    DEFINE_OTA_METHOD_NAME( "prvPAL_GetPlatformImageState" );

    uwp_ota_flash_partition_t xOTAPartition;
    OTA_PAL_ImageState_t eImageState = eOTA_PAL_ImageState_Unknown;

    /* do not distinguish self test mode */
    if( prvGetOTAPartition(&xOTAPartition) != 0){
        OTA_LOG_L1( "[%s] can't get image.\r\n", OTA_METHOD_NAME );
        return eImageState;
    }

    if( xOTAPartition.bImageValid )
        eImageState = eOTA_PAL_ImageState_Valid;
    else
        eImageState = eOTA_PAL_ImageState_Invalid;

    return eImageState;
}

/* Read the specified signer certificate from the filesystem into a local buffer. The
 * allocated memory becomes the property of the caller who is responsible for freeing it.
 */
uint8_t * prvPAL_ReadAndAssumeCertificate( const uint8_t * const pucCertName,
                                                  uint32_t * const ulSignerCertSize )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_ReadAndAssumeCertificate" );

    uint8_t * pucCertData;
    uint32_t ulCertSize;
    uint8_t * pucSignerCert = NULL;
    CK_RV xResult;

#if 0
    xResult = prvGetCertificate( ( const char * ) pucCertName, &pucSignerCert, ulSignerCertSize );

    if( ( xResult == CKR_OK ) && ( pucSignerCert != NULL ) )
    {
        OTA_LOG_L1( "[%s] Using cert with label: %s OK\r\n", OTA_METHOD_NAME, ( const char * ) pucCertName );
    }
    else
#endif
    {
    	if( (strcmp((const char *)otatestpalCERTIFICATE_FILE, (const char *)pucCertName) != 0) &&
    			(strcmp((const char *)pkcs11configLABEL_CODE_VERIFICATION_KEY, (const char *)pucCertName) != 0) ){
    		OTA_LOG_L1( "[%s] No certificate file: %s.\r\n", OTA_METHOD_NAME , pucCertName );
    		return NULL;
    	}

        OTA_LOG_L1( "[%s] Using aws_ota_codesigner_certificate.h replace %s.\r\n", OTA_METHOD_NAME , otatestpalCERTIFICATE_FILE );

        /* Allocate memory for the signer certificate plus a terminating zero so we can copy it and return to the caller. */
        ulCertSize = sizeof( signingcredentialSIGNING_CERTIFICATE_PEM );
        pucSignerCert = pvPortMalloc( ulCertSize + 1 );                       /*lint !e9029 !e9079 !e838 malloc proto requires void*. */
        pucCertData = ( uint8_t * ) signingcredentialSIGNING_CERTIFICATE_PEM; /*lint !e9005 we don't modify the cert but it could be set by PKCS11 so it's not const. */

        if( pucSignerCert != NULL )
        {
            memcpy( pucSignerCert, pucCertData, ulCertSize );
            /* The crypto code requires the terminating zero to be part of the length so add 1 to the size. */
            pucSignerCert[ ulCertSize ] = 0U;
            *ulSignerCertSize = ulCertSize + 1U;
        }
        else
        {
            OTA_LOG_L1( "[%s] Error: No memory for certificate of size %d!\r\n", OTA_METHOD_NAME, ulCertSize );
        }
    }

    return pucSignerCert;
}

OTA_Err_t prvPAL_CheckFileSignature( OTA_FileContext_t * const C )
{
    DEFINE_OTA_METHOD_NAME( "prvPAL_CheckFileSignature" );

    OTA_Err_t eResult;
    uint32_t ulSignerCertSize;
    void * pvSigVerifyContext;
    uint8_t * pucSignerCert = NULL;

    /* Verify an ECDSA-SHA256 signature. */
    if( CRYPTO_SignatureVerificationStart( &pvSigVerifyContext, cryptoASYMMETRIC_ALGORITHM_ECDSA,
                                           cryptoHASH_ALGORITHM_SHA256 ) == pdFALSE )
    {
        eResult = kOTA_Err_SignatureCheckFailed;
    }
    else
    {
        /*OTA_LOG_L1( "[%s] Started %s signature verification, file: %s\r\n", OTA_METHOD_NAME,
                    pcOTA_JSON_FileSignatureKey, ( const char * ) C->pacCertFilepath );*/
    	OTA_LOG_L1( "[%s] Started signature verification. \r\n", OTA_METHOD_NAME );

        pucSignerCert = prvPAL_ReadAndAssumeCertificate( ( const uint8_t * const ) C->pacCertFilepath, &ulSignerCertSize );

        if( pucSignerCert == NULL )
        {
            eResult = kOTA_Err_BadSignerCert;
        }
        else
        {
            uwp_ota_flash_partition_t xFlashFile;
            prvGetOTAPartition(&xFlashFile);
            CRYPTO_SignatureVerificationUpdate( pvSigVerifyContext, (uint8_t *)(UWP_FLASH_BASE + xOTACtx.partition.ulPartitionAddrOffset),
                                                xOTACtx.partition.ulFileLength );

            if( CRYPTO_SignatureVerificationFinal( pvSigVerifyContext, ( char * ) pucSignerCert, ulSignerCertSize,
                                                   C->pxSignature->ucData, C->pxSignature->usSize ) == pdFALSE )
            {
                eResult = kOTA_Err_SignatureCheckFailed;

                /* Erase the image as signature verification failed.*/
                if( prvErasePartition(&xFlashFile) )
                {
                    OTA_LOG_L1( "[%s] Error: Failed to erase the flash !\r\n", OTA_METHOD_NAME );
                }
            }
            else
            {
                eResult = kOTA_Err_None;
            }
        }
    }

    /* Free the signer certificate that we now own after prvPAL_ReadAndAssumeCertificate(). */
    if( pucSignerCert != NULL )
    {
        vPortFree( pucSignerCert );
    }

    return eResult;
}

#if 0
static CK_RV prvGetCertificateHandle( CK_FUNCTION_LIST_PTR pxFunctionList,
                                      CK_SESSION_HANDLE xSession,
                                      const char * pcLabelName,
                                      CK_OBJECT_HANDLE_PTR pxCertHandle )
{
    CK_ATTRIBUTE xTemplate;
    CK_RV xResult = CKR_OK;
    CK_ULONG ulCount = 0;
    CK_BBOOL xFindInit = CK_FALSE;

    /* Get the certificate handle. */
    if( 0 == xResult )
    {
        xTemplate.type = CKA_LABEL;
        xTemplate.ulValueLen = strlen( pcLabelName ) + 1;
        xTemplate.pValue = ( char * ) pcLabelName;
        xResult = pxFunctionList->C_FindObjectsInit( xSession, &xTemplate, 1 );
    }

    if( 0 == xResult )
    {
        xFindInit = CK_TRUE;
        xResult = pxFunctionList->C_FindObjects( xSession,
                                                 ( CK_OBJECT_HANDLE_PTR ) pxCertHandle,
                                                 1,
                                                 &ulCount );
    }

    if( CK_TRUE == xFindInit )
    {
        xResult = pxFunctionList->C_FindObjectsFinal( xSession );
    }

    return xResult;
}

/* Note that this function mallocs a buffer for the certificate to reside in,
 * and it is the responsibility of the caller to free the buffer. */
static CK_RV prvGetCertificate( const char * pcLabelName,
                                uint8_t ** ppucData,
                                uint32_t * pulDataSize )
{
    /* Find the certificate */
    CK_OBJECT_HANDLE xHandle;
    CK_RV xResult;
    CK_FUNCTION_LIST_PTR xFunctionList;
    CK_SLOT_ID xSlotId;
    CK_ULONG xCount = 1;
    CK_SESSION_HANDLE xSession;
    CK_ATTRIBUTE xTemplate = { 0 };
    uint8_t * pucCert = NULL;
    CK_BBOOL xSessionOpen = CK_FALSE;

    xResult = C_GetFunctionList( &xFunctionList );

    if( CKR_OK == xResult )
    {
        xResult = xFunctionList->C_Initialize( NULL );
    }

    if( ( CKR_OK == xResult ) || ( CKR_CRYPTOKI_ALREADY_INITIALIZED == xResult ) )
    {
        xResult = xFunctionList->C_GetSlotList( CK_TRUE, &xSlotId, &xCount );
    }

    if( CKR_OK == xResult )
    {
        xResult = xFunctionList->C_OpenSession( xSlotId, CKF_SERIAL_SESSION, NULL, NULL, &xSession );
    }

    if( CKR_OK == xResult )
    {
        xSessionOpen = CK_TRUE;
        xResult = prvGetCertificateHandle( xFunctionList, xSession, pcLabelName, &xHandle );
    }

    if( ( xHandle != 0 ) && ( xResult == CKR_OK ) ) /* 0 is an invalid handle */
    {
        /* Get the length of the certificate */
        xTemplate.type = CKA_VALUE;
        xTemplate.pValue = NULL;
        xResult = xFunctionList->C_GetAttributeValue( xSession, xHandle, &xTemplate, xCount );

        if( xResult == CKR_OK )
        {
            pucCert = pvPortMalloc( xTemplate.ulValueLen );
        }

        if( ( xResult == CKR_OK ) && ( pucCert == NULL ) )
        {
            xResult = CKR_HOST_MEMORY;
        }

        if( xResult == CKR_OK )
        {
            xTemplate.pValue = pucCert;
            xResult = xFunctionList->C_GetAttributeValue( xSession, xHandle, &xTemplate, xCount );

            if( xResult == CKR_OK )
            {
                *ppucData = pucCert;
                *pulDataSize = xTemplate.ulValueLen;
            }
            else
            {
                vPortFree( pucCert );
            }
        }
    }
    else /* Certificate was not found. */
    {
        *ppucData = NULL;
        *pulDataSize = 0;
    }

    if( xSessionOpen == CK_TRUE )
    {
        ( void ) xFunctionList->C_CloseSession( xSession );
    }

    return xResult;
}
#endif
/*
 * @bref self test case
 */
void vOTASelfTest(void){
    vLoggingPrint("ota test ...\r\n");
    uwp_ota_flash_partition_t xPartitionInit[3] = {
            {
                    0x40000,
                    0,
                    512*1024,
                    0,
                    false,
                    PartitionAppRunning,
                    0
            },
            {
                    0x2C0000,
                    0,
                    512*1024,
                    0,
                    false,
                    PartitionAppOta,
                    1
            },
            {
                    0x340000,
                    0,
                    768*1024,
                    0,
                    false,
                    PartitionModemOta,
                    1
            },
    };

    uwp_ota_flash_partition_t xPartitionTemp;

    vLoggingPrintf("partition addr:0x%x\r\n", xPartitionFlash);
    int iResult = prvFlashEraseAndWrite( (uint8_t *)xPartitionInit, ((uint32_t)xPartitionFlash) - UWP_FLASH_BASE, sizeof(xPartitionInit));
    if ( iResult != 0 ){
        vLoggingPrintf("write init partition info failed:%d\r\n", iResult);
        return;
    }
    vLoggingPrintf("write init partition info success: partition:%d addr:%x state:%d\r\n",
                      xPartitionFlash[0].ucPartition, xPartitionFlash[0].ulPartitionAddrOffset, xPartitionFlash[0].eState);

#if 0
    iResult = prvGetOTAPartition(&xPartitionTemp);
    if( iResult != 0 ){
        vLoggingPrintf("get ota partition failed:%d\r\n", iResult);
    }
    vLoggingPrintf("get ota partition info success: partition:%d addr:%x state:%d\r\n",
            xPartitionTemp.ucPartition, xPartitionTemp.ulPartitionAddrOffset, xPartitionTemp.eState);

    uint32_t test_addr = 0x40000 - 0x5;
    char test_data[] = "aws sector critical erase and write test\r\n";
    iResult = prvFlashEraseAndWrite((uint8_t *)test_data, test_addr, sizeof(test_data));
    if( iResult != 0 ){
        vLoggingPrintf("prvFlashEraseAndWrite test failed\r\n");
    }
    vLoggingPrintf("prvFlashEraseAndWrite success:%s", (char *)(test_addr + UWP_FLASH_BASE));
#endif

}

