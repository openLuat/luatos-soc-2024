#include "common_api.h"
#include "luat_debug.h"
#include "luat_rtos.h"

#include "luat_sms.h"
#include "luat_mem.h"
#include "protocol_text.h"
#include "param_ctrl.h"
#include "time.h"
#define CMD_MAX_LEN 180
#define NUMBER "17527613057"
extern luat_rtos_task_handle sms_task_handle;
static luat_rtos_task_handle send_msg_task_handle;
static luat_rtos_task_handle recv_msg_task_handle;
static luat_rtos_semaphore_t send_message_semaphore;

void luat_sms_send_pdu_msg(uint8_t *p_input, char *p_des)
{
    size_t phone_len = 0;
    size_t payload_len = 0;
    const char* phone = p_des;
    const char* payload = p_input;
    int ret = 0;
    char phone_buff[32] = {0};
    payload_len = strlen(payload);
	phone_len = strlen(phone);

    if (payload_len == 0) {
        LUAT_DEBUG_PRINT("sms is empty");
        return 0;
    }
    if (payload_len >= 140) {
        LUAT_DEBUG_PRINT("sms is too long %d", payload_len);
        return 0;
    }
    int pdu_mode = 0;
    for (size_t i = 0; i < payload_len; i++)
    {
        if (payload[i] & 0x80) {
            LUAT_DEBUG_PRINT("found non-ASCII char, using PDU mode");
            pdu_mode = 1;
            break;
        }
    }

    
    if (phone_len < 3 || phone_len > 29) {
        LUAT_DEBUG_PRINT("phone is too short or too long!! %d", phone_len);
        return 0;
    }
    // +8613416121234
    if (pdu_mode) { // PDU模式下, 必须带上国家代码
        if (phone[0] == '+') {
            memcpy(phone_buff, phone + 1, phone_len - 1);
        }
        // 13416121234
        else if (phone[0] != '8' && phone[1] != '6') {
            phone_buff[0] = '8';
            phone_buff[1] = '6';
            memcpy(phone_buff + 2, phone, phone_len);
        }
        else {
            memcpy(phone_buff, phone, phone_len);
        }
    }
    else {
        if (phone[0] == '+') {
            memcpy(phone_buff, phone + 3, phone_len - 3);
        }
        else if (phone[0] == '8' && phone[1] == '6') {
            memcpy(phone_buff, phone+2, phone_len - 2);
        }
        else {
            memcpy(phone_buff, phone, phone_len);
        }
    }
    
    
    phone_len = strlen(phone_buff);
    phone = phone_buff;
    LUAT_DEBUG_PRINT("phone %s", phone);
    if (pdu_mode) {
        char pdu[280 + 100] = {0};
        // 首先, 填充PDU头部
        strcat(pdu, "00"); // 使用内置短信中心,暂时不可设置
        strcat(pdu, "01"); // 仅收件信息, 不传保留时间
        strcat(pdu, "00"); // TP-MR, 固定填0
        sprintf_(pdu + strlen(pdu), "%02X", phone_len); // 电话号码长度
        strcat(pdu, "91"); // 目标地址格式
        // 手机方号码
        for (size_t i = 0; i < phone_len; i+=2)
        {
            if (i == (phone_len - 1) && phone_len % 2 != 0) {
                pdu[strlen(pdu)] = 'F';
                pdu[strlen(pdu)] = phone[i];
            }
            else {
                pdu[strlen(pdu)] = phone[i+1];
                pdu[strlen(pdu)] = phone[i];
            }
        }
        strcat(pdu, "00"); // 协议标识(TP-PID) 是普通GSM类型，点到点方式
        strcat(pdu, "08"); // 编码格式, UCS编码
        size_t pdu_len_offset = strlen(pdu);
        strcat(pdu, "00"); // 这是预留的, 填充数据会后更新成正确的值
        uint16_t unicode = 0;
        size_t pdu_userdata_len = 0;
        for (size_t i = 0; i < payload_len; i++)
        {
            // 首先是不是单字节
            if (payload[i] & 0x80) {
                // 非ASCII编码
                if (payload[i] && 0xE0) { // 1110xxxx 10xxxxxx 10xxxxxx
                    unicode = ((payload[i] & 0x0F) << 12) + ((payload[i+1] & 0x3F) << 6) + (payload[i+2] & 0x3F);
                    //LUAT_DEBUG_PRINT("unicode %04X %02X%02X%02X", unicode, payload[i], payload[i+1], payload[i+2]);
                    sprintf_(pdu + strlen(pdu), "%02X%02X", (unicode >> 8) & 0xFF, unicode & 0xFF);
                    i+=2;
                    pdu_userdata_len += 2;
                    continue;
                }
                if (payload[i] & 0xC0) { // 110xxxxx 10xxxxxx
                    unicode = ((payload[i] & 0x1F) << 6) + (payload[i+1] & 0x3F);
                    //LUAT_DEBUG_PRINT("unicode %04X %02X%02X", unicode, payload[i], payload[i+1]);
                    sprintf_(pdu + strlen(pdu), "%02X%02X", (unicode >> 8) & 0xFF, unicode & 0xFF);
                    i++;
                    pdu_userdata_len += 2;
                    continue;
                }
                LUAT_DEBUG_PRINT("bad UTF8 string");
                break;
            }
            // 单个ASCII字符, 但需要扩展到2位
            else {
                // ASCII编码
                strcat(pdu, "00");
                sprintf_(pdu + strlen(pdu), "%02X", payload[i]);
                pdu_userdata_len += 2;
                continue;
            }
        }
        // 修正pdu长度
        char tmp[3] = {0};
        sprintf_(tmp, "%02X", pdu_userdata_len);
        memcpy(pdu + pdu_len_offset, tmp, 2);

        // 打印PDU数据, 调试用
        LUAT_DEBUG_PRINT("PDU %s", pdu);
        payload = pdu;
        payload_len = strlen(pdu);
        phone = "";
        luat_sms_send_msg(pdu, "", 1, payload_len);
    }
}

