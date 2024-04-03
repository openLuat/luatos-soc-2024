
#ifndef LUAT_CONF_BSP
#define LUAT_CONF_BSP

//------------------------------------------------------
// 以下custom --> 到  <-- custom 之间的内容,是供用户配置的
// 同时也是云编译可配置的部分. 提交代码时切勿删除会修改标识
//custom -->
//------------------------------------------------------

// 外设,按需启用, 最起码启用uart和wdt库
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
#define LUAT_USE_CAMERA 1
#define LUAT_USE_OTP 1
#define LUAT_USE_WLAN 1
#define LUAT_USE_SMS 1
#define LUAT_USE_HMETA 1

#define LUAT_USE_IOTAUTH 1
#define LUAT_USE_MQTT 1
#define LUAT_USE_WEBSOCKET 1
#define LUAT_USE_IO_QUEUE 1
// #define LUAT_USE_RNDIS 1

//----------------------------
// 常用工具库, 按需启用, cjson和pack是强烈推荐启用的
#define LUAT_USE_CRYPTO  1
#define LUAT_USE_CJSON  1
#define LUAT_USE_ZBUFF  1
#define LUAT_USE_PACK  1
#define LUAT_USE_LIBGNSS  1
#define LUAT_USE_FS  1
#define LUAT_USE_SENSOR  1
#define LUAT_USE_SFUD  1
// #define LUAT_USE_SFD   1
// #define LUAT_USE_STATEM 1
// 性能测试
//#define LUAT_USE_COREMARK 1
#define LUAT_USE_IR 1
// FDB 提供kv数据库, 与nvm库类似
#define LUAT_USE_FDB 1
// fskv提供与fdb兼容的API,旨在替代fdb库
#define LUAT_USE_FSKV 1
#define LUAT_USE_I2CTOOLS 1
#define LUAT_USE_LORA 1
// #define LUAT_USE_LORA2 1
// #define LUAT_USE_MAX30102 1
// #define LUAT_USE_MLX90640 1
// zlib压缩,更快更小的实现
#define LUAT_USE_MINIZ 1
// #define LUAT_USE_FASTLZ 1
#define LUAT_USE_FTP 1
// #define LUAT_USE_HTTPSRV 1
//32bitluavm对64bit数据的额外支持
#define LUAT_USE_BIT64 1
#define LUAT_USE_W5500 1
//---------------FATFS特别配置
// fatfs的长文件名和非英文文件名支持需要180k的ROM, 非常奢侈
#define LUAT_USE_FATFS 1
//#define LUAT_USE_FATFS_CHINESE

// #define LUAT_USE_PROFILER 1
// #define LUAT_USE_MQTTCORE 1
// #define LUAT_USE_LIBCOAP 1

// #define LUAT_USE_ERCOAP 1
// #define LUAT_USE_SQLITE3 1
// #define LUAT_USE_WS2812 1
#define LUAT_USE_YMODEM 1

//----------------------------
// 高通字体, 需配合芯片使用
// #define LUAT_USE_GTFONT 1
// #define LUAT_USE_GTFONT_UTF8

//----------------------------
// 高级功能
#define LUAT_USE_REPL 1
// 多虚拟机支持,实验性,一般不启用
// #define LUAT_USE_VMX 1
// #define LUAT_USE_NES 1
#define LUAT_USE_PROTOBUF 1
#define LUAT_USE_RSA      1
// #define LUAT_USE_XXTEA    1


// 编码转换,需要大概40k空间,并非完整的GBK映射
// #define LUAT_USE_ICONV 1

// 国密算法 SM2/SM3/SM4
// #define LUAT_USE_GMSSL 1

// 禁用TLS/SSL加密功能
// #define LUAT_USE_TLS_DISABLE

