#ifndef __ECU_RS485_H__
#define __ECU_RS485_H__

#include "common_api.h"
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"

#include "luat_uart.h"
#include "luat_gpio.h"
#include "luat_pm.h"


#define RS485_UART_ID       UART_ID1
#define RS485_BAUDRATE      9600
#define RS485_DATA_BITS     8
#define RS485_STOP_BITS     1
#define RS485_DELAY         10
#define RS485_PIN           HAL_GPIO_11
#define RS485_EN            HAL_GPIO_15
#define RS485_EN_LEVEL      Luat_GPIO_HIGH

#define RS485_RXBUFF_SIZE   1024
#define ECU_RXBUF_SIZE      32
#define ECU_POLL_TIME_MS    120

enum{
    ECU_RX_ERROR,
	ECU_RX_METER,
    ECU_RX_CONTROL,
    ECU_RX_BMS,
    ECU_RX_CENTRALLOCK,

	ECU_RX_METER_LEN        = 8,
    ECU_RX_CONTROL_LEN      = 17,
    ECU_RX_BMS_LEN          = 17,
    ECU_RX_CENTRALLOCK_LEN  = 9,
};

/*
    RS485协议数据帧格式：
    帧起始符1 帧起始符2 对方地址 本机地址 数据长度  数据 校验位  帧结束符(0x0D)

    主机:ECU                帧起始符:0xC5 0x5C
    从机:BSM/仪表/控制器     帧起始符:0xB6 0x6B

    设备    设备地址
    ECU:     0xAA
    BSM:     0x5A
    仪表:     0xBA
    控制器:   0xDA
    中控锁:   0x5B
*/

#define ECU_ADDR_ECU            0xAA
#define ECU_ADDR_BMS            0x5A
#define ECU_ADDR_METER          0xBA
#define ECU_ADDR_CONTROL        0xDA
#define ECU_ADDR_CENTRALLOCK    0x5B

#define ECU_MASTER_START1       0xC5
#define ECU_MASTER_START2       0x5C
#define ECU_SLAVE_START1        0xB6
#define ECU_SLAVE_START2        0x6B
#define ECU_FLAG_END            0x0D

typedef struct{
    const uint8_t start1;                           // 帧起始符1 0xC5
    const uint8_t start2;                           // 帧起始符2 0x5C
    const uint8_t addr1;                            // 对方地址 0xBA
    const uint8_t addr2;                            // 本机地址 0xAA
    const uint8_t data_len;                         // 数据长度 0x0E
    uint8_t battery_percent;                        // 剩余电量百分比
    uint8_t current;                                // 控制器电流
    uint8_t speed_value;                            // 速度值
    uint8_t temp;                                   // 控制器温度
    uint8_t time_clock;                             // 时间时钟
    uint8_t time_minute;                            // 时间分钟
    union{
        struct{
            uint8_t fault_control_communication:1;  // 控制器通讯故障
            uint8_t fault_control_overcurrent:1;    // 控制器过流故障
            uint8_t fault_control_powertube:1;      // 控制器功率管故障
            uint8_t fault_control_hall:1;           // 控制器霍尔故障
            uint8_t fault_control_handlebar:1;      // 控制器转把故障
            uint8_t fault_battery_communication:1;  // 电池通讯故障
            uint8_t fault_charge_overcurrent:1;     // 充电过流
            uint8_t fault_charge_overvoltage:1;     // 充电过压
        };
        uint8_t fault_code1;                        // 故障代码1
    };
    union{
        struct{
            uint8_t fault_battery_temp_high:1;      // 电池高温预警
            uint8_t fault_discharge_overcurrent:1;  // 电池放电过流
            uint8_t fault_charge_temp_low:1;        // 充电低温
            uint8_t fault_discharge_temp_low:1;     // 放电低温
            uint8_t fault_connect_network:1;        // 网络连接异常
            uint8_t fault_connect_gps:1;            // GPS连接异常
        };
        uint8_t fault_code2;                        // 故障代码2
    };
    union{
        struct{
            uint8_t flag_stop:1;                    // 住车信号标志位
            uint8_t flag_undervoltage:1;            // 欠压标志位
            uint8_t flag_charge:1;                  // 充电标志位
            uint8_t flag_bluetooth:1;               // 蓝牙标志位
        };
        uint8_t flag_status;                        // 状态标志位
    };
    uint8_t gear;                                   // 档位数据
    uint8_t motor_speed_h;                          // 电机转速高位
    uint8_t motor_speed_l;                          // 电机转速低位
    uint8_t mileage_this;                           // 本次行驶里程
    uint8_t mileage_remain;                         // 剩余行驶里程
    uint8_t flag_cr;                                // 有效数据校验加密位
    const uint8_t flag_end;                         // 帧结束符 0x0D
}ecu_meter_t;                                       // ECU -> 仪表

