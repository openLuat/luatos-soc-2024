#include "luat_debug.h"
#include "luat_pm.h"
#include "protocol_jt808.h"
#include "protocol_text.h"
#include "param_ctrl.h"
#include "gpsmsg.h"
#include "time.h"
#include "luat_mobile.h"
#include "luat_base.h"
#include "luat_adc.h"

#define PROTOCOL_HEADER_JT808 0x7e
#define PROTOCOL_HEADER_ESCAPE 0x7d

#define GOOME_HEARTBEAT_MIN 0x0
#define GOOME_HEARTBEAT_MAX 0xffff

#define CONFIG_SPEEDTHR_MIN 0
#define CONFIG_SPEEDTHR_MAX 999

#define CONFIG_UPLOAD_TIME_MAX 1000
#define CMD_MAX_LEN 100

static Jt808MsgSave s_jt_msg_save = {0};

uint8_t BHIGH_BYTE(uint16_t data)
{
    return ((data >> 8) & 0xff);
}

uint8_t BLOW_BYTE(uint16_t data)
{
    return (data & 0xff);
}

uint16_t WHIGH_WORD(uint32_t data)
{
    return ((data >> 16) & 0xffff);
}

uint16_t WLOW_WORD(uint32_t data)
{
    return (data & 0xffff);
}

static uint32_t get_value_by_length(uint8_t *p,uint8_t l)
{
    if(l==4)
    {
        return( ((uint32_t)p[0]<<24)+((uint32_t)p[1]<<16)+((uint16_t)p[2]<<8)+p[3]);
    }
    else if(l==3)
    {
        return(((uint32_t)p[0]<<16)+((uint16_t)p[1]<<8)+p[2]);
    }
    else if(l==2)
    {
        return(((uint16_t)p[0]<<8)+p[1]);
    }
    else
    {
        return(p[0]);
    }
}

static uint8_t appconverthextobin(uint8_t ptr1,uint8_t ptr2)
{
    uint8_t dest_ptr;
    uint8_t ch = 0;

    // the bit 8,7,6,5
    ch = ptr1;
    // digital 0 - 9
    if (ch >= '0' && ch <= '9')
    {
        dest_ptr = (uint8_t)((ch - '0') << 4);
    }
    else
    {
        return -1;
    }

    // the bit 1,2,3,4
    ch = ptr2;
    // digtial 0 - 9
    if (ch >= '0' && ch <= '9')
    {
        dest_ptr |= (uint8_t)(ch - '0');
    }
    else
    {
        return -1;
    }

    return dest_ptr;
}

/**
 * @brief 打包标识位、消息头(终端手机号、流水号、消息包封装项)
 *
 * @param pdata 消息数据
 * @param idx  消息偏移量
 * @param len  消息长度
 * @param total 消息总包数
 * @param part_num 包序号
 */
static void protocol_jt_pack_head(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t total, uint16_t part_num )
{
    uint8_t imei[20] = {0};

    if((*idx) + 13 > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_head assert(len(%d)) failed.", len);
        return;
    }

    pdata[(*idx)++]=PROTOCOL_HEADER_JT808;  // 1 bytes
    (*idx) += 4;  //id 2  len 2   total 4 bytes

    if (luat_mobile_get_imei(0, imei, sizeof(imei)) <= 0)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_head can not get imei");
    }

    //取imei号后10位, 前面加"1"(0x31)
    pdata[(*idx)++] = appconverthextobin('0','1');
    pdata[(*idx)++] = appconverthextobin(imei[5],imei[6]);
    pdata[(*idx)++] = appconverthextobin(imei[7],imei[8]);
    pdata[(*idx)++] = appconverthextobin(imei[9],imei[10]);
    pdata[(*idx)++] = appconverthextobin(imei[11],imei[12]);
    pdata[(*idx)++] = appconverthextobin(imei[13],imei[14]);
    // 6 bytes

    //消息流水号
    pdata[(*idx)++] = BHIGH_BYTE(s_jt_msg_save.msg_serial);
    pdata[(*idx)++] = BLOW_BYTE(s_jt_msg_save.msg_serial);
    (s_jt_msg_save.msg_serial)++;
    // 2 bytes

    if(total > 1)  // 此时 protocol_jt_pack_id_len的参数depart值应为true
    {
        if((*idx) + 4 > len)
        {
            LUAT_DEBUG_PRINT("protocol_jt_pack_head assert(len(%d)) failed.", len);
            return;
        }

        pdata[(*idx)++] = BHIGH_BYTE(total);
        pdata[(*idx)++] = BLOW_BYTE(total);
        pdata[(*idx)++] = BHIGH_BYTE(part_num);
        pdata[(*idx)++] = BLOW_BYTE(part_num);
    }
}

/**
 * @brief 打包消息头(ID、属性)、校验码、标识位
 *
 * @param pdata 消息数据
 * @param idx  消息偏移量
 * @param cmd  消息ID
 * @param len  消息体长度
 * @param sec  数据加密标识
 * @param depart 分包标识
 */
static void protocol_jt_pack_id_len(uint8_t *pdata, uint16_t *idx, uint16_t cmd, uint16_t len, uint8_t sec, bool depart)
{
    uint8_t  check_sum=0;
    uint16_t j;
    uint16_t attribute = 0;

    // msg id 2 bytes
    pdata[1] = BHIGH_BYTE(cmd);
    pdata[2] = BLOW_BYTE(cmd);

    if(depart)
    {
        depart = 1;
    }
    else
    {
        depart = 0;
    }

    // len 2 bytes
    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    attribute = len & 0x3FF + (((uint16_t)depart) << 13) + (((uint16_t)(sec & 0x7)) << 10);
    pdata[3] = BHIGH_BYTE(attribute);
    pdata[4] = BLOW_BYTE(attribute);

    check_sum = 0;
    for (j=1; j<(*idx); j++)
    {
        check_sum ^= pdata[j];
    }
    pdata[(*idx)++] = check_sum;
    pdata[(*idx)++] = PROTOCOL_HEADER_JT808;
}

/**
 * @brief 转义处理
 *
 * @param src 数据
 * @param src_len  数据长度
 * @param dest  处理后数据
 * @param dest_len  处理后数据长度
 */