// ------------------------------
// 音频相关
// TTS的特别提醒:
// TTS分2种实现:
// 1. 启用 LUAT_USE_TTS_ONCHIP, 资源文件放在片上Flash
// 2. 不启用 LUAT_USE_TTS_ONCHIP, 资源文件放片外SPI Flash
//
// 资源数据很大,需要近800k, 默认配置直接启用TTS_ONCHIP是放不下的!!!
// 1. 禁用所有UI库,包括字体和fonts库
// 2. 禁用大部分工具库,例如rsa,lora等
// 3. 缩减脚本区到 64+48模式, 甚至 32+24 模式
// 4. 使用8k的资源文件可节省100k,音质见仁见智,可选
// ------------------------------
#define LUAT_USE_MEDIA    1
// #define LUAT_USE_TTS    1
// #define LUAT_USE_TTS_ONCHIP    1
// // 仅开启TTS, 禁用AMR/MP3的解码
// #define LUAT_USE_TTS_ONLY 1
//---------------------
// UI
// LCD  是彩屏, 若使用LVGL就必须启用LCD
#define LUAT_USE_LCD
#define LUAT_USE_TJPGD
// EINK 是墨水屏
#define LUAT_USE_EINK

//---------------------
// U8G2
// 单色屏, 支持i2c/spi
// #define LUAT_USE_DISP
#define LUAT_USE_U8G2

/**************FONT*****************/
#define LUAT_USE_FONTS
/**********U8G2&LCD&EINK FONT*************/
// OPPOSANS
#define USE_U8G2_OPPOSANSM_ENGLISH 1
// #define USE_U8G2_OPPOSANSM8_CHINESE
// #define USE_U8G2_OPPOSANSM10_CHINESE
// #define USE_U8G2_OPPOSANSM12_CHINESE
// #define USE_U8G2_OPPOSANSM16_CHINESE
// #define USE_U8G2_OPPOSANSM24_CHINESE
// #define USE_U8G2_OPPOSANSM32_CHINESE
// SARASA
// #define USE_U8G2_SARASA_M8_CHINESE
// #define USE_U8G2_SARASA_M10_CHINESE
// #define USE_U8G2_SARASA_M12_CHINESE
// #define USE_U8G2_SARASA_M14_CHINESE
// #define USE_U8G2_SARASA_M16_CHINESE
// #define USE_U8G2_SARASA_M18_CHINESE
// #define USE_U8G2_SARASA_M20_CHINESE
// #define USE_U8G2_SARASA_M22_CHINESE
// #define USE_U8G2_SARASA_M24_CHINESE
// #define USE_U8G2_SARASA_M26_CHINESE
// #define USE_U8G2_SARASA_M28_CHINESE

/**********LVGL FONT*************/
// #define LV_FONT_OPPOSANS_M_8
// #define LV_FONT_OPPOSANS_M_10
// #define LV_FONT_OPPOSANS_M_12
// #define LV_FONT_OPPOSANS_M_16

//---------------------
// LVGL
// 主推的UI库, 功能强大但API繁琐
// #define LUAT_USE_LVGL
// #define LUAT_USE_LVGL_DEMO
// #define LUAT_USE_LVGL_JPG 1 // 启用JPG解码支持
// #define LUAT_USE_LVGL_PNG 1 // 启用PNG解码支持
// #define LUAT_USE_LVGL_BMP 1 // 启用BMP解码支持

