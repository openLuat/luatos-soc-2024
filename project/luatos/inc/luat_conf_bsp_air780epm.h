#define LUAT_USE_GPIO 1
#define LUAT_USE_UART 1
#define LUAT_USE_I2C  1
#define LUAT_USE_SPI  1
#define LUAT_USE_CAN  1
#define LUAT_USE_ONEWIRE 1
#define LUAT_USE_ADC  1
#define LUAT_USE_PWM  1
#define LUAT_USE_WDT  1
#define LUAT_USE_PM  1
#define LUAT_USE_RTC 1
#define LUAT_USE_WLAN 1
#define LUAT_USE_SMS 1
#define LUAT_USE_MCU  1


#define LUAT_USE_CRYPTO  1
#define LUAT_USE_CJSON  1
#define LUAT_USE_ZBUFF  1
#define LUAT_USE_PACK  1
#define LUAT_USE_MINIZ 1

#define LUAT_USE_HMETA 1
#define LUAT_USE_FSKV 1

#define LUAT_USE_MQTT 1
#define LUAT_USE_FTP 1
#define LUAT_USE_WEBSOCKET 1
#define LUAT_USE_LIBGNSS  1
#define LUAT_USE_SENSOR  1

#define LUAT_USE_IOTAUTH 1
#define LUAT_USE_BIT64 1
#define LUAT_USE_ICONV 1

#define LUAT_USE_PROTOBUF 1
#define LUAT_USE_RSA      1
#define LUAT_USE_XXTEA    1
#define LUAT_USE_GMSSL    1

#define LUAT_USE_CAMERA 1

#define LUAT_USE_LCD 1
#define LUAT_USE_TP 1
#define LUAT_USE_TJPGD 1
#define LUAT_USE_EINK 1
#define LUAT_USE_U8G2 1

#define LUAT_USE_FONTS
#define USE_U8G2_OPPOSANSM_ENGLISH 1

#define LUAT_HEAP_SIZE (1280*1024) // 1280k 1.25M

#define LUAT_SCRIPT_SIZE 256
#define LUAT_SCRIPT_OTA_SIZE 192

#define LUAT_USE_NETDRV 1
#define LUAT_USE_AIRLINK 1

// 使用DRV框架调用GPIO
#define LUAT_USE_DRV_GPIO 1
// 执行类的数据
#define LUAT_USE_AIRLINK_EXEC_SDATA 1

// 基准大小 67144
// 减去摄像头 202048, 132k
// 减去GMSSL 230936, 30k
// 减去elink 277776, 48k
// 减去lcd jpeg 283552, 6k
// 减去lcd 300512, 18k
// 减去tp 304784, 4k
// 减去fonts 329160, 25k
// 减去u8g2 360976, 32k
// 减去iconv 397080, 36k
// 减去xxtea 398080, 1k
// 减去RSA 399792, 1k
// 减去protobuf 414048, 16k
// 减去ulwip 420416, 6k
// 减去airlink 424144, 4k
// 减去netdrv 454972, 32k
// 减去libgnss 466072, 12k
