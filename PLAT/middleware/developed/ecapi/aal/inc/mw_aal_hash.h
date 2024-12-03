
#ifndef _MW_AAL_HASH_H_
#define _MW_AAL_HASH_H_

#include <stddef.h>
#include <stdint.h>
#include "sctdef.h"

#define MW_SHA_MODE_HW
#ifdef MW_SHA_MODE_HW

#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
#include "tls.h"
#else
#include "sct_aes_sha_api.h"
#endif

typedef uint32_t  MwSha256Ctx_t;
#else /* non-hw */

#include "sha256.h"

typedef mbedtls_sha256_context  MwSha256Ctx_t;
#endif



typedef struct
{
    uint8_t  is224;
    uint8_t  rsvd[3];
    MwSha256Ctx_t info;
}MwAalSha256Ctx_t;


void     mwAalInitSha256( MwAalSha256Ctx_t *ctx, int is224);
void     mwAalDeinitSha256(MwAalSha256Ctx_t *ctx);
uint32_t mwAalUpdateSha256(MwAalSha256Ctx_t *ctx, const uint8_t *input, uint8_t * output, size_t ilen, uint32_t last);

#endif


