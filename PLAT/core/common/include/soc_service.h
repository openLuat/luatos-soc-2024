/*
 * am_service.h
 *
 *  Created on: 2023年8月10日
 *      Author: Administrator
 */

#ifndef CORE_INCLUDE_SOC_SERVICE_H_
#define CORE_INCLUDE_SOC_SERVICE_H_
#include "common_api.h"
#define RNDIS_CID	5
#define IMS_CID	15
typedef enum SOC_MISC_EVENT
{
	SOC_MISC_VBUS_OFF,
	SOC_MISC_VBUS_ON,
	SOC_MISC_WIFI_SCAN_DONE,
}SOC_MISC_EVENT_E;

typedef enum SOC_MOBILE_EVENT
{
	SOC_MOBILE_EVENT_CFUN = 0,
	/*!< SIM卡消息*/
	SOC_MOBILE_EVENT_SIM,
    /*!< 移动网络注册消息*/
	SOC_MOBILE_EVENT_REGISTER_STATUS,
	/*!< 小区基站信号变更消息*/
	SOC_MOBILE_EVENT_CELL_INFO,
	/*!< PDP状态消息*/
	SOC_MOBILE_EVENT_PDP,
	/*!< internet状态*/
	SOC_MOBILE_EVENT_NETIF,
	/*!< 通过基站同步时间完成*/
	SOC_MOBILE_EVENT_TIME_SYNC,
	SOC_MOBILE_EVENT_CSCON,
	SOC_MOBILE_EVENT_BEARER,
	SOC_MOBILE_EVENT_SMS,
	SOC_MOBILE_EVENT_NAS_ERROR,
	SOC_MOBILE_EVENT_IMS_REGISTER_STATUS,
	SOC_MOBILE_EVENT_CC,
	SOC_MOBILE_EVENT_FATAL_ERROR = 0xff,
}SOC_MOBILE_EVENT_E;

typedef enum SOC_MOBILE_CFUN_STATUS
{
	SOC_MOBILE_CFUN_OFF = 0,
	SOC_MOBILE_CFUN_ON,
	SOC_MOBILE_CFUN_NO_RF = 4,
}SOC_MOBILE_CFUN_STATUS_E;

typedef enum SOC_MOBILE_SIM_STATUS
{
	SOC_MOBILE_SIM_READY = 0,
	SOC_MOBILE_NO_SIM,
	SOC_MOBILE_SIM_NEED_PIN,
	SOC_MOBILE_SIM_ENTER_PIN_RESULT,
	SOC_MOBILE_SIM_NUMBER,
	SOC_MOBILE_SIM_NEED_PUK,
	SOC_MOBILE_SIM_ENTER_PIN_RESULT_OK,
}SOC_MOBILE_SIM_STATUS_E;

typedef enum SOC_MOBILE_REGISTER_STATUS
{

    /*!< 网络未注册*/
	SOC_MOBILE_STATUS_UNREGISTER,
    /*!< 网络已注册*/
	SOC_MOBILE_STATUS_REGISTERED,
	SOC_MOBILE_STATUS_SEARCHING,
	/*!< 网络注册被拒绝*/
	SOC_MOBILE_STATUS_DENIED,
	/*!< 网络状态未知*/
	SOC_MOBILE_STATUS_UNKNOW,
	/*!< 网络已注册，漫游*/
	SOC_MOBILE_STATUS_REGISTERED_ROAMING,
	SOC_MOBILE_STATUS_SMS_ONLY_REGISTERED,
	SOC_MOBILE_STATUS_SMS_ONLY_REGISTERED_ROAMING,
	SOC_MOBILE_STATUS_EMERGENCY_REGISTERED,
	SOC_MOBILE_STATUS_CSFB_NOT_PREFERRED_REGISTERED,
	SOC_MOBILE_STATUS_CSFB_NOT_PREFERRED_REGISTERED_ROAMING,
}SOC_MOBILE_REGISTER_STATUS_E;

typedef enum SOC_MOBILE_CELL_INFO_STATUS
{
	SOC_MOBILE_CELL_INFO_UPDATE = 0,
	SOC_MOBILE_SIGNAL_UPDATE,
}SOC_MOBILE_CELL_INFO_STATUS_E;

