/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    plat_config.h
 * Description:  platform configuration header file
 * History:      Rev1.0   2019-01-18
 *               Rev1.1   2019-11-27   Reimplement file operations with OSA APIs(LFS wrapper), not directly using LFS APIs in case of file system replacement
 *               Rev1.2   2020-01-01   Separate plat config into two parts, FS and raw flash
 *
 ****************************************************************************/

#ifndef  _PLAT_CONFIG_H
#define  _PLAT_CONFIG_H

#include "Driver_Common.h"
#include "cmsis_compiler.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


//use FEATURE_PLAT_CFG_FS_SUP_USBNET_ATA not DFEATURE_USBNET_ATA_FOR_AP, the plat cfgfs structure is same for bootloader and ap
#ifndef FEATURE_PLAT_CFG_FS_SUP_USBNET_ATA

#define FS_PLAT_CONFIG_FILE_CURRENT_VERSION           (0)
#else
// the version 1 use scalable plat cfg fs data structure, it's more useful when then data config grows dynamically
#define FS_PLAT_CONFIG_FILE_CURRENT_VERSION           (1)
#endif

#define RAW_FLASH_PLAT_CONFIG_FILE_CURRENT_VERSION    (1)


 /** \brief config file header typedef */
__PACKED_STRUCT _config_file_header
{
    uint16_t fileBodySize;     /**< size of file body, in unit of byte */
    uint8_t  version;          /**< file version, this field shall be updated when file structure is changed */
    uint8_t  checkSum;         /**< check sum value of file body */
};
typedef struct _config_file_header config_file_header_t;

/** \brief typedef of platform configuration stored in fs */
typedef __PACKED_UNION _EPAT_atPortFrameFormat
{
    uint32_t wholeValue;
    __PACKED_STRUCT _config
    {
        uint32_t dataBits    : 3;
        uint32_t parity      : 2;
        uint32_t stopBits    : 2;
        uint32_t flowControl : 3;
    } config;
} atPortFrameFormat_t;



// ulg port enum
typedef enum
{
    PLAT_CFG_ULG_PORT_USB=0,
    PLAT_CFG_ULG_PORT_UART,
    PLAT_CFG_ULG_PORT_MIX,
    PLAT_CFG_ULG_PORT_SRAM,
    PLAT_CFG_ULG_PORT_MAX
} PlatCfgUlgPort_e;



#if (FS_PLAT_CONFIG_FILE_CURRENT_VERSION==0)
/*verion 0 orignal plat cfg, no change the data structure compare to released sdk,
so no merge needed when new sdk release, and plat cfg fs will also not change 
when new sdk burned. */

/** \brief typedef of platform configuration stored in fs */
typedef __PACKED_STRUCT _NVM_EPAT_plat_config
{
    /** PM on/off flag
     *  valid value:
     *        0x504D5544 -- PM is disabled, "PMUD"
     *        0x504D5545 -- PM is enabled, "PMUE"
     */
    uint32_t enablePM;

    /** sleep mode
     *  valid value:
     *        0 -- dummy
     *        1 -- dummy
     */
    uint8_t sleepMode;

    /** wait n ms before sleep, when wakeup from pad
     *  valid value:
     *        0 -- do not wait
     *        x -- wait x ms
     */
    uint32_t slpWaitTime;

    /** AT baudrate,for AP only
     *  should be equal to 'atPortBaudRate' in struct plat_config_raw_flash_t
     */
    uint32_t atPortBaudRate;

    /** AT port frame format*/
    atPortFrameFormat_t atPortFrameFormat;

    /** ECQSCLK config
    *  valid value:
    *        0 -- ECQSCLK set to 0
    *        1 -- ECQSCLK set to 1
    */
    uint8_t ecSclkCfg;

    
} plat_config_fs_t;

