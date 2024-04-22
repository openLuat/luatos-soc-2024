/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    plat_config.c
 * Description:  platform configuration source file
 * History:
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "ec7xx.h"
#include "plat_config.h"
#include "exception_process.h"
#include "sctdef.h"
#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
#include "debug_trace.h"
#include "common.h"
#else
#include DEBUG_LOG_HEADER_FILE
#include "osasys.h"

#define PLAT_CONFIG_FS_ENABLE
#endif

#define PLATCONFIG_TEXT_SECTION     SECTION_DEF_IMPL(.sect_platconfig_text)
#define PLATCONFIG_RODATA_SECTION   SECTION_DEF_IMPL(.sect_platconfig_rodata)
#define PLATCONFIG_DATA_SECTION     SECTION_DEF_IMPL(.sect_platconfig_data)
#define PLATCONFIG_BSS_SECTION      SECTION_DEF_IMPL(.sect_platconfig_bss)



/** \brief config file version
 *  \note when the order of struct \ref plat_config_fs_t and \ref plat_config_raw_flash_t field has changed,
 *        for example, from 1,2,3 to 1,3,2, update version to refresh flash,
 *        in other cases(add more fields or remove some fields from struct \ref plat_config_fs_t), it's not a must to update
 */

// external API declarations
extern uint8_t FLASH_eraseSafe(uint32_t SectorAddress, uint32_t Size);
extern uint8_t FLASH_writeSafe(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint8_t FLASH_eraseSector(uint32_t BlockAddress);
extern uint8_t FLASH_write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint8_t getOSState(void);


/**
  \fn           uint8_t BSP_CalcCrcValue(const uint8_t *buf, uint16_t bufSize)
  \brief        Calculate the "CRC" value of data buffer
  \param[in]    buf         buffer pointer
  \param[in]    bufSize     buffer size
  \returns      crcValue
*/
PLAT_BL_CIRAM_FLASH_TEXT static uint8_t BSP_CalcCrcValue(const uint8_t *buf, uint16_t bufSize)
{
    uint32_t i = bufSize;
    uint32_t a = 1, b = 0;

    EC_ASSERT(buf != NULL && bufSize > 0, buf, bufSize, 0);

    for (i = bufSize; i > 0; )
    {
        a += (uint32_t)(buf[--i]);
        b += a;
    }

    return (uint8_t)(((a>>24)&0xFF)^((a>>16)&0xFF)^((a>>8)&0xFF)^((a)&0xFF)^
                     ((b>>24)&0xFF)^((b>>16)&0xFF)^((b>>8)&0xFF)^((b)&0xFF)^
                     (bufSize&0xFF));
}


#ifdef PLAT_CONFIG_FS_ENABLE
PLATCONFIG_BSS_SECTION static uint8_t          g_fsPlatConfigInitFlag = 0;
PLATCONFIG_BSS_SECTION static plat_config_fs_t g_fsPlatConfig;

#if (FS_PLAT_CONFIG_FILE_CURRENT_VERSION==0)
/**
  \fn           void BSP_SetDefaultFsPlatConfig(void)
  \brief        set default value of "g_fsPlatConfig"
  \return       void
*/
static void BSP_SetDefaultFsPlatConfig(void)
{
    g_fsPlatConfigInitFlag = 1;

    memset(&g_fsPlatConfig, 0x0, sizeof(g_fsPlatConfig));
    g_fsPlatConfig.atPortBaudRate = 115200;
}

void BSP_LoadPlatConfigFromFs(void)
{
    OSAFILE fp = PNULL;
    UINT32  readCount = 0;
    UINT8   crcCheck = 0;
    config_file_header_t fileHeader;

    /*
     * open NVM file
     */
    fp = OsaFopen("plat_config", "rb");   //read only
    if (fp == PNULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_1, P_ERROR,
                    "Can't open 'plat_config' file, use the defult value");

        BSP_SetDefaultFsPlatConfig();
        BSP_SavePlatConfigToFs();

        return;
    }

    /*
     * read file header
     */
    readCount = OsaFread(&fileHeader, sizeof(config_file_header_t), 1, fp);
    if (readCount != 1)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_2, P_ERROR,
                    "Can't read 'plat_config' file header, use the defult value");

        OsaFclose(fp);

        BSP_SetDefaultFsPlatConfig();
        BSP_SavePlatConfigToFs();

        return;
    }

    /*
     * read file body, check validation and handle compatiblity issue
     */
    if(fileHeader.version != FS_PLAT_CONFIG_FILE_CURRENT_VERSION)
    {
        if(fileHeader.version == 0)
        {
        }
        // handle future version below
        else if(0)
        {

        }
        else
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_5, P_ERROR,
                        "'plat_config' version:%d not right, use the defult value", fileHeader.version);

            OsaFclose(fp);

            BSP_SetDefaultFsPlatConfig();
            BSP_SavePlatConfigToFs();
        }
    }
    else
    {
        if(fileHeader.fileBodySize != sizeof(g_fsPlatConfig))
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_3, P_ERROR,
                        "'plat_config' version:%d file body size not right: (%u/%u), use the defult value",
                        fileHeader.version, fileHeader.fileBodySize, sizeof(plat_config_fs_t));

            OsaFclose(fp);

            BSP_SetDefaultFsPlatConfig();
            BSP_SavePlatConfigToFs();
        }
        else
        {
            readCount = OsaFread(&g_fsPlatConfig, sizeof(g_fsPlatConfig), 1, fp);
            crcCheck = BSP_CalcCrcValue((uint8_t *)&g_fsPlatConfig, sizeof(g_fsPlatConfig));

            OsaFclose(fp);

            if (readCount != 1 || crcCheck != fileHeader.checkSum)
            {
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_4, P_ERROR,
                            "Can't read 'plat_config' version:%d file body, or body not right, (%u/%u), use the defult value",
                            fileHeader.version, crcCheck, fileHeader.checkSum);

                BSP_SetDefaultFsPlatConfig();
                BSP_SavePlatConfigToFs();
            }
            else
            {
                g_fsPlatConfigInitFlag = 1;
            }
        }

    }

    return;
}
#endif

#if (FS_PLAT_CONFIG_FILE_CURRENT_VERSION>=1)




#define PLAT_CFG_SCALE_CONTROL_SIZE()  (sizeof(g_fsPlatConfig.scaleAreaStartMark) + \
                                                                    sizeof(g_fsPlatConfig.scaleAreaEndMark) + \
                                                                    sizeof(g_fsPlatConfig.recScaleAreaGrpNum) + \
                                                                    sizeof(g_fsPlatConfig.recPrevArAllSzForUpg) + \
                                                                    sizeof(g_fsPlatConfig.scaleAreaAllSize))

#define PLAT_CFG_SCALE_CTRL_TAIL_SIZE()    (sizeof(g_fsPlatConfig.scaleAreaEndMark) + \
                                                                    sizeof(g_fsPlatConfig.recScaleAreaGrpNum) + \
                                                                    sizeof(g_fsPlatConfig.recPrevArAllSzForUpg) + \
                                                                    sizeof(g_fsPlatConfig.scaleAreaAllSize))



//CHECK the size align to grp unit
SIZE_OF_TYPE_EQUAL_TO_SZX(plat_cfg_sc_grp0_all_data_st, SCALE_AREA_GRP_UNIT_SIZE)

//CHECK the size of plat_config_fs_t legal
SIZE_OF_TYPE_EQUAL_TO_CALX(plat_config_fs_t, PLAT_CFG_FIX_BASE_SIZE+PLAT_CFG_SCALE_CONTROL_SIZE()+SCALE_AREA_GRP_UNIT_SIZE)


#define PLAT_CFG_SCALE_LOAD_FAST_PASS 1