void protocol_jt_pack_escape(uint8_t *src, uint16_t src_len, uint8_t *dest, uint16_t *dest_len)
{
    uint16_t i = 0, j = 0;

    //首字节是 PROTOCOL_HEADER_JT808
    if((*dest_len) < j + 1)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_escape assert(dest_len(%d) >= %d) failed.", *dest_len, j + 1);
        return;
    }
    dest[j] = src[i];
    j++;

    // 最后一个字节也是 PROTOCOL_HEADER_JT808
    for(i = 1; i<src_len - 1; ++i)
    {
        if(src[i] == PROTOCOL_HEADER_JT808)
        {
            if((*dest_len) < j + 2)
            {
                LUAT_DEBUG_PRINT("protocol_jt_pack_escape assert(dest_len(%d) >= %d) failed.", *dest_len, j + 2);
                return;
            }

            dest[j++] = PROTOCOL_HEADER_ESCAPE;
            dest[j++] = 0x02;
        }
        else if(src[i] == PROTOCOL_HEADER_ESCAPE)
        {
            if((*dest_len) < j + 2)
            {
                LUAT_DEBUG_PRINT("protocol_jt_pack_escape assert(dest_len(%d) >= %d) failed.", *dest_len, j + 2);
                return;
            }

            dest[j++] = PROTOCOL_HEADER_ESCAPE;
            dest[j++] = 0x01;
        }
        else
        {
            if((*dest_len) < j + 1)
            {
                LUAT_DEBUG_PRINT("protocol_jt_pack_escape assert(dest_len(%d) >= %d) failed.", *dest_len, j + 1);
                return;
            }

            dest[j++] = src[i];
        }
    }

    dest[j++] = src[i++];  //last byte 0x7e
    (*dest_len) = j;
}

/**
 * @brief 反转义处理
 *
 * @param src 数据
 * @param src_len  数据长度
 */
static int protocol_jt_pack_unescape(uint8_t *src, uint16_t *src_len)
{
    uint16_t i = 0, j = 0;

    //首,尾字节是0x7e

    //其余字节
    for(i = 1,j=1; i< (*src_len - 1); ++i,++j)
    {
        if(src[i] == PROTOCOL_HEADER_ESCAPE)
        {
            if(src[i + 1] == 0x01)
            {
                src[j] = PROTOCOL_HEADER_ESCAPE;
                i++;
            }
            else if(src[i + 1] == 0x02)
            {
                src[j] = PROTOCOL_HEADER_JT808;
                i++;
            }
            else
            {
                LUAT_DEBUG_PRINT("protocol_jt_pack_unescape assert(%02x after 0x7d) failed.", src[i + 1]);
                return -1;
            }
        }
        else
        {
            if(i != j)
            {
                src[j] = src[i];
            }
        }
    }
    src[j++] = src[i++];  //last byte 0x7e
    (*src_len) = j;
    return 0;
}


/**
 * @brief 打包终端通用应答数据消息体
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体长度
 * @param return_len  消息体数据长度
 */
static void protocol_jt_pack_server_info(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
    pdata[(*idx)++] = BHIGH_BYTE(s_jt_msg_save.server_serial);
    pdata[(*idx)++] = BLOW_BYTE(s_jt_msg_save.server_serial);
    pdata[(*idx)++] = BHIGH_BYTE(s_jt_msg_save.msg_id);
    pdata[(*idx)++] = BLOW_BYTE(s_jt_msg_save.msg_id);
    pdata[(*idx)++] = s_jt_msg_save.msg_result;

    *return_len = 5;
}

/**
 * @brief 打包终端通用应答数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_general_ack(uint8_t *pdata, uint16_t *idx, uint16_t len)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_general_ack assert(luat_heap_malloc(%d)) failed.",len);
		return;
	}

    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes
    protocol_jt_pack_server_info(send, &send_len, len, &content_len);  //5 bytes
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_general_ack assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_TERM_ACK, content_len, 0, false);  // 2bytes
    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
    return;
}

/**
 * @brief 打包终端心跳数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_heartbeat_msg(uint8_t *pdata, uint16_t *idx, uint16_t len)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_heartbeat_msg assert(luat_heap_malloc(%d)) failed.", len);
		return;
	}

    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_heartbeat_msg assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_HEART_BEAT, content_len, 0, false);  // 2bytes
    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
    return;
}

/**
 * @brief 打包注销数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_logout_msg(uint8_t *pdata, uint16_t *idx, uint16_t len)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_logout_msg assert(luat_heap_malloc(%d)) failed.", len);
		return;
	}


    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_logout_msg assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_UNREGISTER, content_len, 0, false);  // 2bytes
    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
    return;
}

/**
 * @brief 打包终端注册数据消息体
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体长度
 * @param return_len  消息体数据长度
 */
static void protocol_jt_pack_register_info(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
    uint8_t value_u8;
    uint16_t value_u16;
    uint8_t at_type;
    uint8_t copy_len;
    uint16_t orginal_idx = *idx;
    uint8_t value_string[30];
	uint8_t app_ver;

    //省域ID=2byte
    config_service_get(CFG_JT_PROVINCE, TYPE_SHORT, &value_u16, sizeof(value_u16));
    pdata[(*idx)++] = BHIGH_BYTE(value_u16);
    pdata[(*idx)++] = BLOW_BYTE(value_u16);

    //市域ID=2byte
    config_service_get(CFG_JT_CITY, TYPE_SHORT, &value_u16, sizeof(value_u16));
    pdata[(*idx)++] = BHIGH_BYTE(value_u16);
    pdata[(*idx)++] = BLOW_BYTE(value_u16);

    //制造商ID=5byte
    copy_len = 5;
    memset(value_string, 0 , copy_len);
    config_service_get(CFG_JT_OEM_ID, TYPE_STRING, value_string, sizeof(value_string));
    memcpy(&pdata[(*idx)], value_string, copy_len);
    (*idx) += copy_len;

    //终端型号=8byte
    copy_len = 8;
    memset(value_string, 0 , copy_len);
    config_service_get(CFG_DEVICETYPE, TYPE_SHORT, &value_string, sizeof(value_string));
    memcpy(&pdata[(*idx)], value_string, copy_len);
    (*idx) += copy_len;


    //终端ID=7byte
    copy_len = 7;
    memset(value_string, 0 , copy_len);
    config_service_get(CFG_JT_DEVICE_ID, TYPE_STRING, value_string, sizeof(value_string));
    memcpy(&pdata[(*idx)], value_string, copy_len);
    (*idx) += copy_len;

    //车身颜色
    config_service_get(CFG_JT_VEHICLE_COLOR, TYPE_BYTE, &value_u8, sizeof(value_u8));
    pdata[(*idx)++] = value_u8;


    //车牌号码=nbyte     //default 12bytes
    config_service_get(CFG_JT_VEHICLE_NUMBER, TYPE_STRING, value_string, sizeof(value_string));
    memcpy(&pdata[(*idx)], value_string, copy_len);
    (*idx) += copy_len;

    *return_len = (*idx) - orginal_idx;
}