#else
/*version 1, scalable plat cfg, two method to update the config data structure
1.The total data size for scalable area does not change when some new config data added, the reserved data array still have some bytes spare, 
the added data just use one or some bytes of the reserved data array. 
When the new config data added the upgrade is compatitble. and downgrade is also compatitble too. 
If the new config data added is a automatical parameter, such as the usbNetAdaptResult, it's automatically setted by the UE connected with different Host Windows or Ubuntu.
If the new config data added is not a automatical parameter, it should be setted to a meaningful value otherwise when upgrade ,it may not work correctly as required by the default 0.

In this case, the plat cfg fs data will not be overwrited to fs  when both upgarde or downgrade initlal between the old/new sdk versions that plat cfg fs data size same .
it's more convenient for lightly upgrade or downgrade.

2.The total data size for scalable area is increased when some new config data added. because the reserved data array has no some bytes spare, 
Increase the scalable area with some bytes each time, such as each time 16 bytes(_NVM_PLATCFG_SCALE_UNIT_SZ) for example. use one or some of the increased bytes by new added parameter, 
some other bytes defined as reserverd data array.
In this case, the plat cfg fs data will be writed when upgrade or downgrade ocurrs.

Set recPrevArAllSzForUpg = scaleAreaAllSize - SCALE_AREA_SCALE_UNIT_SIZE


2.1 Upgarde  from  previous plat cfg fs structure to a new added parameter new plat cfg fs structure.
Provided condition: readed scalable area total size from plat cfg fs is not same with current scaleAreaAllSize real size


If the readed scalable area total size from plat cfg fs is same with recPrevArAllSzForUpg, then it's upgradable.
If not same with recPrevArAllSzForUpg, then it's not upgardable because may have a lot of difference between old/new versions.

2.2 Downgrade from a new added parameter new plat cfg fs structure to  an old plat cfg fs structure.
Provided condition: readed scalable area total size from plat cfg fs is not same with current scaleAreaAllSize var

If the readed recPrevArAllSzForUpg from plat cfg fs is current scaleAreaAllSize real size, then it's downgradable.
If recPrevArAllSzForUpg not same with current scaleAreaAllSize real size, then it's not downgradable because may have a lot of difference between old/new versions.
*/

#define _NVM_PLATCFG_SCALE_UNIT_SZ 16


// S 53 C 43 A 41  L 4C
#define PLAT_CFG_SCALE_START_MARK 0x4353   
#define PLAT_CFG_SCALE_END_MARK 0x4C41




#define PLAT_CFG_FIX_BASE_SIZE 18

#define SCALE_AREA_GRP_UNIT_SIZE 16
#define PLAT_CFG_GRP0_RSVBYTES_NUM 15
#define PLAT_CFG_GRP0_TOKBYTES_NUM  (SCALE_AREA_GRP_UNIT_SIZE-PLAT_CFG_GRP0_RSVBYTES_NUM)


#define PLAT_CFG_GRP_DATA_X(_TYPE_XX, __NAME_XX)  _TYPE_XX __NAME_XX
#define PLAT_CFG_DATA_0(_TYPE_XX, __NAME_XX)  PLAT_CFG_DATA_X(_TYPE_XX, __NAME_XX)

#define PLAT_CFG_GRP_RSV_BYTE_X(__GRP_XX, __IDX_XX) uint8_t  rsvBytes_##__GRP_XX##__IDX_XX

#define PLAT_CFG_GRP0_ALL_TOK_BYTES()  \
        uint8_t usbNetAdaptResult;          


