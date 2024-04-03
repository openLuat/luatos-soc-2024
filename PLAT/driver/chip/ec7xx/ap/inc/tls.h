#ifndef __TLS_H__
#define __TLS_H__

#include "stdio.h"
#include "string.h"
#include "ec7xx.h"
#include "bsp.h"



#define SCTDRV_OK           (0)
#define SCTDRV_BUSY         (-1)
#define SCTDRV_TIMEOUT      (-2)
#define SCTDRV_PAMERR       (-5)


typedef enum 
{
    SHA_TYPE_1,
    SHA_TYPE_224,
    SHA_TYPE_256,    
}shaType_e;

typedef struct
{
    uint32_t dir            : 1; // 0: encrypt, 1: decrypt
    uint32_t aesMode        : 2; // 0: ecb, 1:cbc, 2:ctr
    uint32_t paddingMode    : 3; // 0: no padding, 1: PKCS7, 2: paddingOneZeros, 3: paddingZerosLen, 4: paddingZeros
    uint32_t ckLen          : 2; // 0:128, 1:192, 2:256
    uint32_t aesCkSel       : 1; // 0: from memory, 1: from efuse
    uint32_t ckBLEndian     : 1; // Ignore it if key is from efuse. If key is from memory, 0: little; 1: big
    uint32_t aesIvBLEndian  : 1; // input AES IV is big/little endian. 0: little;   1: big
    uint32_t                : 23;
}aesCtrl_t;

typedef struct
{
    uint32_t  ivAddr;
    uint32_t  srcAddr;
    uint32_t  dstAddr;
    uint32_t  aesCkAddr;
    uint32_t  length;
    aesCtrl_t aesCtrl;
}aesInfo_t;


/**
  \brief SCT module init.
  \return               
*/
void sctInit();

void sctInitLzma();

/**
  \brief SCT module deInit.
  \return               
*/
void sctDeInit();

void sctDeInitLzma();

/**
  \brief Aes operation
  \param[in] aesInfo   Aes information.
  \return status              
*/
int32_t aesUpdate(aesInfo_t* aesInfo);

/**
  \brief Sha operation.
  \param[in] shaMode  Choose SHA1, SHA224, SHA256.
  \param[in] srcAddr  SHA input address.
  \param[in] dstAddr  SHA output address.
  \param[in] length   SHA input length.
  \param[in] lastFlag If you need to loop call this api, "lastFlag" should be 0 for intermediate steps, and last step it should be 1.
  \return status              
*/
int32_t shaUpdate(shaType_e shaMode, uint32_t srcAddr, uint32_t dstAddr, uint32_t length, uint32_t lastFlag);

int32_t shaUpdateLzma(shaType_e shaMode, uint32_t srcAddr, uint32_t dstAddr, uint32_t length, uint32_t lastFlag);

#endif