#define PLAT_CFG_SCALE_LOAD_DEFAULT_ERR 2

#define PLAT_CFG_SCALE_LOAD_DWNGRADE_ERR 3

#define PLAT_CFG_SCALE_LOAD_SEEK_ERR 4
#define PLAT_CFG_SCALE_LOAD_READ_ERR 5

#define PLAT_CFG_SCALE_LOAD_CRC_ERR 6

#define PLAT_CFG_SCALE_LOAD_BASESZ_ERR 7
#define PLAT_CFG_SCALE_LOAD_SCALESZ_ERR 8

#define PLAT_CFG_SCALE_LOAD_MARKSTART_ERR 9
#define PLAT_CFG_SCALE_LOAD_MARKEND_ERR 10



static void BSP_SetFsPlatCfgScalArCtrl(void)
{
    uint16_t cur_fixedBaseSize;
    uint16_t cur_scaleAreaAllSize;
    cur_fixedBaseSize = (uint8_t*)(&(g_fsPlatConfig.scaleAreaStartMark)) - (uint8_t*)(&(g_fsPlatConfig));
    cur_scaleAreaAllSize = sizeof(g_fsPlatConfig) -cur_fixedBaseSize;        


    g_fsPlatConfig.scaleAreaStartMark = PLAT_CFG_SCALE_START_MARK;
    
    //scalable area pure data already copied when upgrade or download, do not overwrited here
    //g_fsPlatConfig.usbNetAdaptResult = 0;    
    g_fsPlatConfig.scaleAreaEndMark = PLAT_CFG_SCALE_END_MARK;    
    //g_fsPlatConfig.scaleAreaVerNum = 0;

    g_fsPlatConfig.scaleAreaEndMark = PLAT_CFG_SCALE_END_MARK;    
    g_fsPlatConfig.scaleAreaAllSize = cur_scaleAreaAllSize;    
    
    //record previous scaleAreaAllSize before update to current plat cfg scalable area size
    //current g_fsPlatConfig.scaleAreaAllSize == SCALE_AREA_SCALE_UNIT_SIZE, default0, no need set
    //if (g_fsPlatConfig.scaleAreaAllSize >SCALE_AREA_SCALE_UNIT_SIZE)
    //{
    //    g_fsPlatConfig.recPrevArAllSzForUpg = g_fsPlatConfig.scaleAreaAllSize-SCALE_AREA_SCALE_UNIT_SIZE;
    //}    
}


#define CALC_BUF_SIZE 64
PLAT_BL_CIRAM_FLASH_TEXT static uint8_t BSP_RdAndCalcCrcValue(OSAFILE fp, config_file_header_t *pfHead)
{
    UINT32  readCount = 0;
    uint8_t Crc8;
    uint32_t i ;
    uint16_t bufSize;
    uint32_t a = 1, b = 0;
    uint16_t RestCalDataSz = pfHead->fileBodySize;
    uint8_t buf[CALC_BUF_SIZE];

    readCount = OsaFread(&buf, sizeof(buf), 1, fp);
    while(RestCalDataSz>0)
    {
        if (RestCalDataSz >= CALC_BUF_SIZE)
        {
            bufSize = CALC_BUF_SIZE;
        }
        else
        {
            bufSize = RestCalDataSz;
         }
        RestCalDataSz -= CALC_BUF_SIZE;
        if (OsaFseek(fp, sizeof(config_file_header_t)+RestCalDataSz, SEEK_SET)!=0)
        {
            return PLAT_CFG_SCALE_LOAD_SEEK_ERR;
        }        
        
        readCount = OsaFread(&buf, bufSize, 1, fp);
        if (readCount!=1)
        {
            return PLAT_CFG_SCALE_LOAD_READ_ERR;
        }     
        
        for (i = bufSize; i > 0; )
        {
            a += (uint32_t)(buf[--i]);
            b += a;
        }
        
    }
    
    //for (i = bufSize; i > 0; )
    //{
    //    a += (uint32_t)(buf[--i]);
    //    b += a;
    //}
    Crc8 = (uint8_t)(((a>>24)&0xFF)^((a>>16)&0xFF)^((a>>8)&0xFF)^((a)&0xFF)^
                     ((b>>24)&0xFF)^((b>>16)&0xFF)^((b>>8)&0xFF)^((b)&0xFF)^
                     (bufSize&0xFF));
    if (pfHead->checkSum!=Crc8)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_RdAndCalcCrcValue_1, P_ERROR,
                    "plat_config crc not match, (%u/%u), ", Crc8, pfHead->checkSum);              
        return PLAT_CFG_SCALE_LOAD_CRC_ERR;
    }
    return 0;
}


/**
  \fn           void BSP_SetDefaultFsPlatConfig(void)
  \brief        set default value of "g_fsPlatConfig"
  \return       void
*/
static void BSP_SetDefaultFsPlatConfig(void)
{
    //uint16_t cur_fixedBaseSize;
    //uint16_t cur_scaleAreaAllSize;

    g_fsPlatConfigInitFlag = 1;

    memset(&g_fsPlatConfig, 0x0, sizeof(g_fsPlatConfig));
    //cur_fixedBaseSize = (uint8_t*)(&(g_fsPlatConfig.scaleAreaStartMark)) - (uint8_t*)(&(g_fsPlatConfig));
    //cur_scaleAreaAllSize = sizeof(g_fsPlatConfig) -cur_fixedBaseSize;    

     g_fsPlatConfig.atPortBaudRate = 115200;
    BSP_SetFsPlatCfgScalArCtrl();
    //g_fsPlatConfig.scaleAreaStartMark = PLAT_CFG_SCALE_START_MARK;
    //g_fsPlatConfig.usbNetAdaptResult = 0;    
    //g_fsPlatConfig.scaleAreaEndMark = PLAT_CFG_SCALE_END_MARK;    
    //g_fsPlatConfig.scaleAreaVerNum = 0;
    
    
    //g_fsPlatConfig.scaleAreaEndMark = PLAT_CFG_SCALE_END_MARK;    
    
    //g_fsPlatConfig.scaleAreaAllSize = cur_scaleAreaAllSize;
    //record previous scaleAreaAllSize before update to current plat cfg scalable area size
    //current g_fsPlatConfig.scaleAreaAllSize == SCALE_AREA_SCALE_UNIT_SIZE, default0, no need set
    //if (g_fsPlatConfig.scaleAreaAllSize >SCALE_AREA_SCALE_UNIT_SIZE)
    //{
    //    g_fsPlatConfig.recPrevArAllSzForUpg = g_fsPlatConfig.scaleAreaAllSize-SCALE_AREA_SCALE_UNIT_SIZE;
    //}

}