void app_sms_send_msg(uint8_t *p_input, char *p_des)
{
    bool input_has_chinese = false;
    char* judgeChinese = (char*)p_input;
    for (int i = 0; i < strlen(judgeChinese); i++)
    {
        if (*(judgeChinese+i) & 0x80)
        {
            LUAT_DEBUG_PRINT("[DIO]The input is Chinese");
            input_has_chinese = true;
            break;
        }
    }

    if (!input_has_chinese)
    {
        LUAT_DEBUG_PRINT("[DIO]The input is English");
        luat_sms_send_msg(p_input, p_des, false, 0);
    }
    else
    {
        LUAT_DEBUG_PRINT("[DIO]The input is Chinese");
        luat_sms_send_pdu_msg(p_input, p_des);
    }
}

static void msg_send_task_proc(void *arg)
{
	int ret = -1;
    protocol_text_cmd message_id;
	uint8_t *data = NULL;
	while(1)
	{
		if(luat_rtos_message_recv(send_msg_task_handle, (uint32_t *)&message_id, (void **)&data, LUAT_WAIT_FOREVER) == 0)
		{
            app_sms_send_msg(data, NUMBER);
            luat_rtos_semaphore_take(send_message_semaphore, LUAT_WAIT_FOREVER);
            luat_rtos_task_sleep(5000);
			LUAT_MEM_FREE(data);
			data = NULL;
        }
    }
    luat_rtos_task_delete(send_msg_task_handle);
}

static void luat_sms_recv_cb(uint8_t event, void *param)
{
    if (event == 0)
    {
        LUAT_SMS_RECV_MSG_T *sms_data = NULL;
        sms_data = (LUAT_SMS_RECV_MSG_T *)malloc(sizeof(LUAT_SMS_RECV_MSG_T));
        memset(sms_data, 0x00, sizeof(LUAT_SMS_RECV_MSG_T));
        memcpy(sms_data, (LUAT_SMS_RECV_MSG_T *)param, sizeof(LUAT_SMS_RECV_MSG_T));
        int ret = luat_rtos_message_send(recv_msg_task_handle, 0, sms_data);
        if (ret != 0)
        {
            LUAT_MEM_FREE(sms_data);
            sms_data = NULL;
        }
    }
}

