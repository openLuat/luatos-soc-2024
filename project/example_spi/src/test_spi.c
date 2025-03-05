#include "common_api.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_spi.h"
#include "luat_pm.h"
#include "luat_gpio.h"

#define TEST_SPI_ID   0
#define TEST_BUFF_SIZE (1600)
#define TEST_CS_PIN HAL_GPIO_8
#define TEST_RDY_PIN HAL_GPIO_26

static uint8_t start;
static uint8_t slave_rdy;
luat_rtos_task_handle spi_task_handle;
luat_rtos_task_handle gpio_task_handle;

static int gpio_level_irq(void *data, void* args)
{
	start = 1;
	return 0;
}

static int slave_rdy_irq(void *data, void* args) {
    slave_rdy = 1;
    luat_rtos_event_send(gpio_task_handle, 1, 2, 3, 4, 100);


    return 0;
}

static void print_buff(const char* tag, uint8_t* buff, size_t len) {
    static char tmpbuff[1024] = {0};
    for (size_t i = 0; i < len; i+=8)
    {
        // sprintf(tmpbuff + i * 2, "%02X", buff[i]);
        // LLOGD("SPI TX[%d] 0x%02X", i, buff[i]);
        LUAT_DEBUG_PRINT("SPI %s [%04X-%04X] %02X%02X%02X%02X%02X%02X%02X%02X", tag, i, i + 8, 
            buff[i+0], buff[i+1], buff[i+2], buff[i+3], 
            buff[i+4], buff[i+5], buff[i+6], buff[i+7]);
    }
    // LLOGD("SPI0 %s", tmpbuff);
}


static void task_test_spi(void *param)
{
    luat_spi_t spi_conf = {
        .id = TEST_SPI_ID,
        .CPHA = 1,
        .CPOL = 1,
        .dataw = 8,
        .bit_dict = 0,
        .master = 1,
        .mode = 1,             // mode设置为1，全双工
		.bandrate = 31000000,
        .cs = 255
    };
    luat_pm_iovolt_ctrl(0, 3300);
    luat_spi_setup(&spi_conf);
	luat_gpio_cfg_t gpio_cfg;

    // 触发脚
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = HAL_GPIO_0;
	gpio_cfg.mode = LUAT_GPIO_IRQ;
	gpio_cfg.irq_type = LUAT_GPIO_RISING_IRQ;
	gpio_cfg.pull = LUAT_GPIO_PULLDOWN;
	gpio_cfg.irq_cb = gpio_level_irq;
	luat_gpio_open(&gpio_cfg);

    // 从机准备好脚
    luat_gpio_set_default_cfg(&gpio_cfg);
    gpio_cfg.pin = TEST_RDY_PIN;
    gpio_cfg.mode = LUAT_GPIO_IRQ;
    gpio_cfg.irq_type = LUAT_GPIO_FALLING_IRQ;
    gpio_cfg.pull = LUAT_GPIO_PULLUP;
    gpio_cfg.irq_cb = slave_rdy_irq;
    luat_gpio_open(&gpio_cfg);
    LUAT_DEBUG_PRINT(" gpio rdy setup done %d", TEST_RDY_PIN);

    // CS片选脚
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = TEST_CS_PIN;
	gpio_cfg.mode = LUAT_GPIO_OUTPUT;
	gpio_cfg.pull = LUAT_GPIO_PULLUP;
    gpio_cfg.output_level = 1;
	luat_gpio_open(&gpio_cfg);

    int i;
	static uint8_t send_buf[TEST_BUFF_SIZE] = {0x90,0x80,0x70,0x60};
    static uint8_t recv_buf[TEST_BUFF_SIZE] = {0};
    for(i = 0; i < TEST_BUFF_SIZE; i++)
    {
    	send_buf[i] = (uint8_t)(i & 0xFF);
    }
    while (1)
    {
        while(!start){luat_rtos_task_sleep(100);}
        slave_rdy = 0;
        luat_gpio_set(HAL_GPIO_8, 0);
        for (size_t i = 0; i < 5; i++)
        {
            if (slave_rdy)
            {
                LUAT_DEBUG_PRINT("slave_rdy: %d !!!", slave_rdy);
                break;
            }
            luat_rtos_task_sleep(1);
        }
        LUAT_DEBUG_PRINT("!!slave_rdy: %d", luat_gpio_get(TEST_RDY_PIN));
        slave_rdy = 0;
        luat_spi_transfer(TEST_SPI_ID, send_buf, TEST_BUFF_SIZE, recv_buf, TEST_BUFF_SIZE);
        // luat_rtos_task_sleep(1);
        luat_gpio_set(HAL_GPIO_8, 1);
        // print_buff("TX", send_buf, TEST_BUFF_SIZE);
        // }
        memset(recv_buf, 0xff, TEST_BUFF_SIZE);
        luat_rtos_task_sleep(300);
        start = 0;
    }
}

static  void task_gpio_why(void *param) {
    luat_event_t event;
    while (1) {
        luat_rtos_event_recv(gpio_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
        LUAT_DEBUG_PRINT("GPIO IRQ %d", event.id);
    }
}

#if 0
static void task_demo_spi(void)
{
    luat_rtos_task_create(&spi_task_handle, 4 * 1024, 20, "spi", task_test_spi, NULL, 0);
    luat_rtos_task_create(&gpio_task_handle, 4 * 1024, 20, "gpio", task_gpio_why, NULL, 128);
}

INIT_TASK_EXPORT(task_demo_spi,"1");
#endif