void BSP_LoadPlatConfigFromFs(void)
{
    OSAFILE fp = PNULL;
    UINT32  readCount = 0;
    UINT8   crcCheck = 0;
    config_file_header_t fileHeader;

    uint16_t cur_fixedBaseSize;
    uint16_t cur_scaleAreaAllSize;

    uint16_t loaded_scaleAreaAllSize;
    uint16_t loaded_scaleAreaStartMark;
    uint16_t loaded_scaleAreaEndMark;
    
    uint16_t loaded_fixedBaseSize;
    uint16_t loaded_recPrevArAllSzForUpg;
    
    uint8_t loadStat = PLAT_CFG_SCALE_LOAD_DEFAULT_ERR;
    
    uint8_t* p_g_fsPlatConfig;
    
    cur_fixedBaseSize = (uint8_t*)(&(g_fsPlatConfig.scaleAreaStartMark)) - (uint8_t*)(&(g_fsPlatConfig));
    cur_scaleAreaAllSize = sizeof(g_fsPlatConfig) -cur_fixedBaseSize;


    BSP_SetDefaultFsPlatConfig();

    /*
     * open NVM file
     */
    fp = OsaFopen("plat_config", "rb");   //read only
    if (fp == PNULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_1, P_ERROR,
                    "Can't open 'plat_config' file, use the defult value");
        //Called
        //BSP_SetDefaultFsPlatConfig();
        BSP_SavePlatConfigToFs();

        return;
    }

    /*
     * read file header
     */
    readCount = OsaFread(&fileHeader, sizeof(config_file_header_t), 1, fp);
    if (readCount != 1)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_2, P_ERROR,
                    "Can't read 'plat_config' file header, use the defult value");

        OsaFclose(fp);
        //Called at start
        //BSP_SetDefaultFsPlatConfig();
        BSP_SavePlatConfigToFs();

        return;
    }


    /*
     * read file body, check validation and handle compatiblity issue
     */
    if(fileHeader.version != FS_PLAT_CONFIG_FILE_CURRENT_VERSION)
    {
        if(fileHeader.version == 0)
        {
            //memset(&g_fsPlatConfig, 0x0, sizeof(g_fsPlatConfig));
            if (cur_fixedBaseSize== fileHeader.fileBodySize)
            {
                //cur seek pos sizeof(config_file_header_t)
                readCount = OsaFread(&g_fsPlatConfig, fileHeader.fileBodySize, 1, fp);
                
                if (readCount!=1)
                {
                    //unlikely, reload all default data
                    BSP_SetDefaultFsPlatConfig();
                } 
                else
                {
                    //already called in BSP_SetDefaultFsPlatConfig
                    //BSP_SetFsPlatCfgScalArCtrl();
                    //g_fsPlatConfig.scaleAreaStartMark =  PLAT_CFG_SCALE_START_MARK;
                    //g_fsPlatConfig.scaleAreaEndMark =  PLAT_CFG_SCALE_END_MARK;
                    //the scaleAreaAllSize should final set to cur_scaleAreaAllSize
                    //g_fsPlatConfig.scaleAreaAllSize = cur_scaleAreaAllSize;
                    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_3, P_INFO,
                                "'plat_config' version 0, upgrade to plat config scale version %d", FS_PLAT_CONFIG_FILE_CURRENT_VERSION);                    
                }
                
            }
            //Called at start
            //else {
            //    BSP_SetDefaultFsPlatConfig();
            //}
            
            OsaFclose(fp);
            BSP_SavePlatConfigToFs();                
        }
        // handle future version below
        else if(0)
        {

        }
        else
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_4, P_ERROR,
                        "'plat_config' version:%d not right, use the defult value", fileHeader.version);

            OsaFclose(fp);
            //Called at start
            //BSP_SetDefaultFsPlatConfig();
            BSP_SavePlatConfigToFs();
        }
    }
    else
    {
        //scalable update
        
        //Called at start
        //BSP_SetDefaultFsPlatConfig();
        
        //cur_fixedBaseSize = (uint8_t*)(&(g_fsPlatConfig.scalabeAreaMark)) - (uint8_t*)(&(g_fsPlatConfig));
        //pure scalable data area = sizeof(g_fsPlatConfig) - cur_fixedBaseSize-  PLAT_CFG_SCALE_CONTROL_SIZE(); 

        do {

            
            if (fileHeader.fileBodySize>sizeof(g_fsPlatConfig))
            {

                // Dwngrade crc check
                if (BSP_RdAndCalcCrcValue(fp, &fileHeader) != 0)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_CRC_ERR;      
                    break;
                }                            
                
                ////////////////////////////////branch for downgrade start ////////////////////////////////////////////////////////////////////////////////////
                // scaleAreaStartMark, scaleAreaPureData0-scaleAreaPureDataX, scaleAreaEndMark, recScaleAreaGrpNum , recPrevArAllSzForUpg,scaleAreaAllSize
                 //seek to loaded_scaleAreaEndMark
                if (OsaFseek(fp, fileHeader.fileBodySize    \
                                            - sizeof(g_fsPlatConfig.scaleAreaEndMark)  \
                                            -sizeof(g_fsPlatConfig.recScaleAreaGrpNum)  \
                                            -sizeof(loaded_recPrevArAllSzForUpg)  \
                                            -sizeof(loaded_scaleAreaAllSize),  \
                                             SEEK_SET)!=0)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_SEEK_ERR;
                    break;
                }


                readCount = OsaFread(&loaded_scaleAreaEndMark, sizeof(loaded_scaleAreaEndMark), 1, fp);
                if (readCount!=1)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_READ_ERR;
                    break;
                }                     

                if (loaded_scaleAreaEndMark!=PLAT_CFG_SCALE_END_MARK)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_MARKEND_ERR;
                    break;                  
                }            

                if (OsaFseek(fp, fileHeader.fileBodySize    \
                                            -sizeof(loaded_recPrevArAllSzForUpg)  \
                                            -sizeof(loaded_scaleAreaAllSize),  \
                                             SEEK_SET)!=0)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_SEEK_ERR;
                    break;
                }
                
                readCount = OsaFread(&loaded_recPrevArAllSzForUpg, sizeof(loaded_recPrevArAllSzForUpg), 1, fp);
                if (readCount!=1)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_READ_ERR;
                    break;
                }     
                readCount = OsaFread(&loaded_scaleAreaAllSize, sizeof(loaded_scaleAreaAllSize), 1, fp);
                if (readCount!=1)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_READ_ERR;
                    break;
                }                     
                if ((loaded_recPrevArAllSzForUpg!=cur_scaleAreaAllSize) ||(loaded_recPrevArAllSzForUpg > loaded_scaleAreaAllSize))
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_DWNGRADE_ERR;
                    break;
                }

                loaded_fixedBaseSize = fileHeader.fileBodySize -loaded_scaleAreaAllSize;
                if (loaded_fixedBaseSize!=cur_fixedBaseSize)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_BASESZ_ERR;
                    break;
                }                
                
                //branch for dowgrade, seek to sizeof(config_file_header_t)
                if (OsaFseek(fp, sizeof(config_file_header_t), SEEK_SET)!=0)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_SEEK_ERR;
                    break;
                }

                // loaded layout loaded_fixedBaseSize, loaded_scaleAreaStartMark, loaded_scaleAreaPureData0-loaded_scaleAreaPureDataX
                //    real_read_X=cur_fixedBaseSize-PLAT_CFG_SCALE_CONTROL_SIZE 
                //
                    
                readCount = OsaFread(&g_fsPlatConfig, cur_fixedBaseSize + cur_scaleAreaAllSize - PLAT_CFG_SCALE_CTRL_TAIL_SIZE(), 1, fp);
                if (readCount!=1)
                {
                    loadStat = PLAT_CFG_SCALE_LOAD_READ_ERR;
                    break;
                }     

               if (g_fsPlatConfig.scaleAreaStartMark!=PLAT_CFG_SCALE_START_MARK)
               {
                   loadStat = PLAT_CFG_SCALE_LOAD_MARKSTART_ERR;
                   break;                  
               }

               BSP_SetFsPlatCfgScalArCtrl();
               loadStat = 0;
                //all valid data will be loaded to g_fsPlatConfig
                //fileHeader.fileBodySize == loaded_fixedBaseSize + loaded_scaleAreaAllSize;
                //sizeof(g_fsPlatConfig) ==  cur_fixedBaseSize +  cur_scaleAreaAllSize
               break;
                
                ////////////////////////////////branch for downgrade end ///////////////////////////////////////////////////////////////////////////////
            }

            ////////////////////////////////branch for upgrade end ////////////////////////////////////////////////////////////////////////////////////
            //cur seek pos sizeof(config_file_header_t)
            readCount = OsaFread(&g_fsPlatConfig, fileHeader.fileBodySize, 1, fp);
            if (readCount!=1)
            {
                loadStat = PLAT_CFG_SCALE_LOAD_READ_ERR;
                break;
            }
            p_g_fsPlatConfig = (uint8_t*)(&g_fsPlatConfig);

            crcCheck = BSP_CalcCrcValue(p_g_fsPlatConfig, fileHeader.fileBodySize);


            if (crcCheck != fileHeader.checkSum)
            {
                loadStat = PLAT_CFG_SCALE_LOAD_CRC_ERR;
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_6, P_ERROR,
                            "Can't read 'plat_config' crc not match, (%u/%u), ", crcCheck, fileHeader.checkSum);                
                break;
            }            

            if(fileHeader.fileBodySize == sizeof(g_fsPlatConfig))
            {
                loadStat = PLAT_CFG_SCALE_LOAD_FAST_PASS;
                break;
            }

            //init loaded_scaleAreaAllSize from p_g_fsPlatConfig +fileBodySize - 2 bytes
            loaded_scaleAreaAllSize = *((uint16_t*)(p_g_fsPlatConfig + fileHeader.fileBodySize- sizeof(g_fsPlatConfig.scaleAreaAllSize)));
            loaded_fixedBaseSize = fileHeader.fileBodySize -loaded_scaleAreaAllSize;
            if (loaded_fixedBaseSize!=cur_fixedBaseSize)
            {
                loadStat = PLAT_CFG_SCALE_LOAD_BASESZ_ERR;
                break;
            }
            
            if(loaded_scaleAreaAllSize< PLAT_CFG_SCALE_CONTROL_SIZE()) 
            {
                loadStat = PLAT_CFG_SCALE_LOAD_SCALESZ_ERR;
                break;                
            }

            //start mark loc same to cur g_fsPlatConfig.scaleAreaStartMark
            loaded_scaleAreaStartMark = g_fsPlatConfig.scaleAreaStartMark;
            loaded_scaleAreaEndMark = *((uint16_t*)(p_g_fsPlatConfig + fileHeader.fileBodySize
                                                                            -sizeof(g_fsPlatConfig.scaleAreaEndMark)
                                                                            - sizeof(g_fsPlatConfig.scaleAreaAllSize)));
            
            if (loaded_scaleAreaStartMark!=PLAT_CFG_SCALE_START_MARK)
            {
                loadStat = PLAT_CFG_SCALE_LOAD_MARKSTART_ERR;
                break;                  
            }

            if (loaded_scaleAreaEndMark!=PLAT_CFG_SCALE_END_MARK)
            {
                loadStat = PLAT_CFG_SCALE_LOAD_MARKEND_ERR;
                break;                  
            }

            
            BSP_SetFsPlatCfgScalArCtrl();
            loadStat = 0;
            
        }while(0);

        if(loadStat==0)
        {
            //update
            //give cond: fileHeader.fileBodySize<=sizeof(g_fsPlatConfig) 
            //           cur_fixedBaseSize == loaded_fixedBaseSize
            //           sizeof(g_fsPlatConfig) == cur_scaleAreaAllSize + cur_fixedBaseSize;
            //           fileHeader.fileBodySize == (loaded_scaleAreaAllSize + loaded_fixedBaseSize);
            //if cur_scaleAreaAllSize<loaded_scaleAreaAllSize 
            //      means fileHeader.fileBodySize<sizeof(g_fsPlatConfig)        
            //else 
            //        means cur_scaleAreaAllSize==loaded_scaleAreaAllSize 
            //            and fileHeader.fileBodySize==sizeof(g_fsPlatConfig)     
            //if (cur_scaleAreaAllSize<loaded_scaleAreaAllSize)
            //{

                //g_fsPlatConfig.scaleAreaEndMark =  PLAT_CFG_SCALE_END_MARK;
                //the scaleAreaAllSize should final set to cur_scaleAreaAllSize
                //g_fsPlatConfig.scaleAreaAllSize = cur_scaleAreaAllSize;
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_7, P_INFO,
                            "plat_config version:%d, upd scale area size  from loaded %d to %d",
                            fileHeader.version, loaded_scaleAreaAllSize, cur_scaleAreaAllSize);
                OsaFclose(fp);
                BSP_SavePlatConfigToFs();                      
            //}
            //else {
                //crcCheck, loaded_fixedBaseSize, loaded_scaleAreaAllSize, loaded_scaleAreaStartMark,  loaded_scaleAreaEndMark
                //all check pass, no change
                //Called at start
                //BSP_SetDefaultFsPlatConfig();                
            //    g_fsPlatConfigInitFlag = 1;
            //}
        }
        else
        {
            if(PLAT_CFG_SCALE_LOAD_FAST_PASS!=loadStat)
            {
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_LoadPlatConfig_8, P_ERROR,
                            "Can't load 'plat_config' version:%d, load_error %d, use the defult value",
                            fileHeader.version, loadStat);
                //reload all default data
                BSP_SetDefaultFsPlatConfig();
                OsaFclose(fp);
                BSP_SavePlatConfigToFs();             
            }
        }

    }

    return;
}

