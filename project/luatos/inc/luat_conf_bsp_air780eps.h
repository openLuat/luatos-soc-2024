
// #if !defined(TYPE_EC716U)
// #error("Air780EPS只能是EC718U芯片")
// #endif


#define LUAT_USE_UART 1
#define LUAT_USE_GPIO 1
#define LUAT_USE_I2C  1
#define LUAT_USE_SPI  1
#define LUAT_USE_ADC  1
#define LUAT_USE_PWM  1
#define LUAT_USE_WDT  1
#define LUAT_USE_PM  1
#define LUAT_USE_MCU  1
#define LUAT_USE_RTC 1
#define LUAT_USE_OTP 1
#define LUAT_USE_WLAN 1
#define LUAT_USE_SMS 1
#define LUAT_USE_HMETA 1

#define LUAT_USE_IOTAUTH 1
#define LUAT_USE_MQTT 1
#define LUAT_USE_FTP 1
#define LUAT_USE_WEBSOCKET 1
#define LUAT_USE_SOFT_UART 1

#define LUAT_USE_CRYPTO  1
#define LUAT_USE_CJSON  1
#define LUAT_USE_ZBUFF  1
#define LUAT_USE_PACK  1
#define LUAT_USE_FS  1
#define LUAT_USE_SENSOR  1

#define LUAT_USE_FSKV 1
#define LUAT_USE_I2CTOOLS 1
#define LUAT_USE_MINIZ 1
#define LUAT_USE_BIT64 1
#define LUAT_USE_ICONV 1

#define LUAT_USE_PROTOBUF 1
#define LUAT_USE_RSA      1
#define LUAT_USE_XXTEA    1
#define LUAT_USE_GMSSL    1

#define LUAT_SCRIPT_SIZE 512
#define LUAT_SCRIPT_OTA_SIZE 360

#define LUAT_HEAP_SIZE (300*1024)