#define PLAT_CFG_GRP0_RSVBYTE_0()     PLAT_CFG_GRP_RSV_BYTE_X(0, 0);
#define PLAT_CFG_GRP0_RSVBYTE_1()     PLAT_CFG_GRP_RSV_BYTE_X(0, 1);
#define PLAT_CFG_GRP0_RSVBYTE_2()     PLAT_CFG_GRP_RSV_BYTE_X(0, 2);
#define PLAT_CFG_GRP0_RSVBYTE_3()     PLAT_CFG_GRP_RSV_BYTE_X(0, 3);
#define PLAT_CFG_GRP0_RSVBYTE_4()     PLAT_CFG_GRP_RSV_BYTE_X(0, 4);
#define PLAT_CFG_GRP0_RSVBYTE_5()     PLAT_CFG_GRP_RSV_BYTE_X(0, 5);
#define PLAT_CFG_GRP0_RSVBYTE_6()     PLAT_CFG_GRP_RSV_BYTE_X(0, 6);
#define PLAT_CFG_GRP0_RSVBYTE_7()     PLAT_CFG_GRP_RSV_BYTE_X(0, 7);
#define PLAT_CFG_GRP0_RSVBYTE_8()     PLAT_CFG_GRP_RSV_BYTE_X(0, 8);
#define PLAT_CFG_GRP0_RSVBYTE_9()     PLAT_CFG_GRP_RSV_BYTE_X(0, 9);
#define PLAT_CFG_GRP0_RSVBYTE_10()     PLAT_CFG_GRP_RSV_BYTE_X(0, 10);
#define PLAT_CFG_GRP0_RSVBYTE_11()     PLAT_CFG_GRP_RSV_BYTE_X(0, 11);
#define PLAT_CFG_GRP0_RSVBYTE_12()     PLAT_CFG_GRP_RSV_BYTE_X(0, 12);
#define PLAT_CFG_GRP0_RSVBYTE_13()     PLAT_CFG_GRP_RSV_BYTE_X(0, 13);
#define PLAT_CFG_GRP0_RSVBYTE_14()     PLAT_CFG_GRP_RSV_BYTE_X(0, 14);
#define PLAT_CFG_GRP0_RSVBYTE_15()     PLAT_CFG_GRP_RSV_BYTE_X(0, 15);


#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_0()  PLAT_CFG_GRP0_RSVBYTE_0() 

#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_1()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_0()   \
                                                                        PLAT_CFG_GRP0_RSVBYTE_1() 
                                                                        
#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_2()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_1()  \
                                                                            PLAT_CFG_GRP0_RSVBYTE_2() 
                                                                            
#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_3()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_2() \
                                                                                PLAT_CFG_GRP0_RSVBYTE_3() 

#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_4()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_3() \
                                                                                PLAT_CFG_GRP0_RSVBYTE_4() 
                                                                                
#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_5()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_4 ()\
                                                                                    PLAT_CFG_GRP0_RSVBYTE_5() 

#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_6()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_5() \
                                                                                    PLAT_CFG_GRP0_RSVBYTE_6() 


#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_7()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_6() \
                                                                                    PLAT_CFG_GRP0_RSVBYTE_7() 


#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_8()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_7() \
                                                                                    PLAT_CFG_GRP0_RSVBYTE_8() 
                                                                                    
#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_9()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_8() \
                                                                                        PLAT_CFG_GRP0_RSVBYTE_9() 
                                                                                        
#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_10()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_9() \
                                                                                            PLAT_CFG_GRP0_RSVBYTE_10() 


#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_11()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_10() \
                                                                                            PLAT_CFG_GRP0_RSVBYTE_11() 



#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_12()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_11()  \
                                                                                            PLAT_CFG_GRP0_RSVBYTE_12() 


#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_13()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_12() \
                                                                                            PLAT_CFG_GRP0_RSVBYTE_13() 
                                                                                            
#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_14()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_13() \
                                                                                            PLAT_CFG_GRP0_RSVBYTE_14() 

#define PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_15()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_14() \
                                                                                            PLAT_CFG_GRP0_RSVBYTE_15() 


#if (PLAT_CFG_GRP0_RSVBYTES_NUM== 0)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()   
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==1)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_0()
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==2)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_1()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==3)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_2()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==4)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_3()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==5)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_4()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==6)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_5()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==7)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_6()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==8)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_7()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM== 9)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_8()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==10)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_9()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==11)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_10)  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==12)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_11)  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==13)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_12()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==14)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_13()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==15)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_14()  
#elif (PLAT_CFG_GRP0_RSVBYTES_NUM==16)
#define PLAT_CFG_GRP0_ALL_RSV_BYTES()  PLAT_CFG_GRP0_RANGE_RSV_BYTES_0_15()   
#endif



typedef  __PACKED_STRUCT plat_cfg_sc_grp0_tok_tag {                   
    PLAT_CFG_GRP0_ALL_TOK_BYTES()         
}plat_cfg_sc_grp0_tok_st;