/**
 * @brief 打包终端注册数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_regist_msg(uint8_t *pdata, uint16_t *idx, uint16_t len)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_regist_msg assert(luat_heap_malloc(%d)) failed.", len);
		return;
	}


    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes
    protocol_jt_pack_register_info(send, &send_len, len, &content_len);   // max 57 bytes; default 33
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_regist_msg assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_REGISTER, content_len, 0, false);  // 2bytes

    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
}

/**
 * @brief 打包终端ICCID上报数据消息体
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体长度
 * @param return_len  消息体数据长度
 */
static void protocol_jt_pack_iccid(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
    uint8_t iccid[24] = {0};
    uint16_t content_len = 0;

    if (luat_mobile_get_iccid(0, iccid, sizeof(iccid)) <= 0)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_iccid can not get iccid");
    }
    content_len = strlen((const char *)iccid);
    if (0 == content_len)
    {
        memset(iccid, 0, sizeof(iccid));
    }

    if((*idx) + content_len > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_iccid assert(len(%d)) failed.", content_len);
        return;
    }

    memcpy(&pdata[*idx], &iccid[0], content_len);
    (*idx) += content_len;
    *return_len = content_len;
}

/**
 * @brief 打包终端ICCID上报数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_iccid_msg(uint8_t *pdata, uint16_t *idx, uint16_t len)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_iccid_msg assert(luat_heap_malloc(%d)) failed.", len);
		return;
	}


    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes
    protocol_jt_pack_iccid(send, &send_len, len, &content_len);  //20 bytes
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_iccid_msg assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_ICCID, content_len, 0, false);  // 2bytes
    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
    return;
}

/**
 * @brief 打包终端鉴权数据消息体
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体长度
 * @param return_len  消息体数据长度
 */
static void protocol_jt_pack_auth_code(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
    uint8_t content_len;
    uint8_t auth_code[100];

    memset(auth_code, 0x00, sizeof(auth_code));
    config_service_get(CFG_JT_AUTH_CODE, TYPE_STRING, &auth_code, sizeof(auth_code));

    //鉴权码=nbyte
    content_len = auth_code[0];
    if (content_len > 100)
    {
        auth_code[0] = 0;
        content_len = 0;
    }

    if((*idx) + content_len > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_auth_code assert(len(%d)) failed.", content_len);
        return;
    }

    memcpy(&pdata[*idx], &auth_code[1], content_len);
    (*idx) += content_len;

    *return_len = content_len;
}

/**
 * @brief 打包终端鉴权数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_auth_msg(uint8_t *pdata, uint16_t *idx, uint16_t len)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_auth_msg assert(luat_heap_malloc(%d)) failed.", len);
		return;
	}


    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes
    protocol_jt_pack_auth_code(send, &send_len, len, &content_len);
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_auth_msg assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_AUTH, content_len, 0, false);  // 2bytes

    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
    return;
}

static void protocol_jt_pack_item_num(uint8_t *pdata, uint16_t *idx, uint32_t item, uint32_t value, uint8_t item_len)
{
    uint16_t value_u16;
    uint8_t value_u8;

    pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(item));
    pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(item));
    pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(item));
    pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(item));
    pdata[(*idx)++] = item_len;

    if(item_len == 4)
    {
        pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(value));
        pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(value));
        pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(value));
        pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(value));
    }
    else if (item_len == 2)
    {
        value_u16 = value;
        pdata[(*idx)++] = BHIGH_BYTE(value_u16);
        pdata[(*idx)++] = BLOW_BYTE(value_u16);
    }
    else // if (item_len == 1)
    {
        value_u8 = value;
        pdata[(*idx)++] = value_u8;
    }

    //(*idx) += item_len;
}

static void protocol_jt_pack_item_string(uint8_t *pdata, uint16_t *idx, uint32_t item, uint8_t *pitem, uint8_t item_len)
{
    pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(item));
    pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(item));
    pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(item));
    pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(item));
    pdata[(*idx)++] = item_len;

    memcpy(&pdata[(*idx)], pitem, item_len);
    (*idx) += item_len;
}

/**
 * @brief 打包查询终端参数应答数据消息体
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体长度
 * @param return_len  消息体数据长度
 */
static void protocol_jt_pack_param_info(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
    uint16_t orginal_idx = *idx;
    uint16_t value_u16;
    uint8_t value_u8;
    uint8_t value_str[26];
    uint32_t value_u32 = 0;

    pdata[(*idx)++] = BHIGH_BYTE(s_jt_msg_save.server_serial);
    pdata[(*idx)++] = BLOW_BYTE(s_jt_msg_save.server_serial);
    pdata[(*idx)++] = 40;  // total 40 items

    config_service_get(CFG_HEART_INTERVAL, TYPE_SHORT, &value_u16, sizeof(value_u16));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_HEART_INTERVAL, value_u16, 4);

    protocol_jt_pack_item_num(pdata, idx, 0x0002, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0003, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0004, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0005, 0, 4);


    protocol_jt_pack_item_num(pdata, idx, 0x0006, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0007, 0, 4);

    config_service_get(CFG_APN_NAME, TYPE_STRING, value_str, sizeof(value_str));
    protocol_jt_pack_item_string(pdata, idx, JT_PARAM_APN, value_str, strlen((const char *)value_str));

    protocol_jt_pack_item_string(pdata, idx, 0x0011, value_str, 0);
    protocol_jt_pack_item_string(pdata, idx, 0x0012, value_str, 0);


    protocol_jt_pack_item_string(pdata, idx, 0x0013, value_str, 0);
    protocol_jt_pack_item_string(pdata, idx, 0x0014, value_str, 0);
    protocol_jt_pack_item_string(pdata, idx, 0x0015, value_str, 0);
    protocol_jt_pack_item_string(pdata, idx, 0x0016, value_str, 0);
    protocol_jt_pack_item_string(pdata, idx, 0x0017, value_str, 0);


    protocol_jt_pack_item_num(pdata, idx, 0x0018, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0019, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0020, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0021, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0022, 0, 4);

    config_service_get(CFG_JT_HBINTERVAL, TYPE_SHORT, &value_u16, sizeof(value_u16));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_HEART_LOCATE_INTERVAL, value_u16, 4);

    protocol_jt_pack_item_num(pdata, idx, 0x0028, 0, 4);

    config_service_get(CFG_UPLOADTIME, TYPE_SHORT, &value_u16, sizeof(value_u16));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_TIME_INTERVAL, value_u16, 4);

    config_service_get(CFG_TURN_ANGLE, TYPE_SHORT, &value_u16, sizeof(value_u16));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_TURN_ANGLE, value_u16, 4);

    config_service_get(CFG_SPEEDTHR, TYPE_BYTE, &value_u8, sizeof(value_u8));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_MAX_SPEED, value_u8, 4);

    config_service_get(CFG_SPEED_CHECK_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_EXCEED_SPEED_TIME, value_u8, 4);

    protocol_jt_pack_item_num(pdata, idx, 0x0057, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0058, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0059, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x005A, 0, 4);


    protocol_jt_pack_item_num(pdata, idx, 0x0070, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0071, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0072, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0073, 0, 4);
    protocol_jt_pack_item_num(pdata, idx, 0x0074, 0, 4);

    config_service_get(CFG_JT_MILIAGE, TYPE_SHORT, &value_u32, sizeof(value_u32));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_MILIAGE, value_u32, 4);

    config_service_get(CFG_JT_PROVINCE, TYPE_SHORT, &value_u16, sizeof(value_u16));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_PROVINCE, value_u16, 2);

    config_service_get(CFG_JT_CITY, TYPE_SHORT, &value_u16, sizeof(value_u16));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_CITY, value_u16, 2);

    config_service_get(CFG_JT_VEHICLE_NUMBER, TYPE_STRING, value_str, sizeof(value_str));
    protocol_jt_pack_item_string(pdata, idx, JT_PARAM_BRAND, value_str, strlen((const char *)value_str));

    config_service_get(CFG_JT_VEHICLE_COLOR, TYPE_BYTE, &value_u8, sizeof(value_u8));
    protocol_jt_pack_item_num(pdata, idx, JT_PARAM_BRAND_COLOR, value_u8, 1);

    *return_len = (*idx) - orginal_idx;
}

