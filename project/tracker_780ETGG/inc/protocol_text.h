#ifndef PROTOCOL_TEXT_H
#define PROTOCOL_TEXT_H

typedef enum
{
    DEVICETYPE = 1,
    APN,
    GETSERVER,
    TIMER,
    RESET,
    RELAY,
    DYD,
    HFYD,
    GPSON,
    GPSOFF,
    PARAM,
    IMEI,
    ICCID,
    IMSI,
    WHERE,
    STATUS,
    LBSON,
    LBS,
    LANG,
    HBT,
    HBTS,
    SPEED,
    VERSION,
    PROTOCOL,
    APNC,
    SENALM,
    PROTOCOL_MAX,
}protocol_text_cmd;

typedef struct
{
    uint16_t result;
    uint8_t latitude[20];
    uint8_t longitude[20];
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}lbs_info_t;


typedef enum
{
    ENGLISH = 0,
    CHINESE,
}protocol_text_type;

typedef struct
{
    uint8_t *data;
    protocol_text_cmd cmd;
}protocol_text_t;

protocol_text_cmd protocol_text_receive_data(uint8_t* src, uint16_t len, uint8_t* dest, protocol_text_type flag);

#endif