#endif

void BSP_SetFsPorDefaultValue(void)
{
    bool fsCfgChanged = false;
    if(g_fsPlatConfig.ecSclkCfg != 0)
    {
        g_fsPlatConfig.ecSclkCfg = 0;
        fsCfgChanged = true;
    }
    if(fsCfgChanged)
    {
        BSP_SavePlatConfigToFs();
    }
}



void BSP_SavePlatConfigToFs(void)
{
    OSAFILE fp = PNULL;
    UINT32  writeCount = 0;
    config_file_header_t fileHeader;

    /*
     * open the config file
    */
    fp = OsaFopen("plat_config", "wb");   //write & create
    if (fp == PNULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SavePlatConfig_1, P_ERROR,
                    "Can't open/create 'plat_config' file, save plat_config failed");

        return;
    }

    /*
     * write the header
    */
    fileHeader.fileBodySize   = sizeof(g_fsPlatConfig);
    fileHeader.version        = FS_PLAT_CONFIG_FILE_CURRENT_VERSION;
    fileHeader.checkSum       = BSP_CalcCrcValue((uint8_t *)&g_fsPlatConfig, sizeof(g_fsPlatConfig));

    writeCount = OsaFwrite(&fileHeader, sizeof(config_file_header_t), 1, fp);
    if (writeCount != 1)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SavePlatConfig_2, P_ERROR,
                   "Write 'plat_config' file header failed");

        OsaFclose(fp);
        return;
    }

    /*
     * write the file body
    */
    writeCount = OsaFwrite(&g_fsPlatConfig, sizeof(g_fsPlatConfig), 1, fp);
    if (writeCount != 1)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SavePlatConfig_3, P_ERROR,
                   "Write 'plat_config' file body failed");
    }

    OsaFclose(fp);
    return;

}

plat_config_fs_t* BSP_GetFsPlatConfig(void)
{
    return &g_fsPlatConfig;
}

uint32_t BSP_GetFSAssertCount(void)
{
    plat_info_layout_t platInfo;

    // read
    memcpy((uint8_t*)&platInfo, (void*)FLASH_MEM_PLAT_INFO_ADDR, sizeof(plat_info_layout_t));

    return platInfo.fsAssertCount;
}