typedef  __PACKED_STRUCT plat_cfg_sc_grp0_all_data_tag {                   
    PLAT_CFG_GRP0_ALL_TOK_BYTES()         
    PLAT_CFG_GRP0_ALL_RSV_BYTES()           
}plat_cfg_sc_grp0_all_data_st;


#define SIZE_OF_TYPE_EQUAL_TO_SZX(type_x, size_x)       \
        static inline char size_of##type_x##_equal_to_##size_x(void)  {     \
            char __dummy1[sizeof(type_x) - size_x];                                      \
            char __dummy2[size_x-sizeof(type_x)];                                       \
            return __dummy1[-1]+__dummy2[-1];                                          \
        }

#define SIZE_OF_TYPE_EQUAL_TO_CALX(type_x, cal_sz_x)       \
        static inline char size_of##type_x##_equal_to_cal_sz_x(void)  {     \
            char __dummy1[sizeof(type_x) - (cal_sz_x)];                                      \
            char __dummy2[(cal_sz_x)-sizeof(type_x)];                                       \
            return __dummy1[-1]+__dummy2[-1];                                          \
        }


        
typedef __PACKED_STRUCT _NVM_EPAT_plat_config
{
    /*do not change any variable function or name of base fix area if want to upgrade ,
        because when sync para from prev plat config scale fs the para may be syncd unmatched and used uncorrectly*/        

    /** PM on/off flag
     *  valid value:
     *        0x504D5544 -- PM is disabled, "PMUD"
     *        0x504D5545 -- PM is enabled, "PMUE"
     */
    uint32_t enablePM;

    /** sleep mode
     *  valid value:
     *        0 -- dummy
     *        1 -- dummy
     */
    uint8_t sleepMode;

    /** wait n ms before sleep, when wakeup from pad
     *  valid value:
     *        0 -- do not wait
     *        x -- wait x ms
     */
    uint32_t slpWaitTime;

    /** AT baudrate,for AP only
     *  should be equal to 'atPortBaudRate' in struct plat_config_raw_flash_t
     */
    uint32_t atPortBaudRate;

    /** AT port frame format*/
    atPortFrameFormat_t atPortFrameFormat;

    /** ECQSCLK config
    *  valid value:
    *        0 -- ECQSCLK set to 0
    *        1 -- ECQSCLK set to 1
    */
    uint8_t ecSclkCfg;


    /*Mark for reserve bytes area*/
    uint16_t scaleAreaStartMark;

    /*scalable area data define*/
    /*do not change any variable function or name of used scale area data,
        because when sync para from prev plat config scale fs the para may be syncd unmatched and used uncorrectly*/      
    //scaleAreaPureData0        
    //uint8_t usbNetAdaptResult;/*  7-4: Adapt valid, 3-0:adapt result*/

    
    //scaleAreaPureData1-   scaleAreaPureDataX X=(SCALE_AREA_SCALE_UNIT_SIZE-1)
    //uint8_t scaleAreaSpareBytes[SCALE_AREA_SCALED_SPARE_SZ];
    PLAT_CFG_GRP0_ALL_TOK_BYTES()
    PLAT_CFG_GRP0_ALL_RSV_BYTES()
    
    uint16_t scaleAreaEndMark;

    //scalable area data structure version, added 1 by customer if sometimes need to upgrade from specific scalable area verson to another specific scalable area version, 
    //if not need specific upgrade,  set to defautl 0 ro add 1 just for record
    uint16_t recScaleAreaGrpNum; 
    
    uint16_t recPrevArAllSzForUpg;
    
    //uint16_t scaleTimers;
    uint16_t scaleAreaAllSize;
} plat_config_fs_t;



#endif