/**
 * @brief 打包查询终端参数应答数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_param_ack(uint8_t *pdata, uint16_t *idx, uint16_t len)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_general_ack assert(luat_heap_malloc(%d)) failed.", len);
		return;
	}


    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes
    protocol_jt_pack_param_info(send, &send_len, len, &content_len);  //5 bytes
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_general_ack assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_GET_PARAM_ACK, content_len, 0, false);  // 2bytes
    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
    return;
}

static void protocol_jt_pack_alarm_status(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len, uint8_t alarm)
{
    uint32_t sys_state;
    uint32_t alarm_state = 0;

    if((*idx) + 4 > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_alarm_status assert(len(%d)) failed.", len);
        return;
    }
    alarm_state = (alarm << 15);
/*
    sys_state = system_state_get_status_bits();


	if(sys_state & SYSBIT_ALARM_SOS)
    {
        SET_BIT0(alarm_state);
    }
    if(sys_state & SYSBIT_ALARM_SPEED)
    {
        SET_BIT1(alarm_state);
    }

    if(sys_state & SYSBIT_ALARM_LOW_POWER)
    {
        SET_BIT7(alarm_state);
    }

    if(sys_state & SYSBIT_ALARM_NO_POWER)
    {
        SET_BIT8(alarm_state);
    }

    if(sys_state & SYSBIT_ALARM_COLLIDE)
    {
        SET_BIT16(alarm_state);
    }

    if(sys_state & SYSBIT_ALARM_SPEED_UP)
    {
        SET_BIT17(alarm_state);
    }

    if(sys_state & SYSBIT_ALARM_TURN_OVER)
    {
        SET_BIT29(alarm_state);
    }

    if(sys_state & SYSBIT_ALARM_SPEED_DOWN)
    {
        SET_BIT30(alarm_state);
    }

    if(sys_state & SYSBIT_ALARM_SHARP_TURN)
    {
        SET_BIT31(alarm_state);
    }
*/
    pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(alarm_state));
    pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(alarm_state));
    pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(alarm_state));
    pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(alarm_state));
    *return_len = 4;
}

static void protocol_jt_pack_device_status(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len, nmea_msg *gpsx)
{
    bool value_bool;
    uint32_t device_status = 0;

    if(config_acc_get() == 1)
    {
        device_status |= (1 << 0);
    }

    if(gpsx->gpssta == 1)
    {
        device_status |= (1 << 1);
    }

    // 纬度
    if (gpsx->nshemi == 'S')
    {
        device_status |= (1 << 2);  //南纬
    }

    // 经度
    if (gpsx->ewhemi == 'W')
    {
        device_status |= (1 << 3);  //西经
    }

    if (config_relay_get())
    {
        device_status |= (1 << 10);
    }
     device_status |= (1 << 18);
    device_status |= (1 << 19);
    device_status |= (1 << 20);
    device_status |= (1 << 21);
    pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(device_status));
    pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(device_status));
    pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(device_status));
    pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(device_status));

    *return_len = 4;
}


static int decimal_bcd_code(int decimal)//十进制转BCD码
{
	int sum = 0, i;//i计数变量，sum返回的BCD码
	for ( i = 0; decimal > 0; i++)
	{
		sum |= ((decimal % 10 ) << ( 4*i));
		decimal /= 10;
	}
	return sum;
}


