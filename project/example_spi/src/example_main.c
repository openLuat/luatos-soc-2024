#include "common_api.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_spi.h"

#define TEST_SPI_ID   1


luat_rtos_task_handle spi_task_handle;

static void task_test_spi(void *param)
{
    luat_spi_t spi_conf = {
        .id = TEST_SPI_ID,
        .CPHA = 0,
        .CPOL = 0,
        .dataw = 8,
        .bit_dict = 0,
        .master = 1,
        .mode = 1,             // mode设置为1，全双工
        .bandrate = 25600000,
        .cs = 8
    };

    luat_spi_setup(&spi_conf);

	static uint8_t send_buf[4] = {0x90,0x80,0x70,0x60};
    static uint8_t recv_buf[4] = {0};
    while (1)
    {
        luat_rtos_task_sleep(1000);
        luat_spi_transfer(TEST_SPI_ID, send_buf, 4, recv_buf, 4);
        for (size_t i = 0; i < 4; i++){
            LUAT_DEBUG_PRINT("send_buf[%d]: 0x%02X",i,send_buf[i]);
        }
        for (size_t i = 0; i < 4; i++){
            LUAT_DEBUG_PRINT("recv_buf[%d]: 0x%02X",i,recv_buf[i]);
        }
        memset(recv_buf, 0x0, 4);
    }
}

static void task_demo_spi(void)
{
    luat_rtos_task_create(&spi_task_handle, 4 * 1024, 20, "spi", task_test_spi, NULL, 0);
}

INIT_TASK_EXPORT(task_demo_spi,"1");