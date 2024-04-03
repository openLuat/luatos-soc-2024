/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    ec_ctlwm2m_api.h
 * Description:  API interface implementation header file for ChinaNet lwm2m
 * History:      Rev1.0   2018-11-27
 *
 ****************************************************************************/
#ifndef __EC_CTLWM2M_API_H__
#define __EC_CTLWM2M_API_H__
#include "cms_util.h"
//#include "port/ct_platform.h"
#include "ctiot_lwm2m_sdk.h"

#define MAX_PSKID_LEN         (15)
#define MAX_PSK_LEN           (32)
#define MAX_SERVER_LEN        (256)
#define MAX_IMSI_LEN          (15)

#define ENDPOINT_NAME_SIZE 150

#define MAX_RSP_LEN           (100)

typedef enum
{
    SYSTEM_STATUS_FREE,
    SYSTEM_STATUS_BUSY
}system_status_e;


#if 0
#define MAX_SM9_PARAM_SIZE       (768)
#define MAX_SM9_KEY_SIZE         (176)
#define MAX_SM9_SIGNKEY_SIZE     (112)
#define MAX_SM9_SIGN_SIZE        (161)

typedef struct _smTemplate
{
    struct _smTemplate *next;    
    uint8_t id;
} sm9Template_t;

typedef struct _ct_sm9_context
{
    struct _ct_sm9_context *next;    
    uint8_t  index;
    uint8_t  valid;  //0:not valid;1:valid. bit0:param; bit1:encryptKey; bit2:signkey; bit3:be inited
    uint16_t paramLen;
    uint16_t keyLen;
    uint8_t  ptype;  //1:encode/decode; 2:sign/verfiy;
    uint8_t* param;
    uint8_t* key;    //ptype:1 -> encryptKey  ptype:2 -> signkey
    char* imei;
}ct_sm9_context_t;

typedef struct _ct_sm9_retent
{
    uint8_t  index;
    uint8_t  bInitial;  //0:no inital 1: has inital;
    uint16_t paramLen;
    uint16_t reserve;
    uint16_t keyLen;
    uint8_t  ptype;  //1:encode/decode; 2:sign/verfiy;
    uint8_t  param[MAX_SM9_PARAM_SIZE] ;
    uint8_t  key[MAX_SM9_KEY_SIZE];
    char     imei[16];
}ct_sm9_retent_t;
#endif

uint16_t ctiot_set_endpoint_mod(uint8_t endpointMode);
uint16_t ctiot_set_authtype(uint8_t type);
uint16_t ctiot_get_authtype(uint8_t* type);
uint16_t ctiot_set_nat_mod(uint8_t natType);
uint16_t ctiot_set_uq_mod(uint8_t uqMode);
uint16_t ctiot_set_certificate_mod(uint8_t certificateMode);
void ctiot_get_mod(uint8_t* endpointMode, uint8_t* natType, uint8_t* UQMode, uint8_t* certificateMode);
uint16_t ctiot_set_pm(char* serverIP, uint16_t port, uint32_t lifeTime, uint32_t reqhandle);
void ctiot_get_pm(char* serverIP, uint16_t* port, uint32_t* lifeTime);
uint16_t ctiot_set_psk(uint8_t* psk, uint8_t pskLen, uint8_t* pskId);
void ctiot_get_psk(char* rsp, uint8_t ref);
ctiot_login_status_e ctiot_get_loginstatus(void);
ctiot_boot_flag_e ctiot_get_bootflag(void);
void ctiot_set_nnmi_mode(uint8_t type);
void ctiot_get_nnmi_mode(uint8_t* type);
uint16_t ctiot_get_recvData(uint16_t* datalen, uint8_t** dataStr);
bool ctiot_match_uri(lwm2m_uri_t uriC,lwm2m_uri_t targetUri);
void ctiot_update_sendOption(coap_packet_t* messageP,ctiot_updata_list_t* dataP);
void ctiot_send_message(ctiot_context_t* pContext,coap_packet_t* message,void* sessionH);
void ctiot_send_message_con(lwm2m_transaction_t* transacP, void * message);