static void protocol_jt_pack_gps_info(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len, nmea_msg *gpsx)
{
    uint16_t orginal_idx = *idx;
    uint32_t latitudev = 0;
    uint32_t longitudev = 0;
    uint16_t gps_speed = 0;
    uint16_t gps_angle = 0;
    uint16_t gps_hightv = 0;
    uint8_t i;

    latitudev = gpsx->latitude;
    longitudev = gpsx->longitude;
    gps_hightv = ((uint16_t)gpsx->altitude >= 8000 || (uint16_t)gpsx->altitude < 0) ? 0 : ((uint16_t)gpsx->altitude);
    gps_speed = ((uint16_t)gpsx->speed * 10 > 1800) ? 1800 : ((uint16_t)gpsx->speed * 10);
    // gps_speed = 120;
    gps_angle = ((uint16_t)gpsx->course > 360) ? 0 : ((uint16_t)gpsx->course);
    pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(latitudev));
    pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(latitudev));
    pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(latitudev));
    pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(latitudev));

    pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(longitudev));
    pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(longitudev));
    pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(longitudev));
    pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(longitudev));

    pdata[(*idx)++] = BHIGH_BYTE(gps_hightv);
    pdata[(*idx)++] = BLOW_BYTE(gps_hightv);

    pdata[(*idx)++] = BHIGH_BYTE(gps_speed);
    pdata[(*idx)++] = BLOW_BYTE(gps_speed);

    pdata[(*idx)++] = BHIGH_BYTE(gps_angle);
    pdata[(*idx)++] = BLOW_BYTE(gps_angle);
    LUAT_DEBUG_PRINT("(%04d-%02d-%02d %02d:%02d:%02d).",gpsx->utc.year, gpsx->utc.month, gpsx->utc.date, gpsx->utc.hour, gpsx->utc.min, gpsx->utc.sec);
    struct tm stm;
    stm.tm_year=gpsx->utc.year-1900;
	stm.tm_mon= gpsx->utc.month-1;
	stm.tm_mday=gpsx->utc.date;
	stm.tm_hour=gpsx->utc.hour;
	stm.tm_min=gpsx->utc.min;
	stm.tm_sec=gpsx->utc.sec;
    time_t utctime = mktime(&stm);
    struct tm *btm;
    btm = localtime(&utctime);
    LUAT_DEBUG_PRINT("beijing time (%04d-%02d-%02d %02d:%02d:%02d)" ,btm->tm_year + 1900,btm->tm_mon + 1,btm->tm_mday,btm->tm_hour,btm->tm_min,btm->tm_sec);
    pdata[(*idx)++] = decimal_bcd_code(btm->tm_year + 1900 - 2000);
    pdata[(*idx)++] = decimal_bcd_code(btm->tm_mon + 1);
    pdata[(*idx)++] = decimal_bcd_code(btm->tm_mday);
    pdata[(*idx)++] = decimal_bcd_code(btm->tm_hour);
    pdata[(*idx)++] = decimal_bcd_code(btm->tm_min);
    pdata[(*idx)++] = decimal_bcd_code(btm->tm_sec);
    *return_len = (*idx) - orginal_idx;
}

/**
 * @brief 打包位置信息汇报数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */

void protocol_jt_pack_gps_msg(nmea_msg *gpsx, uint8_t *pdata, uint16_t *idx, uint16_t len, uint8_t alarm, uint8_t need_base_station_info)
{
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
    uint16_t content_all = 0;
	uint8_t app_ver;

	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_gps_msg assert(luat_heap_malloc(%d)) failed.", len);
		return;
	}

    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes
    protocol_jt_pack_alarm_status(send, &send_len, len, &content_len, alarm);  //4 bytes
    content_all += content_len;
    protocol_jt_pack_device_status(send, &send_len, len, &content_len, gpsx);  //4 bytes
    content_all += content_len;
    protocol_jt_pack_gps_info(send, &send_len, len, &content_len,gpsx);  //20 bytes
    content_all += content_len;
    LUAT_DEBUG_PRINT("need_base_station_info%d", need_base_station_info);
    if(need_base_station_info==1)
    {
        protocol_jt_pack_base_station_info(send, &send_len, len, &content_len);
        content_all += content_len;
    }
    else if (need_base_station_info==2)
    {
        protocol_jt_pack_gps_station_info(send, &send_len, len, &content_len);
        content_all += content_len;
    }
    else if (need_base_station_info==3)
    {
        protocol_jt_pack_volt_station_info(send, &send_len, len, &content_len);
        content_all += content_len;
    }



    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_gps_msg assert(len(%d)) failed.", len);
        luat_heap_free(send);
        return;
    }
        LUAT_DEBUG_PRINT("content_all %d",content_all);
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_LOCATE, content_all, 0, false);  // 2bytes
        LUAT_DEBUG_PRINT("content_all %d",content_all);
    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);

    return;
}

void protocol_jt_pack_base_station_info(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
	// luat_mobile_cell_info_t cell_info;
    // int ret = luat_mobile_get_cell_info(&cell_info);//同步方式获取cell_info
    // uint16_t mnc = cell_info.lte_service_info.mnc;
    // uint16_t mcc = cell_info.lte_service_info.mcc;
    // uint16_t tac = cell_info.lte_service_info.tac;
    // uint32_t rssi = cell_info.lte_service_info.rssi;
    // uint32_t cid = cell_info.lte_service_info.cid;
    // LUAT_DEBUG_PRINT("TEST CELL INFO %d, %d, %d, %d, %d", mnc, mcc, tac, cid, rssi);
    // pdata[(*idx)++] = 0x53;
    // pdata[(*idx)++] = 0x0b;
    // pdata[(*idx)++] = 0x01;
    // pdata[(*idx)++] = BHIGH_BYTE(mcc);
    // pdata[(*idx)++] = BLOW_BYTE(mcc);
    // pdata[(*idx)++] = mnc;
    // pdata[(*idx)++] = BHIGH_BYTE(tac);
    // pdata[(*idx)++] = BLOW_BYTE(tac);

    // pdata[(*idx)++] = BHIGH_BYTE(WHIGH_WORD(cid));
    // pdata[(*idx)++] = BLOW_BYTE(WHIGH_WORD(cid));
    // pdata[(*idx)++] = BHIGH_BYTE(WLOW_WORD(cid));
    // pdata[(*idx)++] = BLOW_BYTE(WLOW_WORD(cid));
    // uint8_t retRssi;
    // if (rssi <= -113)
    // {
    //     retRssi = 0;
    // }
    // else if (rssi < -52)
    // {
    //     retRssi = (rssi + 113) >> 1;
    // }
    // else
    // {
    //     retRssi = 31;
    // }
    // pdata[(*idx)++] = retRssi;
    // *return_len = 13;


    uint8_t csq = soc_mobile_get_csq();
    if(csq >= 25)
    {
        csq = 100;
    }
    else if(15 < csq < 25)
    {
        csq = 60;
    }
    else
    {
        csq = 10;
    }
    pdata[(*idx)++] = 0x30;
    pdata[(*idx)++] = BHIGH_BYTE(csq);
    pdata[(*idx)++] = BLOW_BYTE(csq);
    LUAT_DEBUG_PRINT("当前信号:%d",csq);
    csq = 0;
    *return_len = 3;
}

void protocol_jt_pack_gps_station_info(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
    extern nmea_msg gpsx;
    LUAT_DEBUG_PRINT("GPS个数:%d",gpsx.posslnum);
    pdata[(*idx)++] = 0x31;
    pdata[(*idx)++] = BHIGH_BYTE(gpsx.posslnum);
    pdata[(*idx)++] = BLOW_BYTE(gpsx.posslnum);
    *return_len = 3;
}

void protocol_jt_pack_volt_station_info(uint8_t *pdata, uint16_t *idx, uint16_t len, uint16_t *return_len)
{
    uint8_t vbat = change_vbat_volt();
    pdata[(*idx)++] = 0x04;
    pdata[(*idx)++] = BHIGH_BYTE(0);
    pdata[(*idx)++] = BLOW_BYTE(0);
    pdata[(*idx)++] = BHIGH_BYTE(vbat*258);
    pdata[(*idx)++] = BLOW_BYTE(vbat*258);
    *return_len = 5;
}

