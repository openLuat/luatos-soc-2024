

#ifndef FOTA_ZIPCODEC_H
#define FOTA_ZIPCODEC_H


typedef enum
{
    FOTA_ZA_LZMA = 0,
    FOTA_ZA_BZIP,

    FOTA_ZA_MAXNUM
}FotaZipAlgo_e;


int32_t FOTA_encodeLzma(FotaBufferDesc_t *outBuf, FotaBufferDesc_t *inBuf);
int32_t FOTA_decodeLzma(FotaBufferDesc_t *outBuf, FotaBufferDesc_t *inBuf);


#endif