#define LUAT_USE_LVGL_ARC   //圆弧 无依赖
#define LUAT_USE_LVGL_BAR   //进度条 无依赖
#define LUAT_USE_LVGL_BTN   //按钮 依赖容器CONT
#define LUAT_USE_LVGL_BTNMATRIX   //按钮矩阵 无依赖
#define LUAT_USE_LVGL_CALENDAR   //日历 无依赖
#define LUAT_USE_LVGL_CANVAS   //画布 依赖图片IMG
#define LUAT_USE_LVGL_CHECKBOX   //复选框 依赖按钮BTN 标签LABEL
#define LUAT_USE_LVGL_CHART   //图表 无依赖
#define LUAT_USE_LVGL_CONT   //容器 无依赖
#define LUAT_USE_LVGL_CPICKER   //颜色选择器 无依赖
#define LUAT_USE_LVGL_DROPDOWN   //下拉列表 依赖页面PAGE 标签LABEL
#define LUAT_USE_LVGL_GAUGE   //仪表 依赖进度条BAR 仪表(弧形刻度)LINEMETER
#define LUAT_USE_LVGL_IMG   //图片 依赖标签LABEL
#define LUAT_USE_LVGL_IMGBTN   //图片按钮 依赖按钮BTN
#define LUAT_USE_LVGL_KEYBOARD   //键盘 依赖图片按钮IMGBTN
#define LUAT_USE_LVGL_LABEL   //标签 无依赖
#define LUAT_USE_LVGL_LED   //LED 无依赖
#define LUAT_USE_LVGL_LINE   //线 无依赖
#define LUAT_USE_LVGL_LIST   //列表 依赖页面PAGE 按钮BTN 标签LABEL
#define LUAT_USE_LVGL_LINEMETER   //仪表(弧形刻度) 无依赖
#define LUAT_USE_LVGL_OBJMASK   //对象蒙版 无依赖
#define LUAT_USE_LVGL_MSGBOX   //消息框 依赖图片按钮IMGBTN 标签LABEL
#define LUAT_USE_LVGL_PAGE   //页面 依赖容器CONT
#define LUAT_USE_LVGL_SPINNER   //旋转器 依赖圆弧ARC 动画ANIM
#define LUAT_USE_LVGL_ROLLER   //滚筒 无依赖
#define LUAT_USE_LVGL_SLIDER   //滑杆 依赖进度条BAR
#define LUAT_USE_LVGL_SPINBOX   //数字调整框 无依赖
#define LUAT_USE_LVGL_SWITCH   //开关 依赖滑杆SLIDER
#define LUAT_USE_LVGL_TEXTAREA   //文本框 依赖标签LABEL 页面PAGE
#define LUAT_USE_LVGL_TABLE   //表格 依赖标签LABEL
#define LUAT_USE_LVGL_TABVIEW   //页签 依赖页面PAGE 图片按钮IMGBTN
#define LUAT_USE_LVGL_TILEVIEW   //平铺视图 依赖页面PAGE
#define LUAT_USE_LVGL_WIN   //窗口 依赖容器CONT 按钮BTN 标签LABEL 图片IMG 页面PAGE

//-------------------------------------------------
// 高级配置
//-------------------------------------------------


// 启用64位虚拟机
// #define LUAT_CONF_VM_64bit

#ifndef LUAT_SCRIPT_SIZE

#define LUAT_SCRIPT_SIZE 256
#define LUAT_SCRIPT_OTA_SIZE 192

// 适合tts_onchip的极限操作, 无需外置SPI FLASH也支持TTS.
// 一定要看 LUAT_USE_TTS_ONCHIP的说明
// #define LUAT_SCRIPT_SIZE 64
// #define LUAT_SCRIPT_OTA_SIZE 48
// #define LUAT_SCRIPT_SIZE 32
// #define LUAT_SCRIPT_OTA_SIZE 24

#endif

// 供本地快速编译Air780EPV时,自动禁用的库, 云编译时会自动删除掉
#if defined TYPE_EC718P && defined (FEATURE_IMS_ENABLE)

#undef LUAT_USE_LVGL
#undef LUAT_USE_LCD
#undef LUAT_USE_TJPGD
#undef LUAT_USE_EINK
#undef LUAT_USE_U8G2
#undef LUAT_USE_FONTS

#undef LUAT_USE_LIBGNSS
#undef LUAT_USE_BIT64
#undef LUAT_USE_PROTOBUF
#undef LUAT_USE_RSA
#undef LUAT_USE_YMODEM
#undef LUAT_USE_FATFS
#undef LUAT_USE_LORA
#undef LUAT_USE_I2CTOOLS
#undef LUAT_USE_SENSOR
#undef LUAT_USE_MINIZ
#undef LUAT_USE_FDB
#undef LUAT_USE_HMETA
#undef LUAT_USE_W5500
#undef LUAT_USE_IOTAUTH
#undef LUAT_USE_WEBSOCKET
#undef LUAT_USE_FTP
#undef LUAT_USE_CAMERA
#endif

//-------------------------------------------------------------------------------
//<-- custom
//------------------------------------------------------------------------------

