/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_http_task.h
*
*  Description: Process http(s) client related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _AT_CTWING_TASK_H_
#define _AT_CTWING_TASK_H_

#include "at_util.h"
#include "ctw_tcp.h"


typedef enum
{
    CTW_HTTP_AUTHREG_REQ,
    CTW_HTTP_SEND_REQ,
    CTW_HTTP_KEEPLIVE_REQ,
} ctwHttpReq_e;
    
typedef struct _list_t
{
    struct _list_t* next;
    uint8_t        id;
}list_t;

typedef struct ctwHttpReq
{
    struct ctwHttpReq * next;
    uint8_t id;
    uint8_t reqType;
    uint32_t nextSendTime;
    uint8_t    sendTimes;
    uint16_t   atHandle;
    char* httpTopic;
    char* httpData;
}ctwHttpReq_t;

/*
 * APPL SGID: APPL_CTW, related PRIM ID
*/
enum applCtwPrimId_Enum
{
    APPL_CTW_PRIM_ID_BASE = 0,

    APPL_CTW_HTTP_REG_CNF,
    APPL_CTW_HTTP_SEND_CNF,
    APPL_CTW_TCP_REG_CNF,
    APPL_CTW_TCP_SEND_CNF,
    APPL_CTW_TCP_DEREG_CNF,
    APPL_CTW_TCP_DL_IND,
    APPL_CTW_MQTT_REG_CNF,
    APPL_CTW_MQTT_SEND_CNF,
    APPL_CTW_MQTT_DL_IND,
    APPL_CTW_MQTT_FOTA_IND,
    APPL_CTW_PRIM_ID_END = 0xFF
};

enum CTW_CMD_TYPE
{
    CTW_HTTP_REG_COMMAND,
    CTW_HTTP_SEND_COMMAND,
    CTW_HTTP_DEREG_COMMAND,
    CTW_TCP_REG_COMMAND,
    CTW_TCP_SEND_COMMAND,
    CTW_MQTT_REG_COMMAND,
    CTW_MQTT_SEND_COMMAND,
    CTW_MQTT_DEREG_COMMAND,
    CTW_MQTT_FOTA_UPDATA_COMMAND,

};
    
typedef struct
{
    uint16_t athandle;
    uint8_t cmd_type;
    uint8_t Qos;
    char* topic;
    char* data;
    uint16_t datalen;
    uint16_t ackMode;
    uint16_t msgId;
    uint32_t port;
    uint32_t fotaFlag;
} ctwCmdMsg_t;

typedef struct
{
    uint8_t ret;
} ctwCnfCmdMsg_t;

typedef struct
{
    void *str;
} ctwIndMsg_t;

CmsRetId ctwHttpRegReq(uint16_t athandle);
CmsRetId ctwHttpSendReq(uint16_t athandle, char* topic, char* data);
void ctwHttpDeregReq(void);
CmsRetId ctwTcpRegReq(uint16_t athandle);
CmsRetId ctwTcpSendReq(uint16_t athandle, char* data, uint16_t len);
void ctwTcpDeregReq(void);
uint8_t ctwTcpRegStatus(void);
void ctwTcpSetRegStatus(uint8_t status);
bool ctwTcpGetRecvTaskOut(void);
CmsRetId ctwMqttRegReq(uint16_t athandle);
CmsRetId ctwMqttSendReq(uint16_t athandle, uint8_t Qos, char* topic, char* data, uint16_t len);
CmsRetId ctwMqttDeregReq(uint16_t athandle);
CmsRetId ctwMqttFotaReq(uint16_t athandle);

#endif

