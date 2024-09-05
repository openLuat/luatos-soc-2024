#ifndef __PS_STK_H__
#define __PS_STK_H__
/******************************************************************************
 ******************************************************************************
 Copyright:      - 2017- Copyrights of AirM2M Ltd.
 File name:      - psstk.h
 Description:    - process STK proactive commands include header files
 History:        - 06/12/2024, Originated
 ******************************************************************************
******************************************************************************/
#include <stdint.h>
#include "cmisim.h"

/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/


#define PS_STK_MAX_ALPHA_ID_STR_LEN     512
#define PS_STK_MAX_TEXT_STR_LEN         512
#define PS_STK_MAX_ITEM_STR_LEN         160
#define PS_STK_MAX_ITEM_NUM             30
#define PS_STK_MAX_ENC_TEXT_STR_LEN     200

#define PS_STK_RESULT_NO_RESPONSE_FROM_USER    0x12

/******************************************************************************
 *****************************************************************************
 * ENUM
 *****************************************************************************
******************************************************************************/
typedef enum PsStkProactCmdTypeTag
{
    STK_COMMAND_PLAY_TONE              = 0x20,
    STK_COMMAND_DISPLAY_TEXT           = 0x21,
    STK_COMMAND_GET_INKEY              = 0x22,
    STK_COMMAND_GET_INPUT              = 0x23,
    STK_COMMAND_SELECT_ITEM            = 0x24,
    STK_COMMAND_SET_UP_MENU            = 0x25,
    STK_COMMAND_SET_UP_IDLE_MODE_TXT   = 0x28,
    STK_COMMAND_LANG_NOTIFICATION      = 0x35,
    STK_COMMAND_MENU_SELECTION         = 0xFD,
    STK_COMMAND_TERMINATE_SESSION      = 0xFE
}
PsStkProactCmdType;

typedef enum PsStkDataCodingSchemeTag
{
    PS_STK_DCS_DEF_7BIT            = 0x00,
    PS_STK_DCS_8BIT                = 0x04,
    PS_STK_DCS_UCS2                = 0x08
}
PsStkDataCodingScheme;

typedef enum PsStkDeviceIdTag
{
    STK_DEV_ID_KEYPAD                  = 0x01,
    STK_DEV_ID_DISPLAY                 = 0x02,
    STK_DEV_ID_EARPIECE                = 0x03,
    STK_DEV_ID_UICC                    = 0x81,
    STK_DEV_ID_TERMINAL                = 0x82,
    STK_DEV_ID_NETWORK                 = 0x83
}
PsStkDeviceId;

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

typedef struct PsStkItemTag
{
    UINT8               itemId;
    UINT8               resvd[2];
    UINT8               itemStrLen;
    CHAR                itemStr[PS_STK_MAX_ITEM_STR_LEN];
}
PsStkItem;

typedef struct PsStkProactiveCmdTag
{
    UINT8               cmdType;//PsStkProactCmdType
    BOOL                crFlag;//comprehension required flag
    UINT16              cmdLen;
    UINT8               *pCmdData;
}
PsStkProactiveCmd;

typedef struct PsStkCommandDetailsTag
{
    UINT8                        cmdNum;//command number
    UINT8                        cmdType;//PsStkProactCmdType
    UINT8                        cmdQualifier;
    UINT8                        resvd;
}
PsStkCommandDetails;


typedef struct PsStkPlayToneCmdInfoTag
{
    UINT8               qualifier;//command details
    CHAR                tone[3];
    UINT8               durationUnit;
    UINT8               durationValue;
    UINT8               iconQualifier;
    UINT8               iconId;
    UINT8               alphaIdLen;
    CHAR                alphaId[PS_STK_MAX_ALPHA_ID_STR_LEN];
}
PsStkPlayToneCmdInfo;

typedef struct PsStkDisplayTextCmdInfoTag
{
    UINT8               qualifier;//command details
    UINT8               dsc;//data coding scheme
    UINT8               immediateRsp;
    UINT8               iconQualifier;
    UINT8               iconId;
    UINT8               textStrLen;
    CHAR                textStr[PS_STK_MAX_TEXT_STR_LEN];
}
PsStkDisplayTextCmdInfo;

typedef struct PsStkGetInkeyCmdInfoTag
{
    UINT8               qualifier;//command details
    UINT8               dsc;//data coding scheme
    UINT8               iconQualifier;
    UINT8               iconId;
    UINT8               textStrLen;
    CHAR                textStr[PS_STK_MAX_TEXT_STR_LEN];
}
PsStkGetInkeyCmdInfo;