// 以下选项仅开发人员可修改, 一般用户切勿自行修改
//-----------------------------
// 内存配置, 默认200k, 128 ~ 324k 可调. 324k属于极限值, 不可使用音频, 并限制TLS连接的数量不超过2个
// #ifdef LUAT_HEAP_SIZE_324K
// #define LUAT_HEAP_SIZE (324*1024)
// #endif
#ifdef LUAT_HEAP_SIZE_300K
#define LUAT_HEAP_SIZE (300*1024)
#endif
#ifdef LUAT_HEAP_SIZE_200K
#define LUAT_HEAP_SIZE (200*1024)
#endif
// // 一般无需修改. 若不需要使用SSL/TLS/TTS,可适当增加,但不应该超过256k
#ifndef LUAT_HEAP_SIZE
#define LUAT_HEAP_SIZE (250*1024)
#endif

#if defined TYPE_EC718P && defined (FEATURE_IMS_ENABLE)
#if LUAT_HEAP_SIZE > (160*1024)
#undef LUAT_HEAP_SIZE
#define LUAT_HEAP_SIZE (160*1024)
#endif

#if LUAT_SCRIPT_SIZE > 128
#undef LUAT_SCRIPT_SIZE
#undef LUAT_SCRIPT_OTA_SIZE
#define LUAT_SCRIPT_SIZE 128
#define LUAT_SCRIPT_OTA_SIZE 96
#endif
#endif

//-----------------------------

// 将UART0切换到用户模式, 默认是UNILOG模式
// 使用UART0, 日志将完全依赖USB输出, 若USB未引出或失效, 将无法获取底层日志
// 本功能仅限完全了解风险的用户使用
// #define LUAT_UART0_FORCE_USER     1
// #define LUAT_UART0_FORCE_ALT1     1
// #define LUAT_UART0_LOG_BR_12M     1

#define LUAT_GPIO_PIN_MAX 48
// #define LUAT__UART_TX_NEED_WAIT_DONE
// // 内存优化: 减少内存消耗, 会稍微减低性能
#define LUAT_USE_MEMORY_OPTIMIZATION_CODE_MMAP 1

//----------------------------------
// 使用VFS(虚拟文件系统)和内置库文件, 必须启用
#define LUAT_USE_VFS_INLINE_LIB 1
#define LUA_USE_VFS_FILENAME_OFFSET 1
// //----------------------------------

#define LUAT_WS2812B_MAX_CNT	(8)

#define LV_DISP_DEF_REFR_PERIOD 30
#define LUAT_LV_DEBUG 0

#define LV_MEM_CUSTOM 1

#define LUAT_USE_LVGL_INDEV 1 // 输入设备

#define LV_HOR_RES_MAX          (160)
#define LV_VER_RES_MAX          (80)
#define LV_COLOR_DEPTH          16

#define LV_COLOR_16_SWAP   1
#define __LVGL_SLEEP_ENABLE__

#undef LV_DISP_DEF_REFR_PERIOD
#define LV_DISP_DEF_REFR_PERIOD g_lvgl_flash_time

#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE  "common_api.h"         /*Header for the system time function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR ((uint32_t)GetSysTickMS())     /*Expression evaluating to current system time in ms*/

// #define LUAT_USE_LCD_CUSTOM_DRAW

#define __LUATOS_TICK_64BIT__

#define LUAT_RET int
#define LUAT_RT_RET_TYPE	void
#define LUAT_RT_CB_PARAM void *param

#define LUAT_USE_NETWORK 1
// LUAT_USE_TLS 通过xmake判断打开
// #define LUAT_USE_TLS 1
#define LUAT_USE_LWIP 1
#define LUAT_USE_DNS 1
#define LUAT_USE_ERR_DUMP 1
#define LUAT_USE_DHCP  1
#define LUAT_USE_ERRDUMP 1
#define LUAT_USE_FOTA 1
#define LUAT_USE_MOBILE 1
#define LUAT_USE_SNTP 1
#define LUAT_USE_WLAN_SCANONLY 1
//目前没用到的宏，但是得写在这里
#define LUAT_USE_I2S
#ifdef LUAT_USE_MEDIA
#define LUAT_SUPPORT_AMR  1
#endif
#ifndef LUAT_USE_HMETA
#define LUAT_USE_HMETA 1
#endif

// MCU引脚复用
#define LUAT_MCU_IOMUX_CTRL 1

#if defined TYPE_EC718P && defined (FEATURE_IMS_ENABLE)
#define LUAT_USE_VOLTE