typedef struct{
    uint8_t start1;                                 // 帧起始符1 0xB6
    uint8_t start2;                                 // 帧起始符2 0x6B
    uint8_t addr1;                                  // 对方地址 0xAA
    uint8_t addr2;                                  // 本机地址 0xBA
    uint8_t data_len;                               // 数据长度 0x01
    uint8_t fault_code;                             // 故障代码
    uint8_t flag_cr;                                // 有效数据校验加密位
    uint8_t flag_end;                               // 帧结束符 0x0D
}meter_ecu_t;                                       // 仪表 -> ECU

typedef struct{
    uint8_t start1;                                 // 帧起始符1 0xC5
    uint8_t start2;                                 // 帧起始符2 0x5C
    uint8_t addr1;                                  // 对方地址 0xDA
    uint8_t addr2;                                  // 本机地址 0xAA
    uint8_t data_len;                               // 数据长度 0x02
    uint8_t fault_code;                             // 故障代码
    uint8_t flag_status;                            // 状态标志位
    uint8_t flag_cr;                                // 有效数据校验加密位
    uint8_t flag_end;                               // 帧结束符 0x0D
}ecu_control_t;                                     // ECU -> 控制器

typedef struct{
    uint8_t start1;                                 // 帧起始符1 0xB6
    uint8_t start2;                                 // 帧起始符2 0x6B
    uint8_t addr1;                                  // 对方地址 0xAA
    uint8_t addr2;                                  // 本机地址 0xDA
    uint8_t data_len;                               // 数据长度 0x0A
    uint8_t gear;                                   // 控制器档位
    uint8_t current_h;                              // 控制器电流高位
    uint8_t current_l;                              // 控制器电流低位
    uint8_t motor_speed_h;                          // 电机转速高位
    uint8_t motor_speed_l;                          // 电机转速低位
    uint8_t temp;                                   // 控制器温度
    union{
        struct{
            uint8_t fault_powertube:1;              // 功率管故障
            uint8_t fault_overcurrent:1;            // 过流故障
            uint8_t fault_temp:1;                   // 过温故障
            uint8_t fault_hall:1;                   // 霍尔故障
            uint8_t fault_undervoltage:1;           // 欠压故障
            uint8_t fault_stall:1;                  // 堵转故障
            uint8_t fault_handlebar:1;              // 转把故障
        };
        uint8_t fault_code;                         // 故障代码
    };
    uint8_t flag_status;                            // 状态标志位
    uint8_t flag_stop;                              // 住车信号标志位
    uint8_t motor_lock;                             // 锁电机字节
    uint8_t flag_cr;                                // 有效数据校验加密位
    uint8_t flag_end;                               // 帧结束符 0x0D
}control_ecu_t;                                     // 控制器 -> ECU