/** \brief typedef of platform configuration stored in raw flash --old v0*/
__PACKED_STRUCT _plat_config_raw_flash_v0
{
    /** action to perform when assert or hard fault occurs
     *  valid value:
     *        0 -- dump full exception info to flash and EPAT tool then trapped in endless loop(while(1))
     *        1 -- print necessary exception info then reset
     *        2 -- dump full exception info to flash then reset
     *        3 -- dump full exception info to flash and EPAT tool then reset
     *        4 -- reset directly
     *       10 -- enable uart help dump and dump full exception info to flash and EPAT tool then trapped in endless loop(while(1))
     *       13 -- enable uart help dump and dump full exception info to flash and EPAT tool, and then reset
     */
    uint8_t faultAction;

    /** port select for dump info output when exception occurs
     *  valid value:
     *        0,1,2,3,(4) -- specify which port
     *        0xff -- disable this function
     */
    uint8_t uartDumpPort;

    /** WDT start/stop control
     *  valid value:
     *        0 -- stop WDT
     *        1 -- start WDT
     */
    uint8_t startWDT;

    /** unilog on/off flag
     *  valid value:
     *        0 -- unilog is disabled
     *        1 -- only sw log is enabled
     *        2 -- All log is enabled
     */
    uint8_t logControl;

    /** uart baudrate for unilog output */
    uint32_t uartBaudRate;

    /** debug trace log level setting, refer to 'DebugTraceLevelType_e' */
    uint32_t logLevel;

    /** unilog output port select
     *  valid value:
     *        0 -- USB
     *        1 -- UART
     *        2 -- MIX(for future use UART/USB dynamic select)
     **/
    PlatCfgUlgPort_e logPortSel;

    /** RNDIS enum control
     *  valid value:
     *        0 -- enable USB init and enum RNDIS
     *        1 -- enable USB init but not enum RNDIS
     *        2 -- disable USB init
     */
    uint8_t usbCtrl;

    /** usb software trace control
     *  valid value:
     *        0 -- disable all usb software trace
     *        1 -- enable all usb software trace
     *        others -- misc usb software trace
     */
    uint8_t usbSwTrace;



    /** USB sleep mask
     * valid value:
     *       0 -- usb should vote to enter sleep
     *       1 -- do not consider usb vote before sleep
     */
    uint8_t usbSlpMask;

    /** USB sleep thd
     * valid value:
     *       set the minimal time to sleep, when usbSlpMask=1
     */
    uint16_t usbSlpThd;

    /** pwrkey mode
     * valid value:
     *       1    power key mode
     *       0    normal key mode
     */
    uint8_t pwrKeyMode;
};

// fota urc port type
typedef enum
{
    PLAT_CFG_FOTA_URC_PORT_USB=0,
    PLAT_CFG_FOTA_URC_PORT_UART,
    PLAT_CFG_FOTA_URC_PORT_MAXTYPE
} PlatCfgFotaUrcPortType_e;

#define PLAT_CFG_FOTA_URC_USB_PORT_IDX_MIN    0
#define PLAT_CFG_FOTA_URC_USB_PORT_IDX_MAX    2

#define PLAT_CFG_FOTA_URC_UART_PORT_IDX_MIN   0
#define PLAT_CFG_FOTA_URC_UART_PORT_IDX_MAX   1


#define PLAT_CFG_RAW_FLASH_RSVD_SIZE    16