typedef struct PsStkGetInputCmdInfoTag
{
    UINT8               qualifier;//command details
    UINT8               dsc;//data coding scheme
    UINT8               minRspLen;
    UINT8               maxRspLen;
    UINT8               iconQualifier;
    UINT8               iconId;
    UINT8               textStrLen;
    UINT8               defTextLen;
    CHAR                textStr[PS_STK_MAX_TEXT_STR_LEN];
    CHAR                defText[PS_STK_MAX_TEXT_STR_LEN];
}
PsStkGetInputCmdInfo;

typedef struct PsStkSetUpIdleModeTextCmdInfoTag
{
    UINT8               qualifier;//command details
    UINT8               dsc;//data coding scheme
    UINT8               iconQualifier;
    UINT8               iconId;
    UINT8               textStrLen;
    CHAR                textStr[PS_STK_MAX_TEXT_STR_LEN];
}
PsStkSetUpIdleModeTextCmdInfo;

typedef struct PsStkLanguageNotificationCmdInfoTag
{
    UINT8               qualifier;//command details
    UINT8               langCode1;//language code 1
    UINT8               langCode2;//language code 2
    UINT8               revd;
}
PsStkLanguageNotificationCmdInfo;

typedef struct PsStkSetUpMenuCmdInfoTag
{
    UINT8               qualifier;//command details
    UINT8               alphaIdLen;
    UINT8               itemNum;
    UINT8               itemNaiNum;//next action indicator number
    CHAR                alphaId[PS_STK_MAX_ALPHA_ID_STR_LEN];
    PsStkItem           itemList[PS_STK_MAX_ITEM_NUM];
    UINT8               itemNai[PS_STK_MAX_ITEM_NUM];//items next action indicator
    UINT8               iconQualifier;
    UINT8               iconId;
    UINT8               itemIconListQualifier;
    UINT8               itemIconNum;
    UINT8               itemIconIdList[PS_STK_MAX_ITEM_NUM];
}
PsStkSetUpMenuCmdInfo;

typedef PsStkSetUpMenuCmdInfo PsStkSelectItemCmdInfo;

typedef struct PsStkProactCmdRspTag
{
    UINT8               result;
    BOOL                addiInfoPresent;
    UINT8               addiInfo;
    UINT8               itemId;
    UINT8               inputStrLen;//>0 valid, and only present in TR for Get inkey/get input cmd
    UINT8               inputStr[PS_STK_MAX_TEXT_STR_LEN];
}
PsStkProactCmdRsp;

/******************************************************************************
 ******************************************************************************
 * External global variable
 ******************************************************************************
******************************************************************************/



/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/
UINT8 psStkGetPendingProactCmdType(void);
void psStkProcProactiveCmdInd(UINT8 cmdType, UINT16 cmdLen, UINT8 *pCmdData);
CmsRetId psStkGetPlayToneCmdInfo(PsStkPlayToneCmdInfo *pPlayToneCmdInfo);
CmsRetId psStkGetDisplayTextCmdInfo(PsStkDisplayTextCmdInfo *pDisplayTextCmdInfo);
CmsRetId psStkGetGetInkeyCmdInfo(PsStkGetInkeyCmdInfo *pGetInkeyCmdInfo);
CmsRetId psStkGetGetInputCmdInfo(PsStkGetInputCmdInfo *pGetInputCmdInfo);
CmsRetId psStkGetSetUpIdleModeTextCmdInfo(PsStkSetUpIdleModeTextCmdInfo *pSetUpIdleModeTextCmdInfo);
CmsRetId psStkGeLanguageNotificationCmdInfo(PsStkLanguageNotificationCmdInfo *pLanguageNotificationCmdInfo);
CmsRetId psStkGetSetUpMenuCmdInfo(PsStkSetUpMenuCmdInfo *pSetUpMenuCmdInfo);
CmsRetId psStkGetSelectItemCmdInfo(PsStkSelectItemCmdInfo *pSelectItemCmdInfo);
CmsRetId psStkSetResponse(UINT8 cmdType, PsStkProactCmdRsp *pProactCmdRsp);
void psStkProcProactiveCmdAutoResp(UINT8 cmdType, UINT16 cmdLen, UINT8 *pCmdData);

#endif