void BSP_SetFSAssertCount(uint32_t value)
{
    plat_info_layout_t platInfo;

    // read
    memcpy((uint8_t*)&platInfo, (void*)FLASH_MEM_PLAT_INFO_ADDR, sizeof(plat_info_layout_t));

    // modify
    platInfo.fsAssertCount = value;

    // erase
    if(FLASH_eraseSafe(FLASH_MEM_PLAT_INFO_NONXIP_ADDR, FLASH_MEM_PLAT_INFO_SIZE) != 0)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SetFSAssertCount_0, P_ERROR, "Erase flash error!!!");
        return;
    }

    // write back
    if(FLASH_writeSafe((uint8_t*)&platInfo, FLASH_MEM_PLAT_INFO_NONXIP_ADDR, sizeof(plat_info_layout_t)) != 0)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SetFSAssertCount_1, P_ERROR, "Update fsAssertCount value error!!!");
    }

}

#endif

#ifdef __USER_CODE__	//user ctrl wfi mode, rndis/ecm mode
extern uint8_t soc_user_wfi_mode(void);
extern uint8_t soc_user_usb_eth_mode(void);
#endif
PLATCONFIG_BSS_SECTION plat_config_raw_flash_t g_rawFlashPlatConfig;

/**
  \fn           void BSP_SetDefaultRawFlashPlatConfig(void)
  \brief        set default value of "g_rawFlashPlatConfig"
  \return       void
*/
PLAT_BL_CIRAM_FLASH_TEXT static void BSP_SetDefaultRawFlashPlatConfig(void)
{
#ifdef __USER_CODE__

    g_rawFlashPlatConfig.faultAction = 4;//silent anable
    g_rawFlashPlatConfig.startWDT = 1;//start wdt
    g_rawFlashPlatConfig.uartDumpPort = 0xff; // default at port
    g_rawFlashPlatConfig.logControl = 0x2;
    g_rawFlashPlatConfig.uartBaudRate = 6000000;
    g_rawFlashPlatConfig.logLevel = P_DEBUG;
    g_rawFlashPlatConfig.logOwnerAndLevel = 0; //default all owner log level is 0
    g_rawFlashPlatConfig.logPortSel = PLAT_CFG_ULG_PORT_MIX;//default MIX mode
    g_rawFlashPlatConfig.usbCtrl = 0;//all en
    g_rawFlashPlatConfig.usbSlpMask = 0;  // no mask
    g_rawFlashPlatConfig.usbSlpThd = 0;
    g_rawFlashPlatConfig.pwrKeyMode = 0;
    g_rawFlashPlatConfig.usbVBUSModeEn = 0;
    g_rawFlashPlatConfig.usbVBUSWkupPad = 1;
    g_rawFlashPlatConfig.usbNet = soc_user_usb_eth_mode();
    g_rawFlashPlatConfig.usbVcomEnBitMap = 0;
    g_rawFlashPlatConfig.atPortBaudRate = 115200;
    g_rawFlashPlatConfig.fotaUrcPortSel = (PLAT_CFG_FOTA_URC_PORT_UART << 4) | 1;
    g_rawFlashPlatConfig.pmuInCdrx = 1;
    g_rawFlashPlatConfig.slpLimitEn = 0;
    g_rawFlashPlatConfig.slpLimitTime = 0;
    g_rawFlashPlatConfig.wfiMode = soc_user_wfi_mode();
    g_rawFlashPlatConfig.apIdlePercentPrintMode = 0;
    g_rawFlashPlatConfig.cpSlpTest = 0;
#else	//__USER_CODE__
#ifdef SDK_REL_BUILD
    g_rawFlashPlatConfig.faultAction = 4;//silent anable
    g_rawFlashPlatConfig.startWDT = 1;//start wdt
#else
    g_rawFlashPlatConfig.faultAction = 0;
    g_rawFlashPlatConfig.startWDT = 0;
#endif
    g_rawFlashPlatConfig.uartDumpPort = 1; // default at port

    g_rawFlashPlatConfig.logControl = 0x2;

    g_rawFlashPlatConfig.uartBaudRate = 3000000;

    g_rawFlashPlatConfig.logLevel = P_DEBUG;

    g_rawFlashPlatConfig.logOwnerAndLevel = 0; //default all owner log level is 0

    g_rawFlashPlatConfig.logPortSel = PLAT_CFG_ULG_PORT_MIX;//default MIX mode

    g_rawFlashPlatConfig.usbCtrl = 0;//all en

    g_rawFlashPlatConfig.usbSlpMask = 0;  // no mask

    g_rawFlashPlatConfig.usbSlpThd = 0;

    g_rawFlashPlatConfig.pwrKeyMode = 0;

    g_rawFlashPlatConfig.usbVBUSModeEn = 0;

    g_rawFlashPlatConfig.usbVBUSWkupPad = 1;

    g_rawFlashPlatConfig.usbNet = 0;

    g_rawFlashPlatConfig.usbVcomEnBitMap = 0;

#ifdef PLAT_CONFIG_FS_ENABLE
    g_rawFlashPlatConfig.atPortBaudRate = g_fsPlatConfigInitFlag ? g_fsPlatConfig.atPortBaudRate : 115200;
#else
    g_rawFlashPlatConfig.atPortBaudRate = 115200;
#endif
#if 0
    g_rawFlashPlatConfig.fotaUrcPortSel = (PLAT_CFG_FOTA_URC_PORT_USB << 4) | 0;
#else
    g_rawFlashPlatConfig.fotaUrcPortSel = (PLAT_CFG_FOTA_URC_PORT_UART << 4) | 1;
#endif

    g_rawFlashPlatConfig.pmuInCdrx = 1;

    g_rawFlashPlatConfig.slpLimitEn = 0;

    g_rawFlashPlatConfig.slpLimitTime = 0;

    g_rawFlashPlatConfig.wfiMode = 0;

    g_rawFlashPlatConfig.apIdlePercentPrintMode = 0;

    g_rawFlashPlatConfig.cpSlpTest = 0;
#endif //__USER_CODE__
    memset(g_rawFlashPlatConfig.resv, 0x0,PLAT_CFG_RAW_FLASH_RSVD_SIZE);
}

void BSP_SavePlatConfigToRawFlash(void)
{
    plat_info_layout_t platInfo;

    // read
    memcpy((uint8_t*)&platInfo, (void*)FLASH_MEM_PLAT_INFO_ADDR, sizeof(plat_info_layout_t));

    // modify start //
    // header part
    platInfo.header.fileBodySize   = sizeof(plat_config_raw_flash_t);
    platInfo.header.version        = RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION;
    platInfo.header.checkSum       = BSP_CalcCrcValue((uint8_t *)&g_rawFlashPlatConfig, sizeof(g_rawFlashPlatConfig));

    // body part
    platInfo.config = g_rawFlashPlatConfig;
    // modify end //

    // write back

#ifndef FEATURE_BOOTLOADER_PROJECT_ENABLE
    if(1 == getOSState())
    {
        if(FLASH_eraseSafe(FLASH_MEM_PLAT_INFO_NONXIP_ADDR, FLASH_MEM_PLAT_INFO_SIZE) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SavePlatConfigToRawFlash_1, P_ERROR, "Erase flash error!!!");
            return;
        }

        if(FLASH_writeSafe((uint8_t*)&platInfo, FLASH_MEM_PLAT_INFO_NONXIP_ADDR, sizeof(plat_info_layout_t)) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SavePlatConfigToRawFlash_2, P_ERROR, "Save plat config to raw flash error!!!");
        }
    }
    else
#endif
    {
        if(FLASH_eraseSector(FLASH_MEM_PLAT_INFO_NONXIP_ADDR) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SavePlatConfigToRawFlash_3, P_ERROR, "Erase flash error!!!");
            return;
        }

        if(FLASH_write((uint8_t*)&platInfo, FLASH_MEM_PLAT_INFO_NONXIP_ADDR, sizeof(plat_info_layout_t)) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SavePlatConfigToRawFlash_4, P_ERROR, "Save plat config to raw flash error!!!");
        }
    }

}