/**
 * @brief 打包终端文本数据
 *
 * @param pdata 数据
 * @param idx  数据长度
 * @param len  消息体数据长度
 */
void protocol_jt_pack_remote_ack(uint8_t *pdata, uint16_t *idx, uint16_t len, uint8_t *pRet, uint16_t retlen)
{
    const char prefix_string[] = "返回:";
    uint8_t *send;
    uint16_t send_len = 0;
    uint16_t content_len = 0;
	send = (uint8_t *) luat_heap_malloc(len);
	if (send == NULL)
	{
        LUAT_DEBUG_PRINT("protocol_jt_pack_remote_ack assert(luat_heap_malloc send(%d)) failed.", len);
		return;
	}

    protocol_jt_pack_head(send, &send_len, len, 1, 0);  // 13bytes | 17bytes

	send[send_len] = 2;	//添加JT808协议, 文本信息编码方式, 默认编码 0:GBK; 1:UCS2; 2:UTF8
	send_len++;

    content_len = retlen + strlen(prefix_string);
    content_len = snprintf((char *)&send[send_len], content_len, "%s%s", prefix_string, pRet);
    send_len += content_len;
    if((send_len + 2) > len)
    {
        LUAT_DEBUG_PRINT("protocol_jt_pack_remote_ack assert(len(%d)) failed content_len(%d) send_len(%d).", len,content_len, send_len);
        luat_heap_free(send);
        return;
    }
    protocol_jt_pack_id_len(send, &send_len, JT_CMD_TEXT_RESULT, content_len, 0, false);  // 2bytes
    *idx = len;
    protocol_jt_pack_escape(send, send_len, pdata, idx);

    luat_heap_free(send);
    return;
}

static void protocol_jt_parse_plat_ack(uint8_t* pdata, uint16_t len)
{
    uint16_t msg_info;
    uint16_t msg_len;
    uint8_t msg_depart;
    uint8_t msg_sec;
    uint8_t data_start;
    uint16_t msg_id;

    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_len = msg_len;
	msg_sec = msg_sec;

    // 流水号 2   消息号2 结果1 checksum 1 flag 1
    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_plat_ack assert(len(%d)).", len);
        return;
    }

    msg_id =  ((uint16_t)pdata[data_start + 2]<<8)+pdata[data_start + 3];
    switch(msg_id)
    {
        case JT_CMD_AUTH:
            // 0 成功,确认  ; 1失败; 2消息有误; 3不支持; 4报警;
            if(pdata[data_start + 4] == 0)
            {
                LUAT_DEBUG_PRINT("protocol_jt_parse_plat_ack auth ok(%d).", pdata[data_start + 4]);
            }
            else
            {
                uint8_t value_u8 = 0;
                LUAT_DEBUG_PRINT("protocol_jt_parse_plat_ack auth failed(%d).", pdata[data_start + 4]);
                config_service_set(CFG_JT_ISREGISTERED, TYPE_BOOL, &value_u8, sizeof(value_u8));
            }
            break;
        case JT_CMD_HEART_BEAT:
            //gps_service_after_receive_heartbeat();
            break;
        case JT_CMD_UNREGISTER:
            break;
        case JT_CMD_LOCATE:
            //gps_service_after_receive_heartbeat();
            break;
        case JT_CMD_LOCATE_MULTI:
            break;
        default:
            break;
    }
}

static void protocol_jt_parse_register_ack(uint8_t* pdata, uint16_t len)
{
    uint16_t msg_info;
    uint16_t msg_len;
    uint8_t msg_depart;
    uint8_t msg_sec;
    uint8_t data_start;
    uint8_t auth_code[100];
    uint8_t value_u8;

    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_sec = msg_sec;

    // {流水号2;结果1;鉴权码n;}checksum 1;flag 1;
    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_register_ack assert(len(%d)).", len);
        return;
    }


    //0 注册成功   3 终端 已注册
    if(pdata[data_start + 2] == 0 || pdata[data_start + 2] == 3)
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_register_ack regist ok(%d).", pdata[data_start + 2]);
        memset(auth_code, 0x00, sizeof(auth_code));
        auth_code[0] = msg_len - 3; //为兼容,第一个字节是长度
        if(auth_code[0])
        {
            memcpy(&auth_code[1], &pdata[data_start + 3], auth_code[0]);
            config_service_set(CFG_JT_AUTH_CODE, TYPE_STRING, &auth_code, auth_code[0] + 1);
            LUAT_DEBUG_PRINT("protocol_jt_parse_register_ack auth %s", &auth_code[1]);
        }

        //记录已注册成功
        value_u8 = 1;
        config_service_set(CFG_JT_ISREGISTERED, TYPE_BOOL, &value_u8, sizeof(value_u8));

        //config_service_save_to_local();

        //gps_service_after_register_response();
    }
    else
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_register_ack regist failed(%d).", pdata[data_start + 2]);
    }
}

