/*
 * Copyright (c) 2023 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "common_api.h"
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"
#include "luat_mcu.h"
#include "luat_uart.h"
#include "luat_gpio.h"
#include "luat_pm.h"

#include "ecu_rs485.h"

static luat_rtos_task_handle ecu_rs485_task_handle = NULL;

static ecu_rs485_t ecu_rs485 = {
    .ecu_meter = {
        .start1 = ECU_MASTER_START1,
        .start2 = ECU_MASTER_START2,
        .addr1 = ECU_ADDR_METER,
        .addr2 = ECU_ADDR_ECU,
        .data_len = 0x0E,
        .flag_end = ECU_FLAG_END,
    },
    .ecu_control = {
        .start1 = ECU_MASTER_START1,
        .start2 = ECU_MASTER_START2,
        .addr1 = ECU_ADDR_CONTROL,
        .addr2 = ECU_ADDR_ECU,
        .data_len = 0x02,
        .flag_end = ECU_FLAG_END,
    },
    .ecu_bms = {
        .start1 = ECU_MASTER_START1,
        .start2 = ECU_MASTER_START2,
        .addr1 = ECU_ADDR_BMS,
        .addr2 = ECU_ADDR_ECU,
        .data_len = 0x01,
        .flag_end = ECU_FLAG_END,
    },
    .ecu_centrallock = {
        .start1 = ECU_MASTER_START1,
        .start2 = ECU_MASTER_START2,
        .addr1 = ECU_ADDR_CENTRALLOCK,
        .addr2 = ECU_ADDR_ECU,
        .data_len = 0x01,
        .flag_end = ECU_FLAG_END,
    },
};

static uint8_t ecu_rs485_cr(uint8_t* data, uint8_t length){
    uint8_t cr = 0;
    for (size_t i = 0; i < length; i++){
        cr ^= data[i];
    }
    return cr;
}

static int ecu_rs485_send_meter(ecu_rs485_t* ecu_rs485){
    ecu_rs485->ecu_meter.flag_cr = ecu_rs485_cr(((uint8_t)&(ecu_rs485->ecu_meter.data_len)), ecu_rs485->ecu_meter.data_len+1);
    luat_uart_write(RS485_UART_ID, &ecu_rs485->ecu_meter, sizeof(ecu_meter_t));
    return 0;
}

static int ecu_rs485_send_control(ecu_rs485_t* ecu_rs485){
    ecu_rs485->ecu_control.flag_cr = ecu_rs485_cr(((uint8_t)&(ecu_rs485->ecu_control.data_len)), ecu_rs485->ecu_control.data_len+1);
    luat_uart_write(RS485_UART_ID, &ecu_rs485->ecu_control, sizeof(ecu_control_t));
    return 0;
}

static int ecu_rs485_send_bms(ecu_rs485_t* ecu_rs485){
    ecu_rs485->ecu_bms.flag_cr = ecu_rs485_cr(((uint8_t)&(ecu_rs485->ecu_bms.data_len)), ecu_rs485->ecu_bms.data_len+1);
    luat_uart_write(RS485_UART_ID, &ecu_rs485->ecu_bms, sizeof(ecu_bms_t));
    return 0;
}

static int ecu_rs485_send_centrallock(ecu_rs485_t* ecu_rs485){
    ecu_rs485->ecu_centrallock.flag_cr = ecu_rs485_cr(((uint8_t)&(ecu_rs485->ecu_centrallock.data_len)), ecu_rs485->ecu_centrallock.data_len+1);
    luat_uart_write(RS485_UART_ID, &ecu_rs485->ecu_centrallock, sizeof(ecu_centrallock_t));
    return 0;
}

static int ecu_rs485_parse(uint8_t* data, uint32_t data_len){
    uint8_t flag_cr = 0,event = ECU_RX_ERROR;
    if (data[0]==ECU_SLAVE_START1 && data[1]==ECU_SLAVE_START2 && data[2]==ECU_ADDR_ECU && data[data_len-1]==ECU_FLAG_END){
        // for (size_t i = 0; i < data_len; i++){
        //     LUAT_DEBUG_PRINT("%02d 0x%02x", i,data[i]);
        // }
        if (data[3]==ECU_ADDR_BMS && data_len == ECU_RX_BMS_LEN){
            memcpy(&ecu_rs485.bms_ecu, data, data_len);
            flag_cr = ecu_rs485_cr(((uint8_t*)&(ecu_rs485.bms_ecu.data_len)), ecu_rs485.bms_ecu.data_len+1);
            if (flag_cr == ecu_rs485.bms_ecu.flag_cr){
                event = ECU_RX_BMS;
            }else{
                goto _error;
            }
        }else if (data[3]==ECU_ADDR_METER && data_len == ECU_RX_METER_LEN){
            memcpy(&ecu_rs485.meter_ecu, data, data_len);
            flag_cr = ecu_rs485_cr(((uint8_t*)&(ecu_rs485.meter_ecu.data_len)), ecu_rs485.meter_ecu.data_len+1);
            if (flag_cr == ecu_rs485.meter_ecu.flag_cr){
                event = ECU_RX_METER;
            }else{
                goto _error;
            }
        }else if (data[3]==ECU_ADDR_CONTROL && data_len == ECU_RX_CONTROL_LEN){
            memcpy(&ecu_rs485.control_ecu, data, data_len);
            flag_cr = ecu_rs485_cr(((uint8_t*)&(ecu_rs485.control_ecu.data_len)), ecu_rs485.control_ecu.data_len+1);
            if (flag_cr == ecu_rs485.control_ecu.flag_cr){
                event = ECU_RX_CONTROL;
            }else{
                goto _error;
            }
        }else if (data[3]==ECU_ADDR_CENTRALLOCK && data_len == ECU_RX_CENTRALLOCK_LEN){
            memcpy(&ecu_rs485.centrallock_ecu, data, data_len);
            flag_cr = ecu_rs485_cr(((uint8_t*)&(ecu_rs485.centrallock_ecu.data_len)), ecu_rs485.centrallock_ecu.data_len+1);
            
            if (flag_cr == ecu_rs485.centrallock_ecu.flag_cr){
                event = ECU_RX_CENTRALLOCK;
            }else{
                goto _error;
            }
        }else{
            goto _error;
        }
        luat_rtos_event_send(ecu_rs485_task_handle, event, 0, 0, 0, 0);
        return 0;
    }
_error:
    luat_rtos_event_send(ecu_rs485_task_handle, event, 0, 0, 0, 0);
    LUAT_DEBUG_PRINT("ecu_rs485_uart_recv_cb error flag_cr:0x%02x",flag_cr);
    return -1;
}

static void ecu_rs485_uart_recv_cb(int uart_id, uint32_t data_len){
    static uint8_t data[ECU_RXBUF_SIZE] = {0};
    if (uart_id == RS485_UART_ID){
        if (data_len && data_len <= ECU_RXBUF_SIZE){
            luat_uart_read(RS485_UART_ID, data, data_len);
            ecu_rs485_parse(data,data_len);
        }else{
            luat_rtos_event_send(ecu_rs485_task_handle, ECU_RX_ERROR, 0, 0, 0, 0);
        }
    }
}

static void ecu_rs485_task(void *param){
    luat_event_t event;
    uint32_t tick1,tick2,ecu_send_done = ECU_RX_CENTRALLOCK;
    luat_gpio_cfg_t gpio_cfg;
    // rs485 pin init
    luat_gpio_set_default_cfg(&gpio_cfg);
    gpio_cfg.pin = RS485_EN;
    luat_gpio_open(&gpio_cfg);
    luat_gpio_set(RS485_EN, RS485_EN_LEVEL);// 485 ON
    // rs485 uart init
    luat_uart_t uart_rs485 = {
        .id = RS485_UART_ID,
        .baud_rate = RS485_BAUDRATE,
        .data_bits = RS485_DATA_BITS,
        .stop_bits = RS485_STOP_BITS,
        .bufsz = RS485_RXBUFF_SIZE,
        .pin485 = RS485_PIN,
        .delay = RS485_DELAY,
    };

    luat_uart_setup(&uart_rs485);
    luat_uart_ctrl(RS485_UART_ID, LUAT_UART_SET_RECV_CALLBACK, ecu_rs485_uart_recv_cb);

    while (1){
        switch(ecu_send_done){
            case ECU_RX_BMS:
                ecu_rs485_send_meter(&ecu_rs485);
                ecu_send_done = ECU_RX_METER;
                break;
            case ECU_RX_METER:
                ecu_rs485_send_control(&ecu_rs485);
                ecu_send_done = ECU_RX_CONTROL;
                break;
            case ECU_RX_CONTROL:
                ecu_rs485_send_centrallock(&ecu_rs485);
                ecu_send_done = ECU_RX_CENTRALLOCK;
                break;
            case ECU_RX_CENTRALLOCK:
                ecu_rs485_send_bms(&ecu_rs485);
                ecu_send_done = ECU_RX_BMS;
                break;
            default:
                LUAT_DEBUG_PRINT("ECU_ERROR");
                break;
        }
        tick1 = luat_mcu_ticks();
        int ret = luat_rtos_event_recv(ecu_rs485_task_handle, 0, &event, NULL, ECU_POLL_TIME_MS);
        tick2 = luat_mcu_ticks();
        if (ret == 0){
            switch(event.id){
                case ECU_RX_BMS:
                    LUAT_DEBUG_PRINT("ECU_RX_BMS");
                    break;
                case ECU_RX_METER:
                    LUAT_DEBUG_PRINT("ECU_RX_METER");
                    break;
                case ECU_RX_CONTROL:
                    LUAT_DEBUG_PRINT("ECU_RX_CONTROL");
                    break;
                case ECU_RX_CENTRALLOCK:
                    LUAT_DEBUG_PRINT("ECU_RX_CENTRALLOCK");
                    break;
                default:
                    LUAT_DEBUG_PRINT("ECU_RX_ERROR");
                    break;
            }
        }
        if (ECU_POLL_TIME_MS-(tick2-tick1)>0){
            luat_rtos_task_sleep(ECU_POLL_TIME_MS-(tick2-tick1));
        }
    }
}

static void task_demo_init(void){
   luat_rtos_task_create(&ecu_rs485_task_handle, 1024 * 8, 30, "ecu_rs485", ecu_rs485_task, NULL, 16);
}

INIT_TASK_EXPORT(task_demo_init, "1");
