#include "cameraDrv.h"
#include "sctdef.h"

AP_PLAT_COMMON_DATA camI2cCfg_t bf30a2_1sdrRegInfo[] = 
{
    {0xf2,0x01},//soft reset
    {0xcf,0xb0},//power up
    {0x12,0x20},//MTK:20 ZX:10 RDA:40
    {0x15,0x80},
    {0x6b,0x71},
    {0x00,0x40},
    {0x04,0x00},
    {0x06,0x26},
    {0x08,0x07},
    {0x1c,0x12},
    {0x20,0x20},
    {0x21,0x20},
    {0x34,0x02},
    {0x35,0x02},
    {0x36,0x21},
    {0x37,0x13},
    {0xca,0x23},
    {0xcb,0x22},
    {0xcc,0x89},
    {0xcd,0x4c},
    {0xce,0x6b},
    {0xa0,0x8e},
    {0x01,0x1b},
    {0x02,0x1d},
    {0x13,0x08},
    {0x87,0x13},
    {0x8b,0x08},
    {0x70,0x1f},
    {0x71,0x40},
    {0x72,0x0a},
    {0x73,0x62},
    {0x74,0xa2},
    {0x75,0xbf},
    {0x76,0x02},
    {0x77,0xcc},
    {0x40,0x32},
    {0x41,0x28},
    {0x42,0x26},
    {0x43,0x1d},
    {0x44,0x1a},
    {0x45,0x14},
    {0x46,0x11},
    {0x47,0x0f},
    {0x48,0x0e},
    {0x49,0x0d},
    {0x4B,0x0c},
    {0x4C,0x0b},
    {0x4E,0x0a},
    {0x4F,0x09},
    {0x50,0x09},
    {0x24,0x50},
    {0x25,0x36},
    {0x80,0x00},
    {0x81,0x20},
    {0x82,0x40},
    {0x83,0x30},
    {0x84,0x50},
    {0x85,0x30},
    {0x86,0xD8},
    {0x89,0x45},
    {0x8a,0x33},
    {0x8f,0x81},
    {0x91,0xff},
    {0x92,0x08},
    {0x94,0x82},
    {0x95,0xfd},
    {0x9a,0x20},
    {0x9e,0xbc},
    {0xf0,0x8f},
    {0x51,0x06},
    {0x52,0x25},
    {0x53,0x2b},
    {0x54,0x0F},
    {0x57,0x2A},
    {0x58,0x22},
    {0x59,0x2c},
    {0x23,0x33},
    {0xa1,0x93},
    {0xa2,0x0f},
    {0xa3,0x2a},
    {0xa4,0x08},
    {0xa5,0x26},
    {0xa7,0x80},
    {0xa8,0x80},
    {0xa9,0x1e},
    {0xaa,0x19},
    {0xab,0x18},
    {0xae,0x50},
    {0xaf,0x04},
    {0xc8,0x10},
    {0xc9,0x15},
    {0xd3,0x0c},
    {0xd4,0x16},
    {0xee,0x06},
    {0xef,0x04},
    {0x55,0x34},
    {0x56,0x9c},
    {0xb1,0x98},
    {0xb2,0x98},
    {0xb3,0xc4},
    {0xb4,0x0C},
    {0xa0,0x8f},
    {0x13,0x07},
};

uint16_t bf30a2GetRegCnt(char* regName)
{
    if (strcmp(regName, "bf30a2_1sdr") == 0)
    {
        return (sizeof(bf30a2_1sdrRegInfo) / sizeof(bf30a2_1sdrRegInfo[0]));
    }

    
    return 0;
}