typedef enum SOC_MOBILE_PDP_STATUS
{
	SOC_MOBILE_PDP_ACTIVED = 0,
	SOC_MOBILE_PDP_DEACTIVING,
	SOC_MOBILE_PDP_DEACTIVED,
}SOC_MOBILE_PDP_STATUS_E;

typedef enum SOC_MOBILE_NETIF_STATUS
{
	SOC_MOBILE_NETIF_LINK_ON = 0,
	SOC_MOBILE_NETIF_LINK_OFF,
	SOC_MOBILE_NETIF_LINK_OOS,	//失去网络连接，尝试恢复中，等同于SOC_MOBILE_NETIF_LINK_OFF
}SOC_MOBILE_NETIF_STATUS_E;

typedef enum SOC_MOBILE_BEARER_STATUS
{
	SOC_MOBILE_BEARER_GET_DEFAULT_APN = 0,
	SOC_MOBILE_BEARER_APN_SET_DONE,
	SOC_MOBILE_BEARER_AUTH_SET_DONE,
	SOC_MOBILE_BEARER_DEL_DONE,
	SOC_MOBILE_BEARER_SET_ACT_STATE_DONE,
}SOC_MOBILE_BEARER_STATUS_E;

typedef enum SOC_MOBILE_SMS_STATUS
{
	SOC_MOBILE_SMS_READY = 0,
	SOC_MOBILE_NEW_SMS,
	SOC_MOBILE_SMS_SEND_DONE,
	SOC_MOBILE_SMS_ACK,
}SOC_MOBILE_SMS_STATUS_E;


typedef enum SOC_MOBILE_IMS_REGISTER_STATUS
{
	SOC_MOBILE_IMS_READY = 0,
}SOC_MOBILE_IMS_REGISTER_STATUS_E;

typedef enum SOC_MOBILE_CC_STATUS
{
	SOC_MOBILE_CC_READY = 0,
	SOC_MOBILE_CC_INCOMINGCALL,
	SOC_MOBILE_CC_CALL_NUMBER,
	SOC_MOBILE_CC_CONNECTED_NUMBER,
	SOC_MOBILE_CC_CONNECTED,
	SOC_MOBILE_CC_DISCONNECTED,
	SOC_MOBILE_CC_SPEECH_START,
	SOC_MOBILE_CC_MAKE_CALL_OK,
	SOC_MOBILE_CC_MAKE_CALL_FAILED,
	SOC_MOBILE_CC_ANSWER_CALL_DONE,
	SOC_MOBILE_CC_HANGUP_CALL_DONE,
	SOC_MOBILE_CC_LIST_CALL_RESULT,
	SOC_MOBILE_CC_PLAY,
}SOC_MOBILE_CC_STATUS_E;

typedef enum SOC_MOBILE_CC_MAKE_CALL_RESULT
{
	SOC_MOBILE_CC_MAKE_CALL_RESULT_OK = 0,
	SOC_MOBILE_CC_MAKE_CALL_RESULT_NO_CARRIER,
	SOC_MOBILE_CC_MAKE_CALL_RESULT_BUSY,
	SOC_MOBILE_CC_MAKE_CALL_RESULT_ERROR,
}SOC_MOBILE_CC_MAKE_CALL_RESULT_E;

typedef enum SOC_MOBILE_CC_PLAY_IND
{
	SOC_MOBILE_CC_PLAY_STOP,
	SOC_MOBILE_CC_PLAY_DIAL_TONE,
	SOC_MOBILE_CC_PLAY_RINGING_TONE,
	SOC_MOBILE_CC_PLAY_CONGESTION_TONE,
	SOC_MOBILE_CC_PLAY_BUSY_TONE,
	SOC_MOBILE_CC_PLAY_CALL_WAITING_TONE,
	SOC_MOBILE_CC_PLAY_MULTI_CALL_PROMPT_TONE,
	SOC_MOBILE_CC_PLAY_CALL_INCOMINGCALL_RINGING,
}SOC_MOBILE_CC_PLAY_IND_E;


enum
{
	SOC_SYS_CTRL_PSRAM,
	SOC_SYS_CTRL_CAMERA,
	SOC_SYS_CTRL_AUDIO,
	SOC_SYS_CTRL_LCD,
	SOC_SYS_CTRL_SOFT_DAC,
	SOC_SYS_CTRL_SPI,
	SOC_SYS_CTRL_UART,
	SOC_SYS_CTRL_GPIO,
	SOC_SYS_CTRL_USER = 16,
	SOC_SYS_CTRL_MAX = 32,
};
typedef void (* mobile_voice_data_input_fn)(uint8_t *input, uint32_t len, uint32_t sample_rate, uint8_t bits);