PLAT_BL_CIRAM_FLASH_TEXT static void BSP_WriteToRawFlash(uint8_t* pBuffer, uint32_t bufferSize)
{
    if(pBuffer == NULL || bufferSize == 0)
    {
        return;
    }

#ifndef FEATURE_BOOTLOADER_PROJECT_ENABLE
    if(1 == getOSState())
    {
        if(FLASH_eraseSafe(FLASH_MEM_PLAT_INFO_NONXIP_ADDR, FLASH_MEM_PLAT_INFO_SIZE) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_WriteToRawFlash_1, P_ERROR, "Erase flash error!!!");
            return;
        }

        if(FLASH_writeSafe(pBuffer, FLASH_MEM_PLAT_INFO_NONXIP_ADDR, bufferSize) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_WriteToRawFlash_2, P_ERROR, "Save plat config to raw flash error!!!");
        }
    }
    else
#endif
    {
        if(FLASH_eraseSector(FLASH_MEM_PLAT_INFO_NONXIP_ADDR) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_WriteToRawFlash_3, P_ERROR, "Erase flash error!!!");
            return;
        }

        if(FLASH_write(pBuffer, FLASH_MEM_PLAT_INFO_NONXIP_ADDR, bufferSize) != 0)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_WriteToRawFlash_4, P_ERROR, "Save plat config to raw flash error!!!");
        }
    }

}

PLAT_BL_CIRAM_FLASH_TEXT void BSP_LoadPlatConfigFromRawFlash(void)
{
    plat_info_layout_t platInfo;
    config_file_header_t header;
    uint32_t fsAssertCount;

    /*
     * read file header
     */
    memcpy((uint8_t*)&header, (void*)FLASH_MEM_PLAT_INFO_ADDR, sizeof(header));

    if(header.version != RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION)
    {
        if(header.version == 0)
        {
            plat_config_raw_flash_v0_t v0Config;

            BSP_SetDefaultRawFlashPlatConfig();

            // migrate from old version
            if(header.fileBodySize == sizeof(plat_config_raw_flash_v0_t))
            {
                memcpy((uint8_t*)&v0Config, (void*)(FLASH_MEM_PLAT_INFO_ADDR + sizeof(config_file_header_t)), sizeof(v0Config));
                memcpy((uint8_t*)&fsAssertCount, (void*)(FLASH_MEM_PLAT_INFO_ADDR + sizeof(config_file_header_t) + sizeof(v0Config)), sizeof(fsAssertCount));

                g_rawFlashPlatConfig.faultAction = v0Config.faultAction;
                g_rawFlashPlatConfig.uartDumpPort = v0Config.uartDumpPort;
                g_rawFlashPlatConfig.startWDT = v0Config.startWDT;
                g_rawFlashPlatConfig.logControl = v0Config.logControl;
                g_rawFlashPlatConfig.uartBaudRate = v0Config.uartBaudRate;
                g_rawFlashPlatConfig.logLevel = v0Config.logLevel;
                g_rawFlashPlatConfig.logPortSel = v0Config.logPortSel;
                g_rawFlashPlatConfig.usbCtrl = v0Config.usbCtrl;
                g_rawFlashPlatConfig.usbSwTrace = v0Config.usbSwTrace;
                g_rawFlashPlatConfig.usbSlpMask = v0Config.usbSlpMask;
                g_rawFlashPlatConfig.usbSlpThd = v0Config.usbSlpThd;
                g_rawFlashPlatConfig.pwrKeyMode = v0Config.pwrKeyMode;

                platInfo.header.fileBodySize   = sizeof(plat_config_raw_flash_t);
                platInfo.header.version        = RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION;
                platInfo.header.checkSum       = BSP_CalcCrcValue((uint8_t *)&g_rawFlashPlatConfig, sizeof(g_rawFlashPlatConfig));

                platInfo.config = g_rawFlashPlatConfig;

                platInfo.fsAssertCount = fsAssertCount;

            }
            // version matches but size is wrong, use default value
            else
            {
                platInfo.header.fileBodySize   = sizeof(plat_config_raw_flash_t);
                platInfo.header.version        = RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION;
                platInfo.header.checkSum       = BSP_CalcCrcValue((uint8_t *)&g_rawFlashPlatConfig, sizeof(g_rawFlashPlatConfig));

                platInfo.config = g_rawFlashPlatConfig;
                platInfo.fsAssertCount = 0;
            }

            BSP_WriteToRawFlash((uint8_t*)&platInfo, sizeof(platInfo));

        }
        else if(0)
        {
            // handle future version
        }
        else
        {
            // version is invalid

            BSP_SetDefaultRawFlashPlatConfig();

            platInfo.header.fileBodySize   = sizeof(plat_config_raw_flash_t);
            platInfo.header.version        = RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION;
            platInfo.header.checkSum       = BSP_CalcCrcValue((uint8_t *)&g_rawFlashPlatConfig, sizeof(g_rawFlashPlatConfig));

            platInfo.config = g_rawFlashPlatConfig;
            platInfo.fsAssertCount = 0;

            BSP_WriteToRawFlash((uint8_t*)&platInfo, sizeof(platInfo));

        }
    }
    else
    {
        // file body check
        memcpy((uint8_t*)&g_rawFlashPlatConfig, (void*)(FLASH_MEM_PLAT_INFO_ADDR + sizeof(config_file_header_t)), sizeof(g_rawFlashPlatConfig));

        if((header.fileBodySize != sizeof(plat_config_raw_flash_t)) ||
           (header.checkSum != BSP_CalcCrcValue((uint8_t *)&g_rawFlashPlatConfig, sizeof(g_rawFlashPlatConfig))))
        {

            BSP_SetDefaultRawFlashPlatConfig();

            platInfo.header.fileBodySize   = sizeof(plat_config_raw_flash_t);
            platInfo.header.version        = RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION;
            platInfo.header.checkSum       = BSP_CalcCrcValue((uint8_t *)&g_rawFlashPlatConfig, sizeof(g_rawFlashPlatConfig));

            platInfo.config = g_rawFlashPlatConfig;
            platInfo.fsAssertCount = 0;

            BSP_WriteToRawFlash((uint8_t*)&platInfo, sizeof(platInfo));
        }
        else
        {
        #ifdef PLAT_CONFIG_FS_ENABLE
            if(g_fsPlatConfigInitFlag && g_fsPlatConfig.atPortBaudRate != g_rawFlashPlatConfig.atPortBaudRate)
            {
                g_rawFlashPlatConfig.atPortBaudRate = g_fsPlatConfig.atPortBaudRate;

                memcpy((uint8_t*)&fsAssertCount, (void*)(FLASH_MEM_PLAT_INFO_ADDR + sizeof(config_file_header_t) + sizeof(g_rawFlashPlatConfig)), sizeof(fsAssertCount));
                platInfo.header.fileBodySize   = sizeof(plat_config_raw_flash_t);
                platInfo.header.version        = RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION;
                platInfo.header.checkSum       = BSP_CalcCrcValue((uint8_t *)&g_rawFlashPlatConfig, sizeof(g_rawFlashPlatConfig));

                platInfo.config = g_rawFlashPlatConfig;
                platInfo.fsAssertCount = fsAssertCount;

                BSP_WriteToRawFlash((uint8_t*)&platInfo, sizeof(platInfo));
            }
        #else
            /* do nothing! */
        #endif
        }


    }

}

plat_config_raw_flash_t* BSP_GetRawFlashPlatConfig(void)
{
    return &g_rawFlashPlatConfig;
}

