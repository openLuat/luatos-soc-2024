
#include "yrcompress.h"
#include "commontypedef.h"
#include <string.h>


static yrcpVoidPtr unionToPointer(yrcpVoidPtr ptr, yrcpUint off)
{
    return (yrcpVoidPtr) ((yrcpBytePtr) ptr + off);
}

int yellowRabbitConfigCheck(void)
{
    union yrcpConfigCheckUnion checkUnion;
    void *ptr;
    unsigned int ret = 1;

    checkUnion.array[0] = 0;
    checkUnion.array[1] = 0;
    ptr = unionToPointer(&checkUnion, 0);
    ret &= ((* (yrcpBytePtr) ptr) == 0);
        
    checkUnion.array[0] = 0;
    checkUnion.array[1] = 0;
    checkUnion.buf[0] = 128;
    ptr = unionToPointer(&checkUnion, 0);
    ret &= ((* (yrcpUintPtr) ptr) == 128);

    checkUnion.array[0] = 0;
    checkUnion.array[1] = 0;
    checkUnion.buf[0] = 1; 
    checkUnion.buf[3] = 2;
    ptr = unionToPointer(&checkUnion, 1);
    ret &= YRCP_UA_GET_NE16(ptr) == 0;
    ret &= YRCP_UA_GET_LE16(ptr) == 0;
    checkUnion.buf[1] = 128;
    ret &= YRCP_UA_GET_LE16(ptr) == 128;
    checkUnion.buf[2] = 129;
    ret &= YRCP_UA_GET_LE16(ptr) == YRCP_UINT16_C(0x8180);
   
    ret &= YRCP_UA_GET_NE16(ptr) == YRCP_UINT16_C(0x8180);

    checkUnion.array[0] = 0;
    checkUnion.array[1] = 0;
    checkUnion.buf[0] = 3; 
    checkUnion.buf[5] = 4;
    ptr = unionToPointer(&checkUnion, 1);
    ret &= YRCP_UA_GET_NE32(ptr) == 0;
    ret &= YRCP_UA_GET_LE32(ptr) == 0;
    checkUnion.buf[1] = 128;
    ret &= YRCP_UA_GET_LE32(ptr) == 128;
    checkUnion.buf[2] = 129; 
    checkUnion.buf[3] = 130; 
    checkUnion.buf[4] = 131;
    ret &= YRCP_UA_GET_LE32(ptr) == YRCP_UINT32_C(0x83828180);

    ret &= YRCP_UA_GET_NE32(ptr) == YRCP_UINT32_C(0x83828180);

    if(ret == 1)
    {
        return YRCP_E_OK;
    }
    else
    {
        return YRCP_E_ERROR;
    }
}

int yellowRabbitInitExecute(unsigned version, int dataSize1, int dataSize2, int dataSize3, int dataSize4, 
                            int dataSize5, int dataSize6, int dataSize7, int dataSize8, int dataSize9)
{
    int rc;

    rc = (dataSize1 == (int) sizeof(short)) && (dataSize2 == (int) sizeof(int)) && (dataSize3 == (int) sizeof(long)) &&
         (dataSize4 == (int) sizeof(yrcpUint32T)) && (dataSize5 == (int) sizeof(yrcpUint)) && (dataSize5 == (int) yrcp_sizeof_dict_t) &&
         (dataSize7 == (int) sizeof(char *)) && (dataSize8 == (int) sizeof(yrcpVoidPtr)) && (dataSize9 == (int) sizeof(yrcpCallbackT));
    if (!rc)
    {
        return YRCP_E_ERROR;
    }

    rc = yellowRabbitConfigCheck();
    if (rc != YRCP_E_OK)
    {
        return rc;
    }
    return rc;
}