static void protocol_jt_parse_set_param(uint8_t* pdata, uint16_t len)
{
    uint16_t msg_info = 0;
    uint16_t msg_len = 0;
    uint8_t msg_depart = 0;
    uint8_t msg_sec = 0;

    uint8_t data_start = 0;
    uint8_t data_count = 0;
    uint8_t para_idx = 0;
    uint32_t para_id = 0;
    uint8_t para_len = 0;

    uint16_t value_u16;
    uint8_t value_u8;
    uint32_t value_u32;
    uint64_t value_u64;

    s_jt_msg_save.msg_id = ((uint16_t)pdata[1]<<8)+pdata[2];
    s_jt_msg_save.server_serial = ((uint16_t)pdata[11]<<8)+pdata[12];
    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_sec = msg_sec;

    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_set_param assert(len(%d)).", len);
        return;
    }

    //参数总数
    data_count = pdata[data_start++];
    while(para_idx < data_count)
    {
        ++para_idx;
        //参数 ID(2)     参数长度(1)         参数值

        para_id = ((uint32_t)pdata[data_start]<<24)+((uint32_t)pdata[data_start+1]<<16)+((uint32_t)pdata[data_start+2]<<8)+((uint32_t)pdata[data_start+3]);
        para_len = pdata[data_start+4];
        data_start += 5;

        if(len < (data_start + para_len + 2))
        {
            LUAT_DEBUG_PRINT("protocol_jt_parse_set_param assert(len(%d)).", len);
            return;
        }

        switch(para_id)
        {

            case JT_PARAM_HEART_INTERVAL:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                if(value_u16 >= GOOME_HEARTBEAT_MIN && value_u16 <= GOOME_HEARTBEAT_MAX)
                {
                    config_service_set(CFG_HEART_INTERVAL, TYPE_SHORT, &value_u16, sizeof(value_u16));
                }
                else
                {
                    LUAT_DEBUG_PRINT("protocol_jt_parse_set_param assert(JT_PARAM_HEART_INTERVAL(%d)) failed.", util_clock(), value_u16);
                }
                break;
            case JT_PARAM_APN:
                config_service_set(CFG_APN_NAME, TYPE_STRING, &pdata[data_start], para_len);
                break;
            case JT_PARAM_APN_USR:
                config_service_set(CFG_APN_USER, TYPE_STRING, &pdata[data_start], para_len);
                break;
            case JT_PARAM_APN_PWD:
                config_service_set(CFG_APN_PWD, TYPE_STRING, &pdata[data_start], para_len);
                break;
            case JT_PARAM_IP:
                config_service_get(CFG_SERVERLOCK, TYPE_BYTE, &value_u8, sizeof(value_u8));
                if(! value_u8)
                {
                    //config_service_change_ip(CFG_SERVERADDR, &pdata[data_start], para_len);
                //    gps_service_change_config();
                }
            case JT_PARAM_PORT:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                //config_service_change_port(CFG_SERVERADDR,value_u16);
                //gps_service_change_config();
                break;
            case JT_PARAM_HEART_LOCATE_INTERVAL:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                if(value_u16 >= GOOME_HEARTBEAT_MIN && value_u16 <= CONFIG_UPLOAD_TIME_MAX)
                {
                    config_service_set(CFG_JT_HBINTERVAL, TYPE_SHORT, &value_u16, sizeof(value_u16));
                }
                else
                {
                    LUAT_DEBUG_PRINT("protocol_jt_parse_set_param assert(JT_PARAM_HEART_LOCATE_INTERVAL(%d)) failed.", value_u16);
                }
                break;
            case JT_PARAM_TIME_INTERVAL:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                if(value_u16 <= CONFIG_UPLOAD_TIME_MAX)
                {
                    config_service_set(CFG_UPLOADTIME, TYPE_SHORT, &value_u16, sizeof(value_u16));
                    config_service_set(CFG_UPLOADTIME_BKP, TYPE_SHORT, &value_u16, sizeof(value_u16));
                }
                else
                {
                    LUAT_DEBUG_PRINT("protocol_jt_parse_set_param assert(JT_PARAM_TIME_INTERVAL(%d)) failed.", value_u16);
                }
                break;
            case JT_PARAM_TURN_ANGLE:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                if(value_u16 < 180)
                {
                    config_service_set(CFG_TURN_ANGLE, TYPE_SHORT, &value_u16, sizeof(value_u16));
                }
                else
                {
                    LUAT_DEBUG_PRINT("protocol_jt_parse_set_param assert(JT_PARAM_TURN_ANGLE(%d)) failed.", value_u16);
                }
                break;
            case JT_PARAM_MAX_SPEED:
                value_u8 = (uint8_t)get_value_by_length(&pdata[data_start], para_len);
                if(0 == value_u8)
                {
                    //只修改开关,不修改速度值
                    config_service_set(CFG_SPEED_ALARM_ENABLE, TYPE_BOOL, &value_u8, sizeof(value_u8));
                }
                else if(value_u8 >=CONFIG_SPEEDTHR_MIN && value_u8 <= CONFIG_SPEEDTHR_MAX)
                {
                    config_service_set(CFG_SPEEDTHR, TYPE_BYTE, &value_u8, sizeof(value_u8));

                    //打开开关
                    value_u8 = 1;
                    config_service_set(CFG_SPEED_ALARM_ENABLE, TYPE_BOOL, &value_u8, sizeof(value_u8));
                }
                else
                {
                    LUAT_DEBUG_PRINT("protocol_jt_parse_set_param assert(JT_PARAM_MAX_SPEED(%d)) failed.", value_u8);
                }
                break;
            case JT_PARAM_EXCEED_SPEED_TIME:
                value_u8 = (uint8_t)get_value_by_length(&pdata[data_start], para_len);
                config_service_set(CFG_SPEED_CHECK_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));
                break;
            case JT_PARAM_PROVINCE:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                config_service_set(CFG_JT_PROVINCE, TYPE_SHORT, &value_u16, sizeof(value_u16));
                break;
            case JT_PARAM_CITY:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                config_service_set(CFG_JT_CITY, TYPE_SHORT, &value_u16, sizeof(value_u16));
                break;
            case JT_PARAM_BRAND:
                value_u8 = (para_len > 12) ? 12 : para_len;
                config_service_set(CFG_JT_VEHICLE_NUMBER, TYPE_STRING, &pdata[data_start], value_u8);
                break;
            case JT_PARAM_BRAND_COLOR:
                value_u16 = (uint16_t)get_value_by_length(&pdata[data_start], para_len);
                config_service_set(CFG_JT_VEHICLE_COLOR, TYPE_SHORT, &value_u16, sizeof(value_u16));
                break;
            case JT_PARAM_MILIAGE:
                value_u32 = get_value_by_length(&pdata[data_start], para_len);
                value_u64 = value_u32 * 100;
                //system_state_set_mileage(value_u64);
                break;
            default:
                break;
        }

        data_start += para_len;
    }

    if(data_count > 0)
    {
        //config_service_save_to_local();

        //发送ack
        //gps_service_after_server_req();

        //gps_service_change_config();
    }
}

static void protocol_jt_parse_get_param(uint8_t* pdata, uint16_t len)
{
    uint16_t msg_info;
    uint16_t msg_len;
    uint8_t msg_depart;
    uint8_t msg_sec;
    uint8_t data_start;

    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_sec = msg_sec;

    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_get_param assert(len(%d)).", len);
        return;
    }

    s_jt_msg_save.msg_id = ((uint16_t)pdata[1]<<8)+pdata[2];
    s_jt_msg_save.server_serial = ((uint16_t)pdata[11]<<8)+pdata[12];

    //gps_service_after_param_get();
}