#ifndef LUAT_USE_MEDIA
#define LUAT_USE_MEDIA 1
#endif

#define LUAT_USE_TTS_ONLY

#endif

// // TTS 相关
#ifdef LUAT_USE_TTS

#ifndef LUAT_USE_TTS_8K
#define LUAT_USE_TTS_16K 1
#endif // LUAT_USE_TTS_8K

#ifndef LUAT_USE_MEDIA
#define LUAT_USE_MEDIA 1
#endif

// #ifdef LUAT_USE_TTS_ONCHIP
// #undef LUAT_USE_SFUD
// #else
// #ifndef LUAT_USE_SFUD
// #define LUAT_USE_SFUD  1
// #endif // LUAT_USE_SFUD
// #endif // LUAT_USE_TTS_ONCHIP

#endif // LUAT_USE_TTS

// 当前不支持软件UART, 自动禁用之
#ifdef LUAT_USE_SOFT_UART
#undef LUAT_USE_SOFT_UART
#endif

// #ifdef LUAT_USE_TTS_ONCHIP
// #undef LUAT_SCRIPT_SIZE
// #undef LUAT_SCRIPT_OTA_SIZE
// #define LUAT_SCRIPT_SIZE 64
// #define LUAT_SCRIPT_OTA_SIZE 48
// #endif

#define LUA_SCRIPT_ADDR (FLASH_FOTA_REGION_START - (LUAT_SCRIPT_SIZE + LUAT_SCRIPT_OTA_SIZE) * 1024)
#define LUA_SCRIPT_OTA_ADDR FLASH_FOTA_REGION_START - (LUAT_SCRIPT_OTA_SIZE * 1024)

#define __LUAT_C_CODE_IN_RAM__ __attribute__((__section__(".platFMRamcode")))

#ifdef LUAT_USE_TLS_DISABLE
#undef LUAT_USE_TLS
#endif

// 关闭加密版本的UDP, 类似于TCP的TLS/SSL, 极少使用
#ifdef LUAT_CONF_TLS_DTLS_DISABLE
#undef MBEDTLS_SSL_PROTO_DTLS
#undef MBEDTLS_SSL_DTLS_ANTI_REPLAY
#undef MBEDTLS_SSL_DTLS_HELLO_VERIFY
#undef MBEDTLS_SSL_DTLS_BADMAC_LIMIT
#endif

// 关闭几个不常用的东西
#ifdef LUAT_CONF_TLS_DISABLE_NC
#undef MBEDTLS_X509_RSASSA_PSS_SUPPORT
#undef MBEDTLS_DES_C
#undef MBEDTLS_DHM_C
#undef MBEDTLS_GCM_C
#undef MBEDTLS_PSA_CRYPTO_C
#undef MBEDTLS_PKCS1_V21
#endif

// 关闭几个不常用的东西
#ifdef LUAT_CONF_TLS_DISABLE_ECP_ECDHE
#undef MBEDTLS_ECP_DP_SECP192R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP224R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP256R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP384R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP521R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP192K1_ENABLED
#undef MBEDTLS_ECP_DP_SECP224K1_ENABLED
#undef MBEDTLS_ECP_DP_SECP256K1_ENABLED
#undef MBEDTLS_ECP_DP_BP256R1_ENABLED
#undef MBEDTLS_ECP_DP_BP384R1_ENABLED
#undef MBEDTLS_ECP_DP_BP512R1_ENABLED
/* Montgomery curves (supporting ECP) */
#undef MBEDTLS_ECP_DP_CURVE25519_ENABLED
#undef MBEDTLS_ECP_DP_CURVE448_ENABLED

#undef MBEDTLS_ECP_NIST_OPTIM
// #undef MBEDTLS_ECDH_LEGACY_CONTEXT

#undef MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#undef MBEDTLS_ECDH_C
#undef MBEDTLS_ECP_C
#undef MBEDTLS_ECDSA_C

#endif

#ifdef LUAT_USE_CAMERA
#define LUAT_USE_LCD_SERVICE 1
#endif

#ifdef LUAT_SUPPORT_AMR
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#define LUAT_USE_INTER_AMR	1
#endif
#endif

#endif