void soc_vsprintf(const char * format, va_list ap);


const char *soc_get_sdk_type(void);
const char *soc_get_sdk_version(void);
const char *soc_get_chip_name(void);
int soc_get_model_name(char *model, uint8_t is_full);
char *soc_get_hardware_version(void);

int soc_get_time_zone(void);
void soc_set_time_zone(int time_zone);
int soc_get_muid(char *muid, uint32_t size);

void soc_mobile_event_register_handler(void *handle);
void soc_misc_event_register_handler(void *handle);

int soc_mobile_make_call(uint8_t sim_id, char *number, uint8_t len);
void soc_mobile_hangup_call(uint8_t sim_id);
int soc_mobile_answer_call(uint8_t sim_id);
int soc_mobile_list_call(uint8_t sim_id);

void soc_mobile_speech_init(mobile_voice_data_input_fn callback);
int soc_mobile_speech_upload(uint8_t *data, uint32_t len);
#if 0
void soc_mobile_get_iccid(uint8_t sim_id, uint8_t *buf);
void soc_mobile_get_imsi(uint8_t sim_id, uint8_t *buf);
void soc_mobile_set_sim_id(uint8_t sim_id);
void soc_mobile_get_sim_id(uint8_t *sim_id, uint8_t *is_auto);
uint8_t soc_mobile_get_sim_state(uint8_t sim_id);
void soc_mobile_reset_stack(void);
void soc_mobile_fly_mode(uint8_t sim_id, uint8_t on_off);
int soc_mobile_comm_mode(uint8_t sim_id);
void soc_mobile_fatal_error_auto_reset_stack(uint8_t onoff);

void soc_mobile_get_sim_id(uint8_t *sim_id, uint8_t *is_auto);
char *soc_mobile_get_default_apn_info(uint8_t sim_id);
char *soc_mobile_get_apn_info(uint8_t sim_id, uint8_t cid, uint32_t *size);
void soc_mobile_set_ipv6_type(uint8_t on_off);
uint8_t soc_mobile_get_ipv6_type(void);
void soc_mobile_get_ip_data_traffic(uint8_t sim_id, uint64_t *uplink, uint64_t *downlink);
void soc_mobile_clear_ip_data_traffic(uint8_t sim_id, uint8_t clear_uplink, uint8_t clear_downlink);
void soc_mobile_set_user_apn_auto_active(uint8_t sim_id, uint8_t cid,
		uint8_t ip_type,
		uint8_t protocol_type,
		uint8_t *apn_name, uint8_t apn_name_len,
		uint8_t *user, uint8_t user_len,
		uint8_t *password, uint8_t password_len);
void soc_mobile_clear_apn(uint8_t sim_id);

void soc_mobile_set_rrc_release_time(uint8_t s);
void soc_mobile_set_check_sim_period(uint32_t check_sim_period);
void soc_mobile_set_check_network_period(uint32_t check_network_period);
void soc_mobile_set_cell_search_period(uint32_t search_period);

uint8_t soc_mobile_get_csq(void);
int soc_mobile_start_get_cell_list(uint8_t sim_id, uint8_t is_sync);
void soc_mobile_print_cell_info(uint8_t sim_id);
#endif

uint8_t soc_rndis_is_enable(void);
uint32_t soc_rndis_host_ip(void);
void soc_psram_speed_init(void);
void soc_psram_dma_on_off(uint32_t mask_bit, uint8_t on_off);
void soc_flash_dma_on_off(uint32_t mask_bit, uint8_t on_off);
void soc_cp_force_wakeup_on_off(uint32_t mask_bit, uint8_t on_off);
void soc_sys_force_wakeup_on_off(uint32_t mask_bit, uint8_t on_off);
void soc_doze_force_wakeup_on_off(uint32_t mask_bit, uint8_t on_off);
uint8_t soc_user_wfi_mode(void);
void soc_set_sys_signal_callback(TaskFun_t fun);
#endif /* CORE_INCLUDE_SOC_SERVICE_H_ */