/** \brief typedef of platform configuration stored in raw flash */
__PACKED_STRUCT _plat_config_raw_flash
{
    /** action to perform when assert or hard fault occurs
     *  valid value:
     *        0 -- dump full exception info to flash and EPAT tool then trapped in endless loop(while(1))
     *        1 -- print necessary exception info then reset
     *        2 -- dump full exception info to flash then reset
     *        3 -- dump full exception info to flash and EPAT tool then reset
     *        4 -- reset directly
     *       10 -- enable uart help dump and dump full exception info to flash and EPAT tool then trapped in endless loop(while(1))
     *       13 -- enable uart help dump and dump full exception info to flash and EPAT tool, and then reset
     */
    uint8_t faultAction;

    /** port select for dump info output when exception occurs
     *  valid value:
     *        0,1,2,3,(4) -- specify which port
     *        0xff -- disable this function
     */
    uint8_t uartDumpPort;


    /** WDT start/stop control
     *  valid value:
     *        0 -- stop WDT
     *        1 -- start WDT
     */
    uint8_t startWDT;

    /** unilog on/off flag
     *  valid value:
     *        0 -- unilog is disabled
     *        1 -- only sw log is enabled
     *        2 -- All log is enabled
     */
    uint8_t logControl;

    /** uart baudrate for unilog output */
    uint32_t uartBaudRate;

    /** debug trace log level setting, refer to 'DebugTraceLevelType_e' */
    uint32_t logLevel;

    /** unilog output port select
     *  valid value:
     *        0 -- USB
     *        1 -- UART
     *        2 -- MIX(for future use UART/USB dynamic select)
     **/
    PlatCfgUlgPort_e logPortSel;

    /** RNDIS enum control
     *  valid value:
     *        0 -- enable USB init and enum RNDIS
     *        1 -- enable USB init but not enum RNDIS
     *        2 -- disable USB init
     */
    uint8_t usbCtrl;

    /** usb software trace control
     *  valid value:
     *        0 -- disable all usb software trace
     *        1 -- enable all usb software trace
     *        others -- misc usb software trace
     */
    uint8_t usbSwTrace;



    /** USB sleep mask
     * valid value:
     *       0 -- usb should vote to enter sleep
     *       1 -- do not consider usb vote before sleep
     */
    uint8_t usbSlpMask;

    /** USB sleep thd
     * valid value:
     *       set the minimal time to sleep, when usbSlpMask=1
     */
    uint16_t usbSlpThd;

    /** pwrkey mode
     * valid value:
     *       1    power key mode
     *       0    normal key mode
     */
    uint8_t pwrKeyMode;

    /** USB VBUS MODE Enable,Disable Flag
     * valid value:
     *       0 -- usb vbus mode disable
     *       1 -- usb vbus mode enable
     */
    uint8_t usbVBUSModeEn;

    /** USB VBUS MODE Wakup Pad Index
     * valid value:
     * 0,1,2,3,4,5 PAD IDX FOR USB VBUS WKUP PAD
     */
    uint8_t usbVBUSWkupPad;

    /** USB NET IF SEL
     * valid value:
     * 0----RNDIS,default
     * 1----ECM
     */
    uint8_t usbNet;

    /** USB VCOM EN bitmap
     * valid value:
     * bit0---vcom0
     * bit1---vcom1
     * ----
     * ----
     */
    uint8_t usbVcomEnBitMap;

    /** AT/fotaURC baudrate, for AP & BL*/
    uint32_t atPortBaudRate;

    /** FOTA control
     *        0 -- disable FOTA
     *        1 -- enable FOTA
     **/
    uint8_t  fotaCtrl;

    /** FOTA URC output port select
     *  valid value(Bit4-7):
     *        0 -- USB
     *        1 -- UART
     **
     *  valid value(Bit0-3):
     *        0-2 -- USB
     *        0-1 -- UART
     **/
    uint8_t  fotaUrcPortSel;

    /** FOTA USB URC output port control
     *        0 -- disable USB URC output
     *        1 -- enable USB URC output
     **/
    uint8_t  fotaUsbUrcCtrl;

    /** pmuInCdrx
     * valid value:
     * 0----
     * 1----
     */
    uint8_t pmuInCdrx;

    /** slpLimitEn
     * valid value:
     * 0---- disable
     * 1---- enable
     */
    uint8_t slpLimitEn;

    /** slpLimitTime
     * valid value:
     * 0---0xFFFFFFFF
     */
    uint32_t slpLimitTime;

    /** logOwnerAndLevel
     * valid value:
     * 0---0xFFFFFFFF
     */
    uint32_t logOwnerAndLevel;

    /** wfi mode, do not enter doze */
    uint8_t wfiMode;

    /** ECIDLEP config
    *  valid value:
    *        0 -- print flag set to 0
    *        1 -- print flag set to 1
    */
    uint8_t apIdlePercentPrintMode;

    /** cpSlpTest
    *   0: disable sleep test
    *   1: CP deepslp
    *   2: Doze+pll vote disable
    *   3. Doze+pll vote enable
    *   4: DFC+WFI
    *   5: WFI DFC disable
    *   6. while
    */
    uint8_t cpSlpTest;

    /* 'PLAT_CFG_RAW_FLASH_RSVD_SIZE' bytes rsvd for future */
    uint8_t resv[PLAT_CFG_RAW_FLASH_RSVD_SIZE];
};