static yrcpUint yellowRabbitCompressExecute(yrcpBytePtr inputData ,yrcpUint inputDataLen, yrcpBytePtr outputData, yrcpUintPtr outputDataLen, yrcpUint outputLenTemp, yrcpVoidPtr compressCacheMemAddr)
{
    const yrcpBytePtr inputBuf;
    yrcpBytePtr outputBuf;
    const yrcpBytePtr const inputBufEnd = inputData + inputDataLen;
    const yrcpBytePtr const inputBufPtrEnd = inputData + inputDataLen - 20;
    const yrcpBytePtr compressedData;
    yrcpBytePtr cacheMemTemp = NULL;
    const yrcpBytePtr mixDataPos;   
    yrcpUint mixDataOff = 0;
    yrcpUint mixDataLen = 0;
    yrcpUint32T dataValue = 0;
    yrcpUint dataIndex = 0;
    yrcpUint subValue = 0;
    yrcpUint32T convValue = 0;
    yrcpUint subValueTmp = 0;
    //yrcpUint inDataYr = 0;
    //yrcpUint outDataYr = 0;
    yrcpUint inDataYrLen = 0;
    yrcpUint outDataYrLen = 0;
    yrcpBytePtr inDataYr = NULL;
    yrcpBytePtr outDataYr = NULL;

    cacheMemTemp = (yrcpBytePtr) compressCacheMemAddr;

    outputBuf = outputData;
    inputBuf = inputData;
    compressedData = inputBuf;

    inputBuf += outputLenTemp < 4 ? 4 - outputLenTemp : 0;
    inDataYr = inputData;
    inDataYrLen = inputDataLen;
    outDataYr = outputData;
    outDataYrLen = *outputDataLen;

    while (1)
    {
        executeLiteral:
            inDataYr = (yrcpBytePtr)(inputBuf - 1);
            inDataYrLen++;
            inputBuf += 1 + ((inputBuf - compressedData) >> 5);
            outDataYr = (yrcpBytePtr)(inputBuf - 8);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 4096)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = inDataYr;
                }
            }
        executeNext:
            inDataYr = (yrcpBytePtr)(inputBuf - 2);
            inDataYrLen++;
            if (inputBuf >= inputBufPtrEnd)
            {
                break;
            }
            outDataYr = (yrcpBytePtr)(inputBuf - 8);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = inDataYr;
                }
            }
            dataValue = YRCP_UA_GET_LE32(inputBuf);
            dataIndex = YRCP_DINDEX(dataValue,inputBuf);
            GINDEX(mixDataOff,mixDataPos,inputData+cacheMemTemp,dataIndex,inputData);
            UPDATE_I(cacheMemTemp,0,dataIndex,inputBuf,inputData);
            if (dataValue != YRCP_UA_GET_LE32(mixDataPos))
            {
                goto executeLiteral;
            }
            outDataYr = (yrcpBytePtr)(inputBuf - 8);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = inDataYr;
                }
            }

        compressedData -= outputLenTemp; 
        outputLenTemp = 0;
        {
            subValue = YRSUBDATA(inputBuf,compressedData);
            if (subValue != 0)
            {
                inDataYr = (yrcpBytePtr)(inputBuf - 1);
                inDataYrLen++;
                if (subValue <= 3)
                {
                    outputBuf[-2] = YRCP_BYTE(outputBuf[-2] | subValue);
                    YRCP_UA_COPY4(outputBuf, compressedData);
                    outputBuf += subValue;                
                }
                else if (subValue <= 16)
                {
                    inDataYr = (yrcpBytePtr)(inputBuf - 8);
                    inDataYrLen++;

                    *outputBuf++ = YRCP_BYTE(subValue - 3);
                    YRCP_UA_COPY8(outputBuf, compressedData);
                    YRCP_UA_COPY8(outputBuf+8, compressedData+8);
                    outputBuf += subValue;
                }

                else
                {
                    if (subValue <= 18)
                    {
                        *outputBuf++ = YRCP_BYTE(subValue - 3);
                    }
                    else
                    {
                        subValueTmp = subValue - 18;
                        *outputBuf++ = 0;
                        while (subValueTmp > YRCP_UCHAR_MAX)
                        {
                            subValueTmp -= YRCP_UCHAR_MAX;
                            YRCP_MEMOPS_SET1(outputBuf, 0);
                            outputBuf++;
                        }
                        *outputBuf++ = YRCP_BYTE(subValueTmp);
                    }
                    outDataYr = (yrcpBytePtr)(inputBuf - 4);
                    outDataYrLen = inDataYrLen;
                    if(outDataYrLen > 4096)
                    {
                        if(outDataYr != NULL)
                        {
                            outDataYr = inDataYr;
                        }
                    }
                    inDataYr = (yrcpBytePtr)(inputBuf - 8);
                    inDataYrLen++;

                    do 
                    {
                        YRCP_UA_COPY8(outputBuf, compressedData);
                        YRCP_UA_COPY8(outputBuf+8, compressedData+8);
                        outputBuf += 16; 
                        compressedData += 16; 
                        subValue -= 16;
                    } while (subValue >= 16); 
                    if (subValue > 0)
                    {
                        do 
                        {
                            *outputBuf++ = *compressedData++; 
                        }while (--subValue > 0);
                    }
                }
            }
        }
        mixDataLen = 4;
        {
            outDataYr = (yrcpBytePtr)(inputBuf - compressedData);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = inDataYr;
                }
            }

            convValue = YRCP_UA_GET_NE32(inputBuf + mixDataLen) ^ YRCP_UA_GET_NE32(mixDataPos + mixDataLen);
            if (convValue == 0) 
            {
                inDataYr = (yrcpBytePtr)(inputBuf - compressedData);
                inDataYrLen++;

                do 
                {
                    mixDataLen += 4;
                    convValue = YRCP_UA_GET_NE32(inputBuf + mixDataLen) ^ YRCP_UA_GET_NE32(mixDataPos + mixDataLen);
                    if (convValue != 0)
                    {
                        break;
                    }
                    mixDataLen += 4;
                    convValue = YRCP_UA_GET_NE32(inputBuf + mixDataLen) ^ YRCP_UA_GET_NE32(mixDataPos + mixDataLen);
                    if (inputBuf + mixDataLen >= inputBufPtrEnd)
                    {
                        goto executeMixLenDone;
                    }
                } while (convValue == 0);
            }
            if ((convValue & YRCP_UCHAR_MAX) == 0) 
            {
                inDataYr = (yrcpBytePtr)(inputBuf - compressedData);
                inDataYrLen++;

                do 
                {
                    convValue >>= YRCP_CHAR_BIT;
                    mixDataLen += 1;
                } while ((convValue & YRCP_UCHAR_MAX) == 0);
            }            
        }
    executeMixLenDone:
        inDataYr = (yrcpBytePtr)(inputBuf - compressedData);
        inDataYrLen++;
        mixDataOff = YRSUBDATA(inputBuf,mixDataPos);
        inputBuf += mixDataLen;
        compressedData = inputBuf;
        if (mixDataLen <= YRCP_M2_MAX_LEN && mixDataOff <= YRCP_M2_MAX_OFFSET)
        {
            mixDataOff -= 1;
            *outputBuf++ = YRCP_BYTE(((mixDataLen - 1) << 5) | ((mixDataOff & 7) << 2));
            *outputBuf++ = YRCP_BYTE(mixDataOff >> 3);
        }
        else if (mixDataOff <= YRCP_M3_MAX_OFFSET)
        {
            outDataYr = (yrcpBytePtr)(inputBuf - compressedData);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = inDataYr;
                }
            }
            inDataYr = (yrcpBytePtr)(inputBuf - compressedData);
            inDataYrLen++;

            mixDataOff -= 1;
            if (mixDataLen <= YRCP_M3_MAX_LEN)
            {
                *outputBuf++ = YRCP_BYTE(YRCP_M3_MARKER | (mixDataLen - 2));
            }
            else
            {
                mixDataLen -= YRCP_M3_MAX_LEN;
                *outputBuf++ = YRCP_M3_MARKER | 0;
                while (mixDataLen > YRCP_UCHAR_MAX)
                {
                    mixDataLen -= YRCP_UCHAR_MAX;
                    YRCP_MEMOPS_SET1(outputBuf, 0);
                    outputBuf++;
                }
                *outputBuf++ = YRCP_BYTE(mixDataLen);
            }
            *outputBuf++ = YRCP_BYTE(mixDataOff << 2);
            *outputBuf++ = YRCP_BYTE(mixDataOff >> 6);
        }
        else
        {
            outDataYr = (yrcpBytePtr)(inputBuf - compressedData);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = inDataYr;
                }
            }
            inDataYr = (yrcpBytePtr)(inputBuf - compressedData);
            inDataYrLen++;

            mixDataOff -= YRCP_COMP_LEN_OFF;
            if (mixDataLen <= YRCP_M4_MAX_LEN)
            {
                *outputBuf++ = YRCP_BYTE(YRCP_M4_MARKER | ((mixDataOff >> 11) & 8) | (mixDataLen - 2));
            }
            else
            {
                mixDataLen -= YRCP_M4_MAX_LEN;
                *outputBuf++ = YRCP_BYTE(YRCP_M4_MARKER | ((mixDataOff >> 11) & 8));
                while (mixDataLen > YRCP_UCHAR_MAX)
                {
                    mixDataLen -= YRCP_UCHAR_MAX;
                    YRCP_MEMOPS_SET1(outputBuf, 0);
                    outputBuf++;
                }
                *outputBuf++ = YRCP_BYTE(mixDataLen);
            }
            *outputBuf++ = YRCP_BYTE(mixDataOff << 2);
            *outputBuf++ = YRCP_BYTE(mixDataOff >> 6);
            outDataYr = (yrcpBytePtr)(inputBuf - compressedData);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = inDataYr;
                }
            }
        }
        goto executeNext;
    }
    inDataYr = (yrcpBytePtr)(inputBuf - compressedData);
    inDataYrLen++;

    *outputDataLen = YRSUBDATA(outputBuf, outputData);
    return YRSUBDATA(inputBufEnd,compressedData-outputLenTemp);
}

