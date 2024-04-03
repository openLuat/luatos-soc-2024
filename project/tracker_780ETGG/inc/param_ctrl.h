#ifndef PARAM_CTRL_H
#define PARAM_CTRL_H

#include "luat_rtos.h"

#define  JT808_PROTOCOL_PARAM "/jt808_param.txt"
#define  JT808_PROTOCOL_DEVICE_TYPE "XXXXXX"

typedef enum
{
    CFG_HEART_INTERVAL,                         //终端心跳发送间隔
    CFG_APN_NAME,                               //主服务器 APN
    CFG_APN_USER,                               //主服务器 APN 拨号用户名
    CFG_APN_PWD,                                //主服务器 APN,拨号密码
    CFG_SERVERLOCK,                             //主服务器锁
    CFG_SERVERADDR,                             //主服务器地址
    CFG_SERVERADDR_PORT,                        //主服务器端口号
    CFG_JT_HBINTERVAL,                          //休眠时汇报时间间隔
    CFG_UPLOADTIME,                             //缺省时间汇报间隔
    CFG_UPLOADTIME_BKP,                         //缺省时间汇报间隔备份
    CFG_TURN_ANGLE,                             //拐点补传角度
    CFG_SPEED_ALARM_ENABLE,                     //速度告警使能
    CFG_SPEEDTHR,                               //最高速度
    CFG_SPEED_CHECK_TIME,                       //超速持续时间
    CFG_JT_MILIAGE,                             //车辆里程表读数
    CFG_JT_PROVINCE,                            //车辆所在的省域
    CFG_JT_CITY,                                //车辆所在的市域
    CFG_JT_OEM_ID,                              //终端制造商编号
    CFG_DEVICETYPE,                             //终端型号
    CFG_JT_DEVICE_ID,                           //终端ID
    CFG_JT_VEHICLE_NUMBER,                      //公安交通同管理部门颁发的机劢车号牌
    CFG_JT_VEHICLE_COLOR,                       //车牌颜色
    CFG_JT_AUTH_CODE,                           //终端鉴权数据
    CFG_JT_ISREGISTERED,                        //终端注册标志
    CFG_LANG,                                   //接收网络发送指令后响应语言，(0:英语,1:中文)默认中文
    CFG_TIME_ZONE,                              //时区
    CFG_APNC_MCC,                               //移动国家代码
    CFG_APNC_MNC,                               //移动网络代码
    CFG_RELAY,                                  //油电控制电路
    CFG_GPS,                                    //GPS开关
    CFG_PROTOCOL_NUM,                           //设备协议号
    CFG_FORTIFICAT_MODE,                        //设防模式(0:自动,1:手动)
    CFG_BASE_UPLOAD_ENABLE,                     //基站上传开关
    CFG_BASE_UPLOAD_INTERVAL,                   //基站上传间隔
    CFG_BASE_WAIT_GPS_TIME,                     //等待GPS定位时间
}CmdType;

typedef enum
{
    TYPE_BYTE,
    TYPE_SHORT,
    TYPE_BOOL,
    TYPE_STRING,
}ParamType;

typedef enum
{
    UNCHARGED,             //未充电
    CHARGING,              //充电中
    CHARGEFULL,            //充满
}ChargeType;

typedef struct
{
    uint8_t addr[40];                           //服务器地址
    uint32_t port;                              //服务器端口
    uint16_t province;                          //省域
    uint16_t city;                              //市域
    uint8_t oemid[5];                           //终端制造商编号
    uint8_t devicetype[11];                      //终端型号
    uint8_t deviceid[11];                        //终端ID
    uint8_t vehiclecolor;                       //车牌颜色
    uint8_t vehiclenumber[12];                  //公安交通同管理部门颁发的机劢车号牌
    uint16_t uploadtime;                        //缺省时间汇报间隔
    uint16_t heart_interval;                    //终端心跳发送间隔
    uint8_t language;                           //接收网络发送指令后响应语言，(0:英语,1:中文)默认中文
    uint8_t timezone;                           //时区，东8区：8，西八区： 58，西时区默认前面加50区分，省略符号
    uint8_t protocol_num;                       //设备协议号
    uint8_t fortificat_mode;                    //设防模式
    uint8_t base_upload_control;                //基站上传开关
    uint8_t base_upload_time;                   //基站上传间隔
    uint8_t base_wait_gps_time;                 //等待GPS定位时间

    uint32_t miliage;                           //车辆里程表读数
    uint8_t apn_name[20];                       //主服务器 APN
    uint8_t apn_user[30];                       //主服务器 APN 拨号用户名
    uint8_t apn_pwd[20];                        //主服务器 APN,拨号密码
    uint16_t hbinterval;                        //休眠时汇报时间间隔
    uint16_t turn_angle;                        //拐点补传角度
    uint8_t speed_enable;                       //超速告警使能
    uint8_t speedthr;                           //最高速度
    uint8_t speed_check_time;                   //超速持续时间
    uint8_t auth_code[33];                      //鉴权数据
    uint8_t isregistered;                       //注册标志
    uint16_t apnc_mcc;                          //移动国家代码
    uint8_t apnc_mnc[2];                        //移动网络代码
    uint8_t relay_control;                      //油电控制电路(0: 通油电，1：断油电)
    uint8_t gps_control;                        //GPS开关
}Jt808Msg;

void config_service_get(CmdType cmd, ParamType type, uint8_t *data, uint32_t len);
void config_service_set(CmdType cmd, ParamType type, uint8_t *data, uint32_t len);
int config_relay_set(int status);
int config_relay_get(void);
int config_gps_set(int status);
int config_gps_get(void);
int config_vbat_get(void);
int config_input_volt_get(void);
int config_charge_get(void);
int config_GS_off_on(int status);

#endif