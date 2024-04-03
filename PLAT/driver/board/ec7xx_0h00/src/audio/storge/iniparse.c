/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    test.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ps_lib_api.h"
#include "storage.h"
#include "iniparse.h"


#define KEY_VALUE_LINE_END                  "\r\n"
#define KEY_VALUE_LINE_LENGTH_MAX           128


static char *getValuePos(char *buffer)
{
    char     *retVal = NULL;
    uint32_t length  = 0;
    bool     valid   = false;

    if (buffer != NULL)
    {
        length = strlen(buffer);
        for (uint32_t i=0; i<length; i++)
        {
            if ((buffer[i] != ' ') && (buffer[i] != '\t'))
            {
                if (valid == true)
                {
                    retVal = buffer + i;
                    goto labelEnd;
                }
                else if (buffer[i] == '=')
                {
                    valid = true;
                }
                else
                {
                    goto labelEnd;
                }
            }
        }
    }

labelEnd:
    return retVal;
}

void *iniKeyValueRead(char *path, char *key, uint8_t valueType)
{
    int32_t    resVal  = LFS_ERR_OK;
    lfs_file_t file    = {0};
    uint32_t   size    = 0;
    char       *buffer = NULL;
    void       *value  = NULL;
    char       *pos    = NULL;

    if ((path == NULL) || (key == NULL) || (valueType >= INI_VALUE_INVALID_TYPE))
    {
        printf("Parameters are invalid.\r\n");
        goto labelEnd;
    }

    resVal = fsFileOpen(&file, path, LFS_O_RDONLY);
    if (resVal != LFS_ERR_OK)
    {
        // printf("Failed to open the file \"%s\".\r\n", path);
        goto labelEnd;
    }

    size = fsFileSize(&file);
    if (size <= 0)
    {
        printf("Failed to get size of the file %s or the %s is empty.\r\n", path, path);
        goto labelEnd;
    }

    buffer = malloc(size + 1);
    if (buffer == NULL)
    {
        printf("Failed to malloc %d bytes for buffer\r\n", size + 1);
        goto labelEnd;
    }

    memset(buffer, 0, size + 1);
    fsFileRead(&file, (void *)buffer, size);
    fsFileClose(&file);

    switch (valueType)
    {
        case INI_VALUE_INT:
            pos = strstr(buffer, key);
            if (pos == NULL)
            {
                printf("Failed to find \"%s\" in \"%s\"\r\n", key, path);
                goto labelEnd;
            }

            pos = getValuePos(pos + strlen(key));
            if (pos == NULL)
            {
                printf("Failed to find the value of \"%s\" in \"%s\"\r\n", key, path);
                goto labelEnd;
            }

            value = malloc(4);
            if (value == NULL)
            {
                printf("Failed to malloc %d bytes for value\r\n", 4);
                goto labelEnd;
            }
            *((int32_t *)value) = atoi(pos);
            break;

        default:
            break;
    }

labelEnd:
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    return value;
}

int32_t iniKeyValueWrite(char *path, char *key, uint8_t valueType, void *value)
{
    int32_t    retVal       = -1;
    lfs_file_t file         = {0};
    bool       opened       = false;
    uint32_t   sizeRead     = 0;
    uint32_t   sizeWrite    = 0;
    char       *bufferRead  = NULL;
    char       *bufferWrite = NULL;
    char       *posBegin    = NULL;
    char       *posEnd      = NULL;
    uint32_t   offset       = 0;

    if ((path == NULL) || (key == NULL) || (valueType >= INI_VALUE_INVALID_TYPE) || (value == NULL))
    {
        printf("Parameters are invalid.\r\n");
        goto labelEnd;
    }

    retVal = fsFileOpen(&file, path, LFS_O_RDWR | LFS_O_CREAT);
    if (retVal != LFS_ERR_OK)
    {
        printf("Failed to open the file \"%s\".\r\n", path);
        goto labelEnd;
    }

    opened   = true;
    sizeRead = fsFileSize(&file);
    if (sizeRead < 0)
    {
        printf("Failed to get size of the file %s\r\n", path);
        goto labelEnd;
    }

    if (sizeRead > 0)
    {
        bufferRead = malloc(sizeRead + 1);
        if (bufferRead == NULL)
        {
            printf("Failed to malloc %d bytes for bufferRead\r\n", sizeRead + 1);
            goto labelEnd;
        }

        memset(bufferRead, 0, sizeRead + 1);
        fsFileRead(&file, (void *)bufferRead, sizeRead);

        posBegin = strstr(bufferRead, key);
        if (posBegin == NULL)
        {
            offset = sizeRead;
        }
        else
        {
            offset = posBegin - bufferRead;
        }
    }

    sizeWrite = sizeRead - offset + KEY_VALUE_LINE_LENGTH_MAX;
    bufferWrite = malloc(sizeWrite + 1);
    if (bufferWrite == NULL)
    {
        printf("Failed to malloc %d bytes for bufferWrite\r\n", sizeWrite + 1);
        goto labelEnd;
    }
    memset(bufferWrite, 0, sizeWrite + 1);

    switch (valueType)
    {
        case INI_VALUE_INT:
            snprintf(bufferWrite, sizeWrite, "%s=%d\r\n", key, *((int32_t *)value));
            break;

        default:
            printf("Unsupported config data type.\r\n");
            goto labelEnd;
            break;
    }

    if (posBegin != NULL)
    {
        posEnd = strstr(posBegin + strlen(key), KEY_VALUE_LINE_END);
        if (posEnd != NULL)
        {
            posEnd += strlen(KEY_VALUE_LINE_END);
            memcpy(bufferWrite + strlen(bufferWrite), posEnd, strlen(posEnd));
        }
    }

    fsFileSeek(&file, offset, LFS_SEEK_SET);
    fsFileWrite(&file, (void *)bufferWrite, strlen(bufferWrite));

    retVal = 0;

labelEnd:
    if (opened == true)
    {
        fsFileClose(&file);
    }
    if (bufferRead != NULL)
    {
        free(bufferRead);
        bufferRead = NULL;
    }
    if (bufferWrite != NULL)
    {
        free(bufferWrite);
        bufferWrite = NULL;
    }
    return retVal;
}