typedef struct{
    uint8_t start1;                                 // 帧起始符1 0xC5
    uint8_t start2;                                 // 帧起始符2 0x5C
    uint8_t addr1;                                  // 对方地址 0x5A
    uint8_t addr2;                                  // 本机地址 0xAA
    uint8_t data_len;                               // 数据长度 0x01
    uint8_t call;                                   // 呼叫字符
    uint8_t flag_cr;                                // 有效数据校验加密位
    uint8_t flag_end;                               // 帧结束符 0x0D
}ecu_bms_t;                                         // ECU -> BMS

typedef struct{
    uint8_t start1;                                 // 帧起始符1 0xB6
    uint8_t start2;                                 // 帧起始符2 0x6B
    uint8_t addr1;                                  // 对方地址 0xAA
    uint8_t addr2;                                  // 本机地址 0x5A
    uint8_t data_len;                               // 数据长度 0x0A
    uint8_t battery_voltage;                        // 电池电压
    uint8_t battery_rercentage;                     // 剩余电量百分比
    uint8_t battery_temp;                           // 电池温度
    uint8_t battery_current;                        // 电池充放电电流
    uint8_t charge_nums_h;                          // 电池充电次数高位
    uint8_t charge_nums_l;                          // 电池充电次数低位
    uint8_t discharge_nums_h;                       // 电池放电次数高位
    uint8_t discharge_nums_l;                       // 电池放电次数低位
    union{
        struct{
            uint8_t fault_overvoltage:1;            // 过压
            uint8_t fault_charge_overcurrent:1;     // 充电过流
            uint8_t fault_discharge_overcurrent:1;  // 放电过流
            uint8_t fault_short_circuit:1;          // 短路
            uint8_t fault_charge_temp_high:1;       // 充电高温
            uint8_t fault_charge_temp_low:1;        // 充电低温
            uint8_t fault_undervoltage:1;           // 欠压
            uint8_t fault_discharge_temp_low:1;     // 放电低温
        };
        uint8_t fault_code;                         // 故障代码
    };
    uint8_t flag_status;                            // 状态标志位
    uint8_t flag_cr;                                // 有效数据校验加密位
    uint8_t flag_end;                               // 帧结束符 0x0D
}bms_ecu_t;                                         // BMS -> ECU

typedef struct{
    uint8_t start1;                                 // 帧起始符1 0xC5
    uint8_t start2;                                 // 帧起始符2 0x5C
    uint8_t addr1;                                  // 对方地址 0x5B
    uint8_t addr2;                                  // 本机地址 0xAA
    uint8_t data_len;                               // 数据长度 0x01
    uint8_t command;                                // 指令参数
    uint8_t flag_cr;                                // 有效数据校验加密位
    uint8_t flag_end;                               // 帧结束符 0x0D
}ecu_centrallock_t;                                 // ECU -> 中控锁

typedef struct{
    uint8_t start1;                                 // 帧起始符1 0xB6
    uint8_t start2;                                 // 帧起始符2 0x6B
    uint8_t addr1;                                  // 对方地址 0xAA
    uint8_t addr2;                                  // 本机地址 0x5B
    uint8_t data_len;                               // 数据长度 0x02
    uint8_t status_lock;                            // 锁状态
    uint8_t status_boot;                            // 启动状态
    uint8_t flag_cr;                                // 有效数据校验加密位
    uint8_t flag_end;                               // 帧结束符 0x0D
}centrallock_ecu_t;                                 // 中控锁 -> ECU

typedef struct{
    ecu_meter_t ecu_meter;                          // ECU -> 仪表
    meter_ecu_t meter_ecu;                          // 仪表 -> ECU
    ecu_control_t ecu_control;                      // ECU -> 控制器
    control_ecu_t control_ecu;                      // 控制器 -> ECU
    ecu_bms_t ecu_bms;                              // ECU -> BMS
    bms_ecu_t bms_ecu;                              // BMS -> ECU
    ecu_centrallock_t ecu_centrallock;              // ECU -> 中控锁
    centrallock_ecu_t centrallock_ecu;              // 中控锁 -> ECU
}ecu_rs485_t;                                       // ECU RS485协议数据结构



#endif /* __ECU_RS485_H__ */
