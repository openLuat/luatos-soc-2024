#include <stdio.h>
#include "cmsis_os2.h"
#include "bsp.h"
#include "sdcard.h"


extern ARM_DRIVER_SPI Driver_SPI0;
static ARM_DRIVER_SPI *spiMasterDrv = &CREATE_SYMBOL(Driver_SPI, 0);
#define SD_SPI_SSN_GPIO_INSTANCE    RTE_SPI0_SSN_GPIO_INSTANCE
#define SD_SPI_SSN_GPIO_INDEX       RTE_SPI0_SSN_GPIO_INDEX
#define TRANSFER_DATA_WIDTH         8
#define SPEED_LOW                   0
#define SPEED_HIGH                  1
#define SD_SPI_SPEED_LOW            100000
#define SD_SPI_SPEED_HIGH           26000000
#define SD_POWER_PAD                46
#define SD_POWER_GPIO               21
#define SD_POWER_PORT               ((SD_POWER_GPIO) / 16)
#define SD_POWER_PIN                ((SD_POWER_GPIO) % 16)


uint8_t gSdType = 0;

#if 0 // unused
static void sdCtrlIoInit(void)
{
    PadConfig_t     padConfig = {0};
    GpioPinConfig_t pinConfig = {0};

    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(SD_POWER_PAD,     &padConfig);

    pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    pinConfig.misc.initOutput = 1;
    GPIO_pinConfig(SD_POWER_PORT, SD_POWER_PIN, &pinConfig);
}
#endif // unused

static void sdSpiIoInit(uint8_t speed)
{
    static bool inited = false;
    uint32_t    freq   = (speed == SPEED_LOW) ? SD_SPI_SPEED_LOW : SD_SPI_SPEED_HIGH;

    if (inited == false)
    {
        inited = true;
        spiMasterDrv->Initialize(NULL);
        spiMasterDrv->PowerControl(ARM_POWER_FULL);
    }

    spiMasterDrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(TRANSFER_DATA_WIDTH) |
                          ARM_SPI_MSB_LSB     | ARM_SPI_SS_MASTER_SW, freq);
}

static void sdSpiCsSet(uint8_t state)
{
    if (state == 0)
    {
        GPIO_pinWrite(SD_SPI_SSN_GPIO_INSTANCE, 1 << SD_SPI_SSN_GPIO_INDEX, 1 << SD_SPI_SSN_GPIO_INDEX);
    }
    else
    {
        GPIO_pinWrite(SD_SPI_SSN_GPIO_INSTANCE, 1 << SD_SPI_SSN_GPIO_INDEX, 0);
    }
}

static uint8_t sdSpiReadWriteByte(uint8_t dataIn)
{
    uint8_t dataOut = 0;

    spiMasterDrv->Transfer(&dataIn, &dataOut, 1);

    return dataOut;
}

static uint8_t sdWaitReady(void)
{
    uint32_t t = 0;

    do
    {
        if (sdSpiReadWriteByte(0xFF) == 0xFF)
        {
            return SD_OK;
        }

        t++;
    } while (t < 0xFFFF);

    return SD_ERROR;
}

static void sdDeselect(void)
{
    sdSpiCsSet(1);
    sdSpiReadWriteByte(0xFF);
}

static uint8_t sdSelect(void)
{
    sdSpiCsSet(0);

    if (sdWaitReady() == 0)
    {
        return SD_OK;
    }

    sdDeselect();
    return SD_ERROR;
}

static uint8_t sdGetResponse(uint8_t response)
{
    uint16_t count = 0xFFFF;

    while ((sdSpiReadWriteByte(0xFF) != response) && count)
    {
        count--;
    }

    if (count == 0)
    {
        return SD_ERROR;
    }

    return SD_OK;
}

static uint8_t sdReceiveData(uint8_t *buf, uint16_t len)
{
    if (sdGetResponse(0xFE))
    {
        return SD_ERROR;
    }

    while (len--)
    {
        *buf = sdSpiReadWriteByte(0xFF);
        buf++;
    }

    sdSpiReadWriteByte(0xFF);
    sdSpiReadWriteByte(0xFF);

    return SD_OK;
}

static uint8_t sdSendBlock(uint8_t *buf, uint8_t cmd)
{
    uint16_t t;

    if (sdWaitReady())
    {
        return SD_ERROR;
    }

    sdSpiReadWriteByte(cmd);

    if (cmd != 0xFD)
    {
        for (t = 0; t < SD_BLOCK_SIZE; t++)
        {
            sdSpiReadWriteByte(buf[t]);
        }

        sdSpiReadWriteByte(0xFF);
        sdSpiReadWriteByte(0xFF);

        t = sdSpiReadWriteByte(0xFF);

        if ((t & 0x1F) != 0x05)
        {
            return SD_ERROR;
        }
    }

    return SD_OK;
}

static uint8_t sdSendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t res;
    uint8_t retry = 0;
    uint8_t crc = 0x01;

    if (cmd & 0x80)
    {
        cmd &= 0x7F;
        res = sdSendCmd(CMD55, 0);

        if (res > 1)
        {
            return res;
        }
    }

    if (cmd != CMD12)
    {
        sdDeselect();

        if (sdSelect())
        {
            return 0xFF;
        }
    }

    sdSpiReadWriteByte(cmd | 0x40);
    sdSpiReadWriteByte(arg >> 24);
    sdSpiReadWriteByte(arg >> 16);
    sdSpiReadWriteByte(arg >> 8);
    sdSpiReadWriteByte(arg);

    if (cmd == CMD0) crc = 0x95;

    if (cmd == CMD8) crc = 0x87;

    sdSpiReadWriteByte(crc);

    if (cmd == CMD12)
    {
        sdSpiReadWriteByte(0xFF);
    }


    retry = 10;

    do
    {
        res = sdSpiReadWriteByte(0xFF);
    } while ((res & 0x80) && retry--);

    return res;
}