uint16_t ctiot_create_socket(ctiot_context_t* pContext);
void ctiot_session_init(ctiot_context_t* pContext);
uint16_t ctiot_check_reg_condition(void);
uint16_t ctiot_reg(void);
uint16_t ctiot_check_dereg_condition(void);
void ctiot_to_dereg(void);
void ctiot_dereg_done(void);
uint16_t ctiot_update(void);
uint16_t ctiot_send(char* data,ctiot_send_mode_e sendMode, UINT8 seqNum);
uint16_t ctiot_set_idauth_pm(char* idAuthStr);

void ctiot_init_sleep_handler(void);
void ctiot_disable_sleepmode(void);
void ctiot_enable_sleepmode(void);
void ctiot_clear_session(BOOL inMainThread);

void ct_send_loop_callback(ctiot_context_t* pContext);

#ifdef  FEATURE_REF_AT_ENABLE
uint16_t ctiot_set_regswt_mode(uint8_t type);
uint16_t ctiot_get_regswt_mode(uint8_t* type);
uint16_t ctiot_get_bs_server(char* serverIP,uint16_t* port);
uint16_t ctiot_set_bs_server(char* serverIP, uint16_t port, uint32_t reqHandle);
uint16_t ctiot_del_serverIP(char* serverIP,uint16_t port);
uint16_t ctiot_get_ep(char* endpoint);
uint16_t ctiot_set_ep(char* endpoint);
uint16_t ctiot_get_lifetime(uint32_t* lifetime);
uint16_t ctiot_set_lifetime(uint32_t lifeTime);
uint16_t ctiot_get_dlpm(uint32_t* buffered, uint32_t* received, uint32_t* dropped);
uint16_t ctiot_get_ulpm(uint32_t* pending, uint32_t*sent, uint32_t*error);
uint16_t ctiot_get_dtlstype(uint8_t* type, uint8_t* natType);
uint16_t ctiot_set_dtlstype(uint8_t type, uint8_t natType);
uint16_t ctiot_reset_dtls(void);
uint16_t ctiot_get_dtls_status(uint8_t * status);
uint16_t ctiot_set_nsmi_mode(uint8_t type);
uint16_t ctiot_get_nsmi_mode(uint8_t* type);
BOOL ctiot_get_data_status(uint8_t* status, uint8_t* seqNum);
uint16_t ctiot_at_dfota_mode(uint8_t mode);
#endif

#if 0
sm9Template_t* sm9ListFind(sm9Template_t *list, uint8_t id);
void sm9ListInsert(sm9Template_t **list, sm9Template_t *listNode);
uint8_t sm9ListSize(sm9Template_t *list);
ct_sm9_context_t * insertNewCxt(uint8_t index);
uint8_t ct_sm9_init(ct_sm9_context_t* cxt);
uint8_t ct_sm9_enc(uint8_t* rid, uint8_t* plainmsg, uint16_t plainmsgLen, uint8_t* ciphermsg, uint16_t* ciphermsgLen);
uint8_t ct_sm9_dec(uint8_t* ciphermsg, uint16_t ciphermsgLen, uint8_t* plainmsg, uint16_t* plainmsgLen);
uint8_t ct_sm9_sign(uint8_t* msgSign, uint16_t msgSignLen, uint8_t* sign, uint16_t* signLen);
uint8_t ct_sm9_verify(uint8_t* imei, uint8_t* msgSign, uint16_t msgSignLen, uint8_t* sign, uint16_t signLen);
uint16_t ct_sm9_calc_endpoint(char* endpoint, char* imei);
uint8_t ct_sm2_key_gen(uint8_t* pkey, uint8_t* skey);
uint8_t ct_sm2_key_dec(uint8_t* prvkey, uint8_t* pubkey, uint8_t* cipher, uint16_t clen, uint8_t* msg, uint16_t* mlen);
#endif

#endif