static void protocol_jt_parse_terminal_control(uint8_t* pdata, uint16_t len)
{
    uint16_t msg_info;
    uint16_t msg_len;
    uint8_t msg_depart;
    uint8_t msg_sec;
    uint8_t data_start;
	int ret = 0;

    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_sec = msg_sec;

    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_terminal_control assert(len(%d)).", len);
        return;
    }

    s_jt_msg_save.msg_id = ((uint16_t)pdata[1]<<8)+pdata[2];
    s_jt_msg_save.server_serial = ((uint16_t)pdata[11]<<8)+pdata[12];

    switch(pdata[data_start])
    {
        case 4: //终端复位
            luat_os_reboot(1);
            ret=0;
            LUAT_DEBUG_PRINT("protocol_jt_parse_terminal_control hard_ware_reboot.");
            break;
        case 0x64: //断油电
            ret = config_relay_set(0);
            LUAT_DEBUG_PRINT("protocol_jt_parse_terminal_control hard_ware_set_relay broke.");
            break;
        case 0x65: //通油电
            ret = config_relay_set(1);
            LUAT_DEBUG_PRINT("protocol_jt_parse_terminal_control hard_ware_set_relay good.");
            break;
    }

    if(0 == ret)
    {
        s_jt_msg_save.msg_result = 0;  //成功
    }
    else
    {
        s_jt_msg_save.msg_result = 1;  //失败
    }

    //发送ack
    //gps_service_after_server_req();
}

static void protocol_jt_parse_gps_req(uint8_t* pdata, uint16_t len)
{
    uint16_t msg_info;
    uint16_t msg_len;
    uint8_t msg_depart;
    uint8_t msg_sec;
    uint8_t data_start;

    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_sec = msg_sec;

    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_gps_req assert(len(%d)).", len);
        return;
    }

    s_jt_msg_save.msg_id = ((uint16_t)pdata[1]<<8)+pdata[2];
    s_jt_msg_save.server_serial = ((uint16_t)pdata[11]<<8)+pdata[12];
    s_jt_msg_save.msg_result = 0;  //成功


    //发送定位数据
    //gps_service_after_server_locate_req();
}

static void protocol_jt_parse_general_msg(uint8_t* pdata, uint16_t len)
{
    uint16_t msg_info;
    uint16_t msg_len;
    uint8_t msg_depart;
    uint8_t msg_sec;
    uint8_t data_start;

    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_sec = msg_sec;

    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_general_msg assert(len(%d)).", len);
        return;
    }

    s_jt_msg_save.msg_id = ((uint16_t)pdata[1]<<8)+pdata[2];
    s_jt_msg_save.server_serial = ((uint16_t)pdata[11]<<8)+pdata[12];
    s_jt_msg_save.msg_result = 0;  //成功


    //发送ack
    //gps_service_after_server_req();
}

static void protocol_jt_parse_text(uint8_t* pdata, uint16_t len)
{
    uint8_t* pOut = NULL;
    uint16_t out_len = 0;
    uint8_t idx = 0;
    uint16_t msg_info;
    uint16_t msg_len;
    uint8_t msg_depart;
    uint8_t msg_sec;
    uint8_t data_start;

    msg_info = ((uint16_t)pdata[3]<<8)+pdata[4];

    // 保留(15 14)  分包(13) 加密(12 11 10) 长度(9-0)
    msg_len = msg_info & 0x3FF;
    msg_depart =(msg_info & JT_DATA_DEPART_FLAG)?1:0;
    msg_sec = (msg_info & JT_DATA_SEC_FLAG) >> 10;
    data_start = msg_depart ? 17: 13;
	msg_sec = msg_sec;

    if(len != (data_start + msg_len + 2))
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_text assert(len(%d)).", len);
        return;
    }

    s_jt_msg_save.msg_id = ((uint16_t)pdata[1]<<8)+pdata[2];
    s_jt_msg_save.server_serial = ((uint16_t)pdata[11]<<8)+pdata[12];
    s_jt_msg_save.msg_result = 0;  //成功

    pOut = luat_heap_malloc(CMD_MAX_LEN + 1);
    if (NULL == pOut)
    {
        LUAT_DEBUG_PRINT("protocol_concox_parse_remote_msg assert(luat_heap_malloc(%d)) failed.", CMD_MAX_LEN + 1);
        return;
    }

    memset(pOut, 0x00, CMD_MAX_LEN + 1);

    // flag(1) command(N)
    idx = data_start + 1;

    pdata[data_start + msg_len] = '\0';
    protocol_text_receive_data(&pdata[idx], msg_len - 1, pOut, ENGLISH);

    out_len = strlen((char *)pOut);

    if (out_len > 0)
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_text data:%s", pOut);
        //gps_service_after_receive_remote_msg(pOut, out_len);
    }

    //发送ack
    //gps_service_after_server_req();
    luat_heap_free(pOut);
}

/**
 * @brief 数据解析
 *
 * @param pdata 数据
 * @param len  数据长度
 */
void protocol_jt_parse_msg(uint8_t *pdata, uint16_t len)
{
    uint16_t j;
    uint8_t check_sum=0;
    uint16_t cmd;

    if(0 != protocol_jt_pack_unescape(pdata, &len))
    {
        return;
    }

    // check_sum 1byte  end_tag 1byte
    for(j=1;j<(len-2);j++)
    {
        check_sum ^= pdata[j];
    }

    if(check_sum != pdata[len-2])
    {
        LUAT_DEBUG_PRINT("protocol_jt_parse_msg assert(checksum(%02x=%02x, %02x, %02x)) failed.", check_sum, pdata[len-3], pdata[len-2], pdata[len-1]);
        return;
    }

    cmd=((uint16_t)pdata[1]<<8)+pdata[2];
    switch(cmd)
    {
        case JT_CMD_PLAT_ACK:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(JT_CMD_PLAT_ACK).");
            protocol_jt_parse_plat_ack(pdata,len);

            break;
        case JT_CMD_REGISTER_ACK:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(JT_CMD_REGISTER_ACK).");
            protocol_jt_parse_register_ack(pdata,len);

            break;
        case JT_CMD_SET_PARAM:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(JT_CMD_SET_PARAM).");
            protocol_jt_parse_set_param(pdata,len);

            break;
        case JT_CMD_GET_PARAM:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(JT_CMD_GET_PARAM).");
            protocol_jt_parse_get_param(pdata,len);

            break;
        case JT_CMD_TERM_CTL:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(JT_CMD_TERM_CTL).");
            protocol_jt_parse_terminal_control(pdata,len);

            break;
        case JT_CMD_LOCATE_REQ:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(JT_CMD_LOCATE_REQ).");
            protocol_jt_parse_gps_req(pdata,len);

            break;
        case JT_CMD_TEXT:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(JT_CMD_TEXT).");
            protocol_jt_parse_text(pdata,len);

            break;
        default:
            LUAT_DEBUG_PRINT("protocol_jt_parse_msg(%04x).",cmd);
            //log_service_print_hex((const char *)pdata,len);
            protocol_jt_parse_general_msg(pdata,len);

            break;
    }
}