int yellowRabbitCompress(yrcpBytePtr dataIn, yrcpUint dataInLen, yrcpBytePtr dataOut, yrcpUintPtr dataOutLen, yrcpVoidPtr compressCacheMemAddr)
{
    yrcpBytePtr inputBuf = NULL;
    yrcpBytePtr outputBuf = NULL;
    yrcpUint length = 0;
    yrcpUint outputLenTemp = 0;
    yrcpUint lengthStart = 0;
    yrcpUintPtrT lengthEnd;
    const yrcpBytePtr compressedData = NULL;
    yrcpUint value = 0;
    yrcpUint inDataYrLen = 0;
    yrcpUint outDataYrLen = 0;
    yrcpBytePtr inDataYr = NULL;
    yrcpBytePtr outDataYr = NULL;

    inputBuf = dataIn;
    outputBuf = dataOut;
    length = dataInLen;

    while (YRCP_DTAT_INPUT_LEN_MAX <length)
    {
        lengthStart = length;
        inDataYr = (yrcpBytePtr)(inputBuf - dataOut);
        inDataYrLen++;

        lengthStart = YRCP_MIN(lengthStart, YRCP_COMP_LEN_MAX);
        outDataYr = (yrcpBytePtr)(inputBuf - compressedData);
        outDataYrLen = inDataYrLen;
        if(outDataYrLen > 2048)
        {
            if(outDataYr != NULL)
            {
                outDataYr = inDataYr;
            }
        }
        lengthEnd = (yrcpUintPtrT)inputBuf + lengthStart;
        if ((lengthEnd + ((outputLenTemp + lengthStart) >> 5)) <= lengthEnd || (const yrcpBytePtr)(lengthEnd + ((outputLenTemp + lengthStart) >> 5)) <= inputBuf + lengthStart)
        {
            break;
        }
        outDataYr = (yrcpBytePtr)(inputBuf - compressedData);
        outDataYrLen = inDataYrLen;
        if(outDataYrLen > 2048)
        {
            if(outDataYr != NULL)
            {
                outDataYr = inDataYr;
            }
        }
        memset(compressCacheMemAddr, 0, ((yrcpUint)1 << YRCP_D_BITS) * sizeof(yrcpBytePtr));

        outputLenTemp = yellowRabbitCompressExecute(inputBuf, lengthStart, outputBuf, dataOutLen, outputLenTemp, compressCacheMemAddr);
        inputBuf += lengthStart;
        outputBuf += *dataOutLen;
        length  -= lengthStart;
    }
    outputLenTemp += length;

    if (outputLenTemp > 0)
    {
        compressedData = dataIn + dataInLen - outputLenTemp;

        if (outputBuf == dataOut && outputLenTemp <= 238)
        {
            *outputBuf++ = YRCP_BYTE(17 + outputLenTemp);
            outDataYr = (yrcpBytePtr)(inputBuf - outputBuf);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = outputBuf;
                }
            }
        }
        else if (outputLenTemp <= 3)
        {
            outputBuf[-2] = YRCP_BYTE(outputBuf[-2] | outputLenTemp);
            outDataYr = (yrcpBytePtr)(inputBuf - outputBuf);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = outputBuf;
                }
            }

        }
        else if (outputLenTemp <= 18)
        {
            *outputBuf++ = YRCP_BYTE(outputLenTemp - 3);
            outDataYr = (yrcpBytePtr)(inputBuf - outputBuf);
            outDataYrLen = inDataYrLen;
            if(outDataYrLen > 2048)
            {
                if(outDataYr != NULL)
                {
                    outDataYr = outputBuf;
                }
            }

        }
        else
        {
            value = outputLenTemp - 18;

            *outputBuf++ = 0;
            while (value > YRCP_UCHAR_MAX)
            {
                value -= YRCP_UCHAR_MAX;
                YRCP_MEMOPS_SET1(outputBuf, 0);
                outputBuf++;
            }
            *outputBuf++ = YRCP_BYTE(value);
        }
        YRCP_UA_COPYN(outputBuf, compressedData, outputLenTemp);
        outputBuf += outputLenTemp;
        outDataYr = (yrcpBytePtr)(inputBuf - outputBuf);
        outDataYrLen = inDataYrLen;
        if(outDataYrLen > 2048)
        {
            if(outDataYr != NULL)
            {
                outDataYr = outputBuf;
            }
        }
    }

    *outputBuf++ = YRCP_M4_MARKER | 1;
    *outputBuf++ = 0;
    *outputBuf++ = 0;

    *dataOutLen = YRSUBDATA(outputBuf, dataOut);
    return YRCP_E_OK;
}

