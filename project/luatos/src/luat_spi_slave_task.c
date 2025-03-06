
#include "common_api.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include <string.h>//add for memset
#include "bget.h"
#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_netdrv.h"
#include "luat_network_adapter.h"
#include "luat_gpio.h"
#include "soc_spi.h"
#include "luat_spinet.h"
#include "luat_spi.h"
#include "driver_gpio.h"

#define LUAT_LOG_TAG "spi.slave"
#include "luat_log.h"

static luat_rtos_task_handle spi_task_handle;

static int is_init;

static void task_slave(void *param);

// void luat_napt_spi_task_init(void) {
// 	if (is_init == 0) {
// 		is_init = 1;
// 		LLOGD("启动SPI从机线程");
// 		create_event_task(task_slave, NULL, 4096, 50, 64, "slave");
// 	}
// }

// int32_t slave_spi_callback(void *pData, void *pParam)
// {
// 	return send_event_to_task(pParam, NULL, 1, 0, 0, 0, 0);
// }
static int32_t slave_spi_cs(void *pData, void *pParam)
{
	return send_event_to_task(pParam, NULL, 2, 0, 0, 0, 0);
}

static void task_slave(void *param) {
#if 0
#else
	#define TEST_ID 0
	#define SPI_CS 8
	#define BLOCK_TEST_LEN 8000
	void *task_handle = xTaskGetCurrentTaskHandle();
	GPIO_IomuxEC7XX(24, 1, 0, 0);
	GPIO_IomuxEC7XX(25, 1, 0, 0);
	GPIO_IomuxEC7XX(26, 1, 0, 0);
	GPIO_PullConfig(24, 1, 1);
	GPIO_PullConfig(25, 1, 1);
	GPIO_PullConfig(26, 1, 1);

	GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(SPI_CS, 0), 0, 0, 0);
	GPIO_Config(SPI_CS, 1, 0);
	GPIO_ExtiConfig(SPI_CS, 0, 1, 0);
	GPIO_ExtiSetCB(SPI_CS, slave_spi_cs, task_handle);
	uint32_t rx_len;
	uint8_t *tx_buf = malloc(BLOCK_TEST_LEN);
	uint8_t *rx_buf = malloc(BLOCK_TEST_LEN);
	OS_EVENT event;
	SPI_SlaveInit(TEST_ID, 255, 1, 104000000, NULL, NULL);
	SPI_SlaveTransferStart(TEST_ID, tx_buf, rx_buf, 8000);
	// TODO 设置RDY脚的状态
	DBG("slave mode %s %s", __DATE__, __TIME__);
	while(1)
	{
		get_event_from_task(task_handle,0, &event, NULL, 0);
		rx_len = SPI_SlaveTransferStopAndGetRxLen(TEST_ID);
		LLOGD("event id %ld", event.ID);
		DBG("total %u, %x,%x,%x,%x", rx_len, rx_buf[0], rx_buf[1], rx_buf[rx_len - 2], rx_buf[rx_len - 1]);
		memcpy(tx_buf, rx_buf, rx_len);
		// DBG("total %u, %x,%x,%x,%x", rx_len, rx_buf[0], rx_buf[1], rx_buf[rx_len - 2], rx_buf[rx_len - 1]);
		SPI_SlaveTransferStart(TEST_ID, tx_buf, rx_buf, 8000);
	}
#endif
}



void luat_airlink_start_slave(void) {
    luat_rtos_task_create(&spi_task_handle, 8 * 1024, 20, "spi", task_slave, NULL, 1024);
}
