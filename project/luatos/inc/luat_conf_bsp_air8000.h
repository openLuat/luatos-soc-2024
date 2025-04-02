#define LUAT_USE_GPIO 1
#define LUAT_USE_UART 1
#define LUAT_USE_I2C  1
#define LUAT_USE_SPI  1
#define LUAT_USE_ADC  1
#define LUAT_USE_PWM  1
#define LUAT_USE_WDT  1
#define LUAT_USE_PM  1
#define LUAT_USE_RTC 1
#define LUAT_USE_WLAN 1
#define LUAT_USE_SMS 1
#define LUAT_USE_MCU  1
#define LUAT_USE_ONEWIRE 1

#define LUAT_USE_CRYPTO  1
#define LUAT_USE_CJSON  1
#define LUAT_USE_ZBUFF  1
#define LUAT_USE_PACK  1
#define LUAT_USE_MINIZ 1
#define LUAT_USE_LIBGNSS 1

#define LUAT_USE_HMETA 1
#define LUAT_USE_FSKV 1

#define LUAT_USE_MQTT 1
#define LUAT_USE_FTP 1
#define LUAT_USE_WEBSOCKET 1
#define LUAT_USE_HTTPSRV 1

#define LUAT_USE_IOTAUTH 1
#define LUAT_USE_BIT64 1
#define LUAT_USE_ICONV 1

#define LUAT_USE_PROTOBUF 1
#define LUAT_USE_RSA      1
#define LUAT_USE_XXTEA    1
#define LUAT_USE_GMSSL    1

#define LUAT_USE_CAMERA 1
#define LUAT_USE_MEDIA    1
#define LUAT_USE_RECORD     1
#define LUAT_USE_TTS    1
#define LUAT_USE_TTS_ONCHIP    1

#define LUAT_USE_LCD
#define LUAT_USE_TP
#define LUAT_USE_TJPGD
#define LUAT_USE_EINK


/**********LVGL FONT*************/
// #define LV_FONT_OPPOSANS_M_8
// #define LV_FONT_OPPOSANS_M_10
#define LV_FONT_OPPOSANS_M_12
// #define LV_FONT_OPPOSANS_M_14
// #define LV_FONT_OPPOSANS_M_16

//---------------------
// LVGL
// 主推的UI库, 功能强大但API繁琐
#define LUAT_USE_LVGL
// #define LUAT_USE_LVGL_DEMO
// #define LV_USE_DEMO_WIDGETS 1
// #if LV_USE_DEMO_WIDGETS
// #define LV_DEMO_WIDGETS_SLIDESHOW 0
// #endif

#define LUAT_USE_LVGL_JPG 1 // 启用JPG解码支持
#define LUAT_USE_LVGL_PNG 1 // 启用PNG解码支持
#define LUAT_USE_LVGL_BMP 1 // 启用BMP解码支持

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

#define LUAT_USE_FONTS
#define USE_U8G2_OPPOSANSM_ENGLISH 1
#define USE_U8G2_OPPOSANSM12_CHINESE
// #define USE_U8G2_OPPOSANSM16_CHINESE
// #define USE_U8G2_SARASA_M8_CHINESE
// #define USE_U8G2_SARASA_M10_CHINESE
#define USE_U8G2_SARASA_M12_CHINESE
// #define USE_U8G2_SARASA_M14_CHINESE
// #define USE_U8G2_SARASA_M16_CHINESE

#define LUAT_HEAP_SIZE (2*1024*1024) // 2M字节

#define LUAT_SCRIPT_SIZE 512
#define LUAT_SCRIPT_OTA_SIZE 384

#define LUAT_USE_NETDRV 1
#define LUAT_USE_NETDRV_NAPT 1

#define LUAT_USE_IPERF 1

#define LUAT_USE_AIRLINK 1
// 使用DRV框架调用GPIO
#define LUAT_USE_DRV_GPIO 1
#define LUAT_USE_DRV_WLAN 1
// 执行类的数据
#define LUAT_USE_AIRLINK_EXEC_SDATA 1