static void luat_sms_send_cb(int ret)
{
	LUAT_DEBUG_PRINT("send ret:[%d]", ret);
    luat_rtos_semaphore_release(send_message_semaphore);
}

static void msg_recv_task_proc(void *arg)
{
    //luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	int ret = -1;
    uint32_t message_id;
	LUAT_SMS_RECV_MSG_T *data = NULL;
    uint8_t hour=0;
    uint8_t minute=0;
    uint8_t second=0;
    int count=0;
	while(1)
	{
		if(luat_rtos_message_recv(recv_msg_task_handle, &message_id, (void **)&data, LUAT_WAIT_FOREVER) == 0)
		{
           
          
            struct tm *t = localtime(NULL);
            if ((t->tm_hour!=data->time.hour)||(t->tm_min!=data->time.minute))
            {
                LUAT_DEBUG_PRINT("time:[%02d]", t->tm_hour);
                LUAT_DEBUG_PRINT("time:[%02d]", t->tm_min);
                break;
            }
	        LUAT_DEBUG_PRINT("Dcs:[%d]", data->dcs_info.alpha_bet);
	        LUAT_DEBUG_PRINT("Time:[\"%02d/%02d/%02d,%02d:%02d:%02d %c%02d\"]", data->time.year, data->time.month, data->time.day, data->time.hour, data->time.minute, data->time.second,data->time.tz_sign, data->time.tz);
	        LUAT_DEBUG_PRINT("Phone:[%s]", data->phone_address);
	        LUAT_DEBUG_PRINT("ScAddr:[%s]", data->sc_address);
	        LUAT_DEBUG_PRINT("PDU len:[%d]", data->sms_length);
	        LUAT_DEBUG_PRINT("PDU: [%s]", data->sms_buffer);
            uint8_t* pOut = NULL;
            pOut = luat_heap_malloc(CMD_MAX_LEN + 1);
            if (NULL == pOut)
            {
                LUAT_DEBUG_PRINT("luat_sms_recv_cb assert(MemoryAlloc(%d)) failed.", CMD_MAX_LEN + 1);
                return;
            }
            memset(pOut, 0x00, CMD_MAX_LEN + 1);
            //获取默认的回显语言
            uint8_t language = 1;
            config_service_get(CFG_LANG, TYPE_BYTE, &language, sizeof(language));
            protocol_text_type which_language = ((language == 1) ? CHINESE : ENGLISH);
            protocol_text_cmd cmd = protocol_text_receive_data(data->sms_buffer, data->sms_length, pOut, which_language);
            if (cmd != PROTOCOL_MAX)
            {
                LUAT_DEBUG_PRINT("[DIO] [%s]", data->sms_buffer);
                LUAT_DEBUG_PRINT("[DIO pOut] [%s] [%d]", pOut, strlen(pOut));
	            int ret = luat_rtos_message_send(send_msg_task_handle, cmd, pOut);
	            if(ret != 0)
	            {
	            	LUAT_MEM_FREE(pOut);
	            	pOut = NULL;
	            }
            }
            else
            {
                LUAT_MEM_FREE(pOut);
	            pOut = NULL;
            }
            
			LUAT_MEM_FREE(data);
			data = NULL;
        }
    }
    luat_rtos_task_delete(recv_msg_task_handle);
}

void luat_sms_task_init(void)
{
    luat_rtos_semaphore_create(send_message_semaphore, 1);
	luat_sms_init();
    luat_sms_recv_msg_register_handler(luat_sms_recv_cb);
    luat_sms_send_msg_register_handler(luat_sms_send_cb);
    luat_rtos_task_create(&send_msg_task_handle, 3*1024, 30, "msg_send", msg_send_task_proc, NULL, 50);
    luat_rtos_task_create(&recv_msg_task_handle, 3*1024, 30, "msg_recv", msg_recv_task_proc, NULL, 50);
}