typedef struct _plat_config_raw_flash plat_config_raw_flash_t;//current
typedef struct _plat_config_raw_flash_v0 plat_config_raw_flash_v0_t;//old v0

/** \brief typedef of platform info layout stored in raw flash */
__PACKED_STRUCT _plat_info_layout
{
    config_file_header_t header;           /**< raw flash plat config header */
    plat_config_raw_flash_t config;        /**< raw flash plat config body */
    uint32_t fsAssertCount;                /**< count for monitoring FS assert, when it reaches specific number, FS region will be re-formated */
};
typedef struct _plat_info_layout plat_info_layout_t;

/** @brief List of platform configuration items used to set/get sepecific setting */
typedef enum _plat_config_id
{
    PLAT_CONFIG_ITEM_FAULT_ACTION = 0,       /**< faultAction item */
    PLAT_CONFIG_ITEM_UART_DUMP_PORT,         /**< uartDumpPort item */
    PLAT_CONFIG_ITEM_START_WDT,              /**< startWDT item */
    PLAT_CONFIG_ITEM_LOG_CONTROL,            /**< logControl item */
    PLAT_CONFIG_ITEM_LOG_BAUDRATE,           /**< uart baudrate for log output */
    PLAT_CONFIG_ITEM_LOG_LEVEL,              /**< logLevel item */
    PLAT_CONFIG_ITEM_ENABLE_PM,              /**< enablePM item */
    PLAT_CONFIG_ITEM_SLEEP_MODE,             /**< sleepMode item */
    PLAT_CONFIG_ITEM_WAIT_SLEEP,             /**< wait ms before sleep */
    PLAT_CONFIG_ITEM_AT_PORT_BAUDRATE,       /**< AT port baudrate */
    PLAT_CONFIG_ITEM_AT_PORT_FRAME_FORMAT,   /**< AT port frame format */
    PLAT_CONFIG_ITEM_ECSCLK_CFG,             /**< ECSCLK config */
    PLAT_CONFIG_ITEM_LOG_PORT_SEL,           /**< ULG output port select */
    PLAT_CONFIG_ITEM_USB_CTRL,               /**< USB control */
    PLAT_CONFIG_ITEM_USB_SW_TRACE_FLAG,      /**< USB control */
    PLAT_CONFIG_ITEM_USB_SLEEP_MASK,         /**< USB Sleep Vote Mask */
    PLAT_CONFIG_ITEM_USB_SLEEP_THD,          /**< USB Sleep Thread */
    PLAT_CONFIG_ITEM_PWRKEY_MODE,            /**< PWRKEY Mode */
    PLAT_CONFIG_ITEM_USB_VBUS_MODE_EN,       /**< USB VBUS MODE ENABLE, DISABLE*/
    PLAT_CONFIG_ITEM_USB_VBUS_WKUP_PAD,      /**< USB VBUS MODE WKUP PAD INDEX*/
    PLAT_CONFIG_ITEM_USB_NET,                /**< USB NET Select*/
    PLAT_CONFIG_ITEM_USBNET_ATA_RESULT,       /*USB NET Autoadapt result */    
    PLAT_CONFIG_ITEM_USB_VCOM_EN_BMP,        /**< USB VCOM Enabled Bitmap*/
    PLAT_CONFIG_ITEM_FOTA_CONTROL, 	         /**< FOTA URC Port control*/
    PLAT_CONFIG_ITEM_FOTA_URC_PORT_SEL,      /**< FOTA URC Port Select*/
    PLAT_CONFIG_ITEM_FOTA_USB_URC_CONTROL, 	 /**< FOTA USB URC control*/
    PLAT_CONFIG_ITEM_PMUINCDRX,              /**< PMUINCDRX Select*/
    PLAT_CONFIG_ITEM_SLP_LIMIT_EN,           /**< enable sleep time limit*/
    PLAT_CONFIG_ITEM_SLP_LIMIT_TIME,         /**< set maximum sleep time in mili second*/
    PLAT_CONFIG_ITEM_WFI_MODE,               /**< WFI Mode */
    PLAT_CONFIG_ITEM_IDLEPERCENT_PRINT_MODE, /**< PRINT IDLE PERCENT MODE */
    PLAT_CONFIG_ITEM_CPSLPTEST_MODE,         /**< CP Sleep Mode Test */
    PLAT_CONFIG_ITEM_TOTAL_NUMBER,           /**< total number of items */
    PLAT_CONFIG_ITEM_LOG_OWNER_AND_LEVEL,    /**< log Owner and logLevel for this Owner item */
} plat_config_id_t;


