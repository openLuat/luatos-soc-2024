

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
// #define LUAT_USE_ULWIP 1
// #define LUAT_USE_NAPT 1
#define LUAT_USE_SOFT_UART 1
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
// #define LUAT_USE_LITTLE_FLASH 1
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
// #define LUAT_USE_FATFS_CHINESE

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
// #define LUAT_USE_RECORD     1
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
// #define LV_FONT_OPPOSANS_M_14
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
#if (defined TYPE_EC718U) ||(defined TYPE_EC718UM) ||(defined TYPE_EC718HM)
#define FLASH_SIZE_8M
#endif
#ifndef LUAT_SCRIPT_SIZE


#ifdef FLASH_SIZE_8M
#define LUAT_SCRIPT_SIZE 512
#define LUAT_SCRIPT_OTA_SIZE 360
#else
#define LUAT_SCRIPT_SIZE 256
#define LUAT_SCRIPT_OTA_SIZE 192
#endif

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

#define LUAT_USE_TTS_ONLY
#endif

#ifdef FLASH_SIZE_8M
#ifndef LUAT_USE_FATFS_CHINESE
#define LUAT_USE_FATFS_CHINESE
#endif
#ifndef LUAT_USE_TTS
#define LUAT_USE_TTS
#endif
#ifndef LUAT_USE_TTS_ONCHIP
#define LUAT_USE_TTS_ONCHIP
#endif
#ifndef USE_U8G2_OPPOSANSM12_CHINESE
#define USE_U8G2_OPPOSANSM12_CHINESE
#endif
#ifndef LUAT_USE_LVGL
#define LUAT_USE_LVGL
#define LUAT_USE_LVGL_JPG 1 // 启用JPG解码支持
#define LUAT_USE_LVGL_PNG 1 // 启用PNG解码支持
#define LUAT_USE_LVGL_BMP 1 // 启用BMP解码支持
#define LV_FONT_OPPOSANS_M_12
#endif
#endif


// 后面的这些是为了生成API文档的, 不用管
#if 0
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
#define LUAT_SUPPORT_AMR  1
#define LUAT_USE_HMETA 1

#endif