uint8_t sdGetStatus(void)
{
    uint8_t res;
    uint8_t retry = 20;

    do
    {
        res = sdSendCmd(ACMD13, 0);
    }while(res && retry--);

    sdDeselect();

    return res;
}

uint8_t sdGetCid(uint8_t *cid_data)
{
    uint8_t res;

    res = sdSendCmd(CMD10, 0);

    if (res == 0x00)
    {
        res = sdReceiveData(cid_data, 16);
    }

    sdDeselect();

    return res;
}

uint8_t sdGetCsd(uint8_t *csd_data)
{
    uint8_t res;
    res = sdSendCmd(CMD9, 0);

    if (res == 0)
    {
        res = sdReceiveData(csd_data, 16);
    }

    sdDeselect();
    return res;
}

uint32_t sdGetBlockCount(void)
{
    uint8_t csd[16];
    uint32_t capacity;
    uint8_t n;
    uint16_t csize;

    if (sdGetCsd(csd) != 0)
    {
        return 0;
    }

    if ((csd[0] & 0xC0) == 0x40)
    {
        csize = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 63) << 16) + 1;
        capacity = (uint32_t)csize << 10;
    }
    else
    {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
        capacity = (uint32_t)csize << (n - 9);
    }

    return capacity;
}

uint8_t sdInit(void)
{
    uint8_t res;
    uint16_t retry;
    uint8_t ocr[4];
    uint16_t i;
    uint8_t cmd;

    // sdCtrlIoInit();
    // osDelay(100);
    sdSpiIoInit(SPEED_LOW);
    sdSpiCsSet(1);

    for (i = 0; i < 10; i++)
    {
        sdSpiReadWriteByte(0xFF);
    }

    retry = 20;

    do
    {
        res = sdSendCmd(CMD0, 0);
    } while ((res != 0x01) && retry--);

    gSdType = 0;

    if (res == 0x01)
    {
        if (sdSendCmd(CMD8, 0x1AA) == 1)
        {
            for (i = 0; i < 4; i++)
            {
                ocr[i] = sdSpiReadWriteByte(0xFF);
            }

            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            {
                retry = 1000;

                do
                {
                    res = sdSendCmd(ACMD41, 1UL << 30);
                } while (res && retry--);

                if (retry && sdSendCmd(CMD58, 0) == 0)
                {
                    for (i = 0; i < 4; i++)
                    {
                        ocr[i] = sdSpiReadWriteByte(0xFF);
                    }

                    if (ocr[0] & 0x40)
                    {
                        gSdType = SD_TYPE_V2HC;
                    }
                    else
                    {
                        gSdType = SD_TYPE_V2;
                    }
                }
            }
        }
        else
        {
            res = sdSendCmd(ACMD41, 0);
            retry = 1000;

            if (res <= 1)
            {
                gSdType = SD_TYPE_V1;
                cmd = ACMD41;
            }
            else
            {
                gSdType = SD_TYPE_MMC;
                cmd = CMD1;
            }

            do
            {
                res = sdSendCmd(cmd, 0);
            } while (res && retry--);

            if (retry == 0 || sdSendCmd(CMD16, SD_BLOCK_SIZE) != 0)
            {
                gSdType = SD_TYPE_ERR;
            }
        }
    }

    sdDeselect();

    if (gSdType)
    {
        res = SD_OK;
        sdSpiIoInit(SPEED_HIGH);
        printf("SD card capacity: %d MB\r\n", sdGetBlockCount() >> 11);
    }
    else
    {
        res = SD_ERROR;
        printf("No SD card.\r\n");
    }

    return res;
}

uint8_t sdReadDisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t res;
    long long lsaddr = saddr;

    if (gSdType != SD_TYPE_V2HC)
    {
        lsaddr <<= 9;
    }

    if (cnt == 1)
    {
        res = sdSendCmd(CMD17, lsaddr);

        if (res == 0)
        {
            res = sdReceiveData(pbuf, SD_BLOCK_SIZE);
        }
    }
    else
    {
        res = sdSendCmd(CMD18, lsaddr);

        do
        {
            res = sdReceiveData(pbuf, SD_BLOCK_SIZE);
            pbuf += SD_BLOCK_SIZE;
        } while (--cnt && res == 0);

        sdSendCmd(CMD12, 0);
    }

    sdDeselect();
    return res;
}

uint8_t sdWriteDisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t retry = 20;
    uint8_t res;
    long long lsaddr = saddr;

    if (gSdType != SD_TYPE_V2HC)
    {
        lsaddr <<= 9;
    }

    if (cnt == 1)
    {
        res = sdSendCmd(CMD24, lsaddr);

        if (res == 0)
        {
            res = sdSendBlock(pbuf, 0xFE);
        }
    }
    else
    {
        if (gSdType != SD_TYPE_MMC)
        {
            do
            {
                res = sdSendCmd(ACMD23, cnt);
            }while(res && retry--);
        }

        res = sdSendCmd(CMD25, lsaddr);

        if (res == 0)
        {
            do
            {
                res = sdSendBlock(pbuf, 0xFC);
                pbuf += SD_BLOCK_SIZE;
            } while (--cnt && res == 0);

            res = sdSendBlock(0, 0xFD);
        }
    }

    sdDeselect();
    return res;
}