/*******************************************************************************
 * API
 ******************************************************************************/

#ifdef __cplusplus
    extern "C" {
#endif

/**
  \fn        void BSP_SavePlatConfigToFs(void)
  \brief     Save platform configuration into FS
  \return    void
 */
void BSP_SavePlatConfigToFs(void);

/**
  \fn        void BSP_LoadPlatConfigFromFs(void)
  \brief     Load platform configuration from FS
  \return    void
 */
void BSP_LoadPlatConfigFromFs(void);

/**
  \fn        plat_config_fs_t* BSP_GetFsPlatConfig(void)
  \brief     Get FS platform configuration variable pointer
  \return    pointer to internal platform configuration loaded from FS
 */
plat_config_fs_t* BSP_GetFsPlatConfig(void);

/**
  \fn        void BSP_SavePlatConfigToRawFlash(void)
  \brief     Save platform configuration into raw flash
  \return    void
 */
void BSP_SavePlatConfigToRawFlash(void);

/**
  \fn        void BSP_LoadPlatConfigFromRawFlash(void)
  \brief     Load platform configuration from raw flash
  \return    void
 */
void BSP_LoadPlatConfigFromRawFlash(void);

/**
  \fn        plat_config_raw_flash_t* BSP_GetRawFlashPlatConfig(void)
  \brief     Get raw flash platform configuration variable pointer
  \return    pointer to internal platform configuration loaded from raw flash
 */
plat_config_raw_flash_t* BSP_GetRawFlashPlatConfig(void);

/**
  \fn        uint32_t BSP_GetPlatConfigItemValue(plat_config_id_t id)
  \brief     Get value of specific platform configuration item
  \param[in] id    id of platform configuration item, \ref plat_config_id_t
  \return    value of current configuration item
 */
uint32_t BSP_GetPlatConfigItemValue(plat_config_id_t id);

/**
  \fn        void BSP_SetPlatConfigItemValue(plat_config_id_t id, uint32_t value)
  \brief     Set value of specific platform configuration item
  \param[in] id    id of platform configuration item, \ref plat_config_id_t
  \param[in] value value of configuration item to set
  \return    void
 */
void BSP_SetPlatConfigItemValue(plat_config_id_t id, uint32_t value);

/**
  \fn        uint32_t BSP_GetFSAssertCount(void)
  \brief     Fetch current 'fsAssertCount' value from PLAT_INFO region
  \return    current fsAssertCount value
 */
uint32_t BSP_GetFSAssertCount(void);

/**
  \fn        void BSP_SetFSAssertCount(uint32_t value);
  \brief     Update 'fsAssertCount' value
  \param[in] value  new value assigned to 'fsAssertCount'
  \return    void
  \note      Internal use only on FS assert occurs
 */
void BSP_SetFSAssertCount(uint32_t value);

/**
  \fn        void BSP_SetFsPorDefaultValue(void);
  \brief     when por happened some data may retore to it's default
  \return    void
 */
void BSP_SetFsPorDefaultValue(void);

/**
  \fn        void BSP_SetPlatCfgUsbNetATAItemVal(void);
  \brief     when USB net auto adapt enabled, to store the adapt result usb net type
  \return    void
 */
void BSP_SetPlatCfgUsbNetATAItemVal(uint32_t val);

/**
  \fn        void BSP_GetPlatCfgUsbNetATAEnabled(void);
  \brief     when USB net type config equal to ATC_ECPCFG_USBNET_VAL_AUTOADAPT_TYPE auto adapt enabled, return 1, other return 0
  \return    void
 */
uint32_t BSP_GetPlatCfgUsbNetATAEnabled(void);


#ifdef __cplusplus
}
#endif

#endif /* _PLAT_CONFIG_H */