PLAT_BL_CIRAM_FLASH_TEXT uint32_t BSP_GetPlatConfigItemValue(plat_config_id_t id)
{
    switch(id)
    {
        case PLAT_CONFIG_ITEM_FAULT_ACTION:
            return g_rawFlashPlatConfig.faultAction;

        case PLAT_CONFIG_ITEM_UART_DUMP_PORT:
            return g_rawFlashPlatConfig.uartDumpPort;

        case PLAT_CONFIG_ITEM_START_WDT:
            return g_rawFlashPlatConfig.startWDT;

        case PLAT_CONFIG_ITEM_LOG_CONTROL:
            return g_rawFlashPlatConfig.logControl;

        case PLAT_CONFIG_ITEM_LOG_BAUDRATE:
            return g_rawFlashPlatConfig.uartBaudRate;

        case PLAT_CONFIG_ITEM_LOG_LEVEL:
            return g_rawFlashPlatConfig.logLevel;

        case PLAT_CONFIG_ITEM_LOG_OWNER_AND_LEVEL:
            return g_rawFlashPlatConfig.logOwnerAndLevel;

        case PLAT_CONFIG_ITEM_ENABLE_PM:
        #ifdef PLAT_CONFIG_FS_ENABLE
            return g_fsPlatConfig.enablePM;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_GET_PLAT_CFG_0, P_ERROR, "Get enablePM unsupported yet!");
            return 0;
        #endif

        case PLAT_CONFIG_ITEM_SLEEP_MODE:
        #ifdef PLAT_CONFIG_FS_ENABLE
            return g_fsPlatConfig.sleepMode;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_GET_PLAT_CFG_1, P_ERROR, "Get sleepMode unsupported yet!");
            return 0;
        #endif

        case PLAT_CONFIG_ITEM_WAIT_SLEEP:
        #ifdef PLAT_CONFIG_FS_ENABLE
            return g_fsPlatConfig.slpWaitTime;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_GET_PLAT_CFG_2, P_ERROR, "Get slpWaitTime unsupported yet!");
            return 0;
        #endif

        case PLAT_CONFIG_ITEM_AT_PORT_BAUDRATE:
        #ifdef PLAT_CONFIG_FS_ENABLE
            #if 0
            EC_ASSERT(g_fsPlatConfig.atPortBaudRate == g_rawFlashPlatConfig.atPortBaudRate,
                      g_fsPlatConfig.atPortBaudRate, g_rawFlashPlatConfig.atPortBaudRate, 0);
            #else
            if(g_fsPlatConfig.atPortBaudRate != g_rawFlashPlatConfig.atPortBaudRate)
            {
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_GET_PLAT_CFG_3, P_WARNING, "non-identical baud between fs(%d) & raw(%d)!",
                                                           g_fsPlatConfig.atPortBaudRate, g_rawFlashPlatConfig.atPortBaudRate);
            }
            #endif
            return g_fsPlatConfig.atPortBaudRate;
        #else
            return g_rawFlashPlatConfig.atPortBaudRate;
        #endif

        case PLAT_CONFIG_ITEM_AT_PORT_FRAME_FORMAT:
        #ifdef PLAT_CONFIG_FS_ENABLE
            return g_fsPlatConfig.atPortFrameFormat.wholeValue;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_GET_PLAT_CFG_4, P_ERROR, "Get atPortFrameFormat unsupported yet!");
            return 0;
        #endif

        case PLAT_CONFIG_ITEM_ECSCLK_CFG:
        #ifdef PLAT_CONFIG_FS_ENABLE
            return g_fsPlatConfig.ecSclkCfg;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_GET_PLAT_CFG_5, P_ERROR, "Get ecSclkCfg unsupported yet!");
            return 0;
        #endif

        case PLAT_CONFIG_ITEM_LOG_PORT_SEL:
            return g_rawFlashPlatConfig.logPortSel;

        case PLAT_CONFIG_ITEM_USB_CTRL:
            return g_rawFlashPlatConfig.usbCtrl;

        case PLAT_CONFIG_ITEM_USB_SW_TRACE_FLAG:
            return g_rawFlashPlatConfig.usbSwTrace;

        case PLAT_CONFIG_ITEM_USB_SLEEP_MASK:
            return g_rawFlashPlatConfig.usbSlpMask;

        case PLAT_CONFIG_ITEM_USB_SLEEP_THD:
            return g_rawFlashPlatConfig.usbSlpThd;

        case PLAT_CONFIG_ITEM_USB_VBUS_MODE_EN:
            return g_rawFlashPlatConfig.usbVBUSModeEn;

        case PLAT_CONFIG_ITEM_USB_VBUS_WKUP_PAD:
            return g_rawFlashPlatConfig.usbVBUSWkupPad;

        case PLAT_CONFIG_ITEM_PWRKEY_MODE:
            return g_rawFlashPlatConfig.pwrKeyMode;

        case PLAT_CONFIG_ITEM_USB_NET:
#ifdef __USER_CODE__
        	return soc_user_usb_eth_mode();
#else
            return g_rawFlashPlatConfig.usbNet;
#ifdef FEATURE_PLAT_CFG_FS_SUP_USBNET_ATA
        case PLAT_CONFIG_ITEM_USBNET_ATA_RESULT:
            #ifdef PLAT_CONFIG_FS_ENABLE
                return g_fsPlatConfig.usbNetAdaptResult;  
            #else
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_GET_PLAT_CFG_6, P_ERROR, "Get usbnet autoadapt unsupported yet!");
                return 0;
            #endif       
#endif      
#endif

        case PLAT_CONFIG_ITEM_USB_VCOM_EN_BMP:
            return g_rawFlashPlatConfig.usbVcomEnBitMap;
		
        case PLAT_CONFIG_ITEM_FOTA_CONTROL:
            return g_rawFlashPlatConfig.fotaCtrl;

        case PLAT_CONFIG_ITEM_FOTA_URC_PORT_SEL:
            return g_rawFlashPlatConfig.fotaUrcPortSel;

        case PLAT_CONFIG_ITEM_FOTA_USB_URC_CONTROL:
            return g_rawFlashPlatConfig.fotaUsbUrcCtrl;

        case PLAT_CONFIG_ITEM_PMUINCDRX:
            return g_rawFlashPlatConfig.pmuInCdrx;

		case PLAT_CONFIG_ITEM_SLP_LIMIT_EN:
            return g_rawFlashPlatConfig.slpLimitEn;

        case PLAT_CONFIG_ITEM_SLP_LIMIT_TIME:
            return g_rawFlashPlatConfig.slpLimitTime;

        case PLAT_CONFIG_ITEM_WFI_MODE:
#ifdef __USER_CODE__
        	return soc_user_wfi_mode();
#else
            return g_rawFlashPlatConfig.wfiMode;
#endif

		case PLAT_CONFIG_ITEM_IDLEPERCENT_PRINT_MODE:
			return g_rawFlashPlatConfig.apIdlePercentPrintMode;

        case PLAT_CONFIG_ITEM_CPSLPTEST_MODE:
            return g_rawFlashPlatConfig.cpSlpTest;

        default:
            return 0;
    }
}

#ifdef FEATURE_PLAT_CFG_FS_SUP_USBNET_ATA


#ifndef ATC_ECPCFG_USBNET_VAL_AUTOADAPT_TYPE
#define ATC_ECPCFG_USBNET_VAL_AUTOADAPT_RDSINIT    2
#endif

#ifndef ATC_ECPCFG_USBNET_VAL_AUTOADAPT_ECMINIT
#define ATC_ECPCFG_USBNET_VAL_AUTOADAPT_ECMINIT    3
#endif

uint32_t BSP_GetPlatCfgUsbNetATAEnabled(void)
{
    uint32_t ret = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_USB_NET) ;
    if ((ret==ATC_ECPCFG_USBNET_VAL_AUTOADAPT_RDSINIT)  || (ret==ATC_ECPCFG_USBNET_VAL_AUTOADAPT_ECMINIT))
    {
        return 1;
    }
    return 0;
}


