#ifndef PROTOCOL_JT808_H
#define PROTOCOL_JT808_H

#define JT_DATA_DEPART_FLAG      0x2000   //分包标志    0010 0000 0000 0000
#define JT_DATA_SEC_FLAG      0x1C00      //加密标志    0001 1100 0000 0000

typedef enum
{
    JT_CMD_TERM_ACK = 0x0001,   //终端通用应答
    JT_CMD_HEART_BEAT = 0x0002,   //终端心跳
    JT_CMD_UNREGISTER = 0x0003,   //注销
    JT_CMD_PLAT_ACK = 0x8001,   //平台通用应答
    JT_CMD_REGISTER = 0x0100,   //终端注册
    JT_CMD_REGISTER_ACK = 0x8100,   //终端注册应答
    JT_CMD_ICCID = 0x0120,       //终端上报iccid
    JT_CMD_AUTH = 0x0102,        //终端鉴权
    JT_CMD_SET_PARAM = 0x8103,   //设置终端参数
    JT_CMD_GET_PARAM = 0x8104,   //查询终端参数
    JT_CMD_TERM_CTL = 0x8105,   //终端控制
    JT_CMD_GET_PARAM_ACK = 0x0104,   //查询终端参数应答
    JT_CMD_LOCATE = 0x0200,   //位置信息汇报
    JT_CMD_LOCATE_REQ = 0x8201,   //位置信息查询
    JT_CMD_LOCATE_REQ_ACK = 0x0201,   //位置信息查询应答
    JT_CMD_LOCATE_MULTI = 0x704,   //定位数据批量上传
    JT_CMD_TEXT_RESULT = 0x6006,  // 终端文本数据上报
    JT_CMD_TEXT = 0x8300        //文本信息下发 
}Jt808CmdType;

typedef enum
{
    JT_PARAM_HEART_INTERVAL = 0x0001,   //终端心跳发送间隔,单位为（s）
    JT_PARAM_APN = 0x0010,   //主服务器 APN,无线通信拨号访问点。
    JT_PARAM_APN_USR = 0x0011,   //主服务器 APN 拨号用户名
    JT_PARAM_APN_PWD = 0x0012,   //主服务器 APN,拨号密码
    JT_PARAM_IP = 0x0013,   //主服务器地址,IP 或域名
    JT_PARAM_IP_BAK = 0x0017,   //备份服务器地址,IP 或域名
    JT_PARAM_PORT = 0x0018,   //服务器 TCP 端口
    JT_PARAM_LOCATE_TYPE = 0x0020,   //位置汇报策略,0:定时汇报；1:定距汇报；2:定时和定距汇报
    JT_PARAM_HEART_LOCATE_INTERVAL = 0x0027,   //休眠时汇报时间间隔,单位为秒（s）,>0
    JT_PARAM_TIME_INTERVAL = 0x0029,   //缺省时间汇报间隔,单位为秒（s）,>0
    JT_PARAM_DISTANCE_INTERVAL = 0x002C,   //缺省距离汇报间隔,单位为米（m）,>0
    JT_PARAM_TURN_ANGLE = 0x0030,   //拐点补传角度,<180
    JT_PARAM_MAX_SPEED = 0x0055,   //最高速度,单位为千米每小时（km/h）
    JT_PARAM_EXCEED_SPEED_TIME = 0x0056,   //超速持续时间,单位为秒（s）
    JT_PARAM_MILIAGE = 0x0080,   //车辆里程表读数,1/10km
    JT_PARAM_PROVINCE = 0x0081,   //车辆所在的省域 ID,1～255
    JT_PARAM_CITY = 0x0082,   //车辆所在的市域 ID,1～255
    JT_PARAM_BRAND = 0x0083,   //公安交通同管理部门颁发的机劢车号牌
    JT_PARAM_BRAND_COLOR = 0x0084,   //车牌颜色,按照 JT/T415—2006 中 5.4.12 的规定
}Jt808ParamType;

/*用于记录发送消息的序列号, 收到消息的序列号等 */
typedef struct
{
    uint16_t msg_serial;  //上传数据序列号, 每次发消息加一
    uint16_t server_serial;  //数据序列号
    uint16_t msg_id;     // 消息号
    uint8_t msg_result;     // 消息号0:成功/确认；1:失败；2:消息有误；3:不支持； 4:报警处理确认
}Jt808MsgSave;

#endif