void BSP_SetPlatCfgUsbNetATAItemVal(uint32_t val)
{
        BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_USBNET_ATA_RESULT, val);
}

uint32_t BSP_GetPlatCfgUsbNetATAItemVal(void)
{
        return BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_USBNET_ATA_RESULT);
}

#else
//for link pass
void BSP_SetPlatCfgUsbNetATAItemVal(uint32_t val)
{
    (void)val;
}

uint32_t BSP_GetPlatCfgUsbNetATAItemVal(void)
{
        return 0;
}

uint32_t BSP_GetPlatCfgUsbNetATAEnabled(void)
{
        return 0;
}

#endif

void BSP_SetPlatConfigItemValue(plat_config_id_t id, uint32_t value)
{
    switch(id)
    {
        case PLAT_CONFIG_ITEM_FAULT_ACTION:
            if(value <= (EXCEP_OPTION_MAX -1))
                g_rawFlashPlatConfig.faultAction = value;
            break;

        case PLAT_CONFIG_ITEM_UART_DUMP_PORT:
            g_rawFlashPlatConfig.uartDumpPort = value;
            break;

        case PLAT_CONFIG_ITEM_START_WDT:
            if(value <= 1)
                g_rawFlashPlatConfig.startWDT = value;
            break;

        case PLAT_CONFIG_ITEM_LOG_CONTROL:
            if(value <= 2)
                g_rawFlashPlatConfig.logControl = value;
            break;

        case PLAT_CONFIG_ITEM_LOG_BAUDRATE:
            g_rawFlashPlatConfig.uartBaudRate = value;
            break;

        case PLAT_CONFIG_ITEM_LOG_LEVEL:
            if(value <= P_ERROR)
                g_rawFlashPlatConfig.logLevel = (DebugTraceLevelType_e)value;
            break;

        case PLAT_CONFIG_ITEM_LOG_OWNER_AND_LEVEL:
            g_rawFlashPlatConfig.logOwnerAndLevel = value;
            break;

        case PLAT_CONFIG_ITEM_ENABLE_PM:
        #ifdef PLAT_CONFIG_FS_ENABLE
            g_fsPlatConfig.enablePM = value;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SET_PLAT_CFG_0, P_ERROR, "Set enablePM unsupported yet!");
        #endif
            break;

        case PLAT_CONFIG_ITEM_SLEEP_MODE:
        #ifdef PLAT_CONFIG_FS_ENABLE
            if(value <= 4) g_fsPlatConfig.sleepMode = value;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SET_PLAT_CFG_1, P_ERROR, "Set sleepMode unsupported yet!");
        #endif
            break;

        case PLAT_CONFIG_ITEM_WAIT_SLEEP:
        #ifdef PLAT_CONFIG_FS_ENABLE
            g_fsPlatConfig.slpWaitTime = value;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SET_PLAT_CFG_2, P_ERROR, "Set slpWaitTime unsupported yet!");
        #endif
            break;

        case PLAT_CONFIG_ITEM_AT_PORT_BAUDRATE:
            g_rawFlashPlatConfig.atPortBaudRate = value;
        #ifdef PLAT_CONFIG_FS_ENABLE
            g_fsPlatConfig.atPortBaudRate = value;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SET_PLAT_CFG_3, P_ERROR, "Set atPortBaudRate unsupported yet!");
        #endif
            break;

        case PLAT_CONFIG_ITEM_AT_PORT_FRAME_FORMAT:
        #ifdef PLAT_CONFIG_FS_ENABLE
            g_fsPlatConfig.atPortFrameFormat.wholeValue = value;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SET_PLAT_CFG_4, P_ERROR, "Set atPortFrameFormat unsupported yet!");
        #endif
            break;

        case PLAT_CONFIG_ITEM_ECSCLK_CFG:
        #ifdef PLAT_CONFIG_FS_ENABLE
            g_fsPlatConfig.ecSclkCfg = value;
        #else
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SET_PLAT_CFG_5, P_ERROR, "Set ecSclkCfg unsupported yet!");
        #endif
            break;

        case PLAT_CONFIG_ITEM_LOG_PORT_SEL:
            g_rawFlashPlatConfig.logPortSel = (PlatCfgUlgPort_e)value;
            break;

        case PLAT_CONFIG_ITEM_USB_CTRL:
            g_rawFlashPlatConfig.usbCtrl = value;
            break;

        case PLAT_CONFIG_ITEM_USB_SW_TRACE_FLAG:
            g_rawFlashPlatConfig.usbSwTrace = value;
            break;

        case PLAT_CONFIG_ITEM_USB_SLEEP_MASK:
            g_rawFlashPlatConfig.usbSlpMask = value;
            break;

        case PLAT_CONFIG_ITEM_USB_SLEEP_THD:
            g_rawFlashPlatConfig.usbSlpThd = value;
            break;

        case PLAT_CONFIG_ITEM_USB_VBUS_MODE_EN:
            g_rawFlashPlatConfig.usbVBUSModeEn = value;
            break;

        case PLAT_CONFIG_ITEM_USB_VBUS_WKUP_PAD:
            g_rawFlashPlatConfig.usbVBUSWkupPad = value;
            break;

        case PLAT_CONFIG_ITEM_PWRKEY_MODE:
            g_rawFlashPlatConfig.pwrKeyMode = value;
            break;

        case PLAT_CONFIG_ITEM_USB_NET:
            g_rawFlashPlatConfig.usbNet = value;
            break;
            
#ifdef FEATURE_PLAT_CFG_FS_SUP_USBNET_ATA
        case PLAT_CONFIG_ITEM_USBNET_ATA_RESULT:
            #ifdef PLAT_CONFIG_FS_ENABLE
                g_fsPlatConfig.usbNetAdaptResult = value;  
            #else
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, BSP_SET_PLAT_CFG_6, P_ERROR, "Set usbnet autoadapt unsupported yet!");
            #endif       
            break;
#endif            
        case PLAT_CONFIG_ITEM_USB_VCOM_EN_BMP:
            g_rawFlashPlatConfig.usbVcomEnBitMap = value;
            break;

        case PLAT_CONFIG_ITEM_FOTA_CONTROL:
            g_rawFlashPlatConfig.fotaCtrl = value;
            break;

        case PLAT_CONFIG_ITEM_FOTA_URC_PORT_SEL:
            g_rawFlashPlatConfig.fotaUrcPortSel = value;
            break;

        case PLAT_CONFIG_ITEM_FOTA_USB_URC_CONTROL:
            g_rawFlashPlatConfig.fotaUsbUrcCtrl = value;
            break;

        case PLAT_CONFIG_ITEM_PMUINCDRX:
            g_rawFlashPlatConfig.pmuInCdrx = value;
            break;

        case PLAT_CONFIG_ITEM_SLP_LIMIT_EN:
            g_rawFlashPlatConfig.slpLimitEn = value;
            break;

        case PLAT_CONFIG_ITEM_SLP_LIMIT_TIME:
            g_rawFlashPlatConfig.slpLimitTime = value;
            break;

        case PLAT_CONFIG_ITEM_WFI_MODE:
            g_rawFlashPlatConfig.wfiMode = value;
            break;

		case PLAT_CONFIG_ITEM_IDLEPERCENT_PRINT_MODE:
			g_rawFlashPlatConfig.apIdlePercentPrintMode = value;
       		break;

        case PLAT_CONFIG_ITEM_CPSLPTEST_MODE:
            g_rawFlashPlatConfig.cpSlpTest = value;

        default:
            break;
    }
    return;

}


