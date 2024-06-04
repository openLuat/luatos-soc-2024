#include "luat_rtos.h"
#include "luat_gpio.h"
#include "luat_i2c.h"
#include "platform_define.h"

#define GSENSOR_INT_PIN             HAL_WAKEUP_0
#define GSENSOR_I2C_ID              I2C_ID0
#define GSENSOR_I2C_ADDR            0X26

#define REG_SPI_CONFIG              0x00
#define REG_CHIP_ID                 0x01
#define REG_ACC_X_LSB               0x02
#define REG_ACC_X_MSB               0x03
#define REG_ACC_Y_LSB               0x04
#define REG_ACC_Y_MSB               0x05
#define REG_ACC_Z_LSB               0x06
#define REG_ACC_Z_MSB               0x07
#define REG_FIFO_STATUS             0x08
#define REG_MOTION_FLAG1            0x09
#define REG_MOTION_FLAG2            0x0A
#define REG_TAP_ACTIVE_STATUS       0x0B
#define REG_ORIENT_STATUS           0x0C
#define REG_STEPS_MSB               0x0D
#define REG_STEPS_LSB               0x0E
#define REG_RESOLUTION_RANGE        0x0F
#define REG_MODE_ODR                0x10
#define REG_MODE_AXIS               0x11
#define REG_SWAP_POLARITY           0x12
#define REG_FIFO_CTRL               0x14
#define REG_INT_SET0                0x15
#define REG_INT_SET1                0x16
#define REG_INT_SET2                0x17
#define REG_INT_MAP1                0x19
#define REG_INT_MAP2                0x1A
#define REG_INT_MAP3                0x1B
#define REG_INT_MAP4                0x1C
#define REG_INT_CONFIG              0x20
#define REG_INT_LATCH               0x21
#define REG_FREEFALL_DUR            0x22
#define REG_FREEFALL_THS            0x23
#define REG_FREEFALL_HYST           0x24
#define REG_TAP_QUIET               0x29
#define REG_TAP_DUR                 0x2A
#define REG_TAP_THS                 0x2B
#define REG_ORIENT_HYST             0x2C
#define REG_Z_BLOCK                 0x2D
#define REG_RESET_STEP              0x2E
#define REG_STEP_FILTER             0x33
#define REG_ACTIVE_DUR              0x34
#define REG_ACTIVE_X_THS            0x38
#define REG_ACTIVE_Y_THS            0x39
#define REG_ACTIVE_Z_THS            0x3A
#define REG_SENS_COMP               0x8C



enum
{
    DA267_EVENT_INT = 1,
};

luat_rtos_task_handle gsensor_task_handle;
static int gsensor_interrupt_cb(int pin, void *arg)
{
    if (pin == GSENSOR_INT_PIN)
    {
        luat_rtos_event_send(gsensor_task_handle, 0, 0, 0, 0, 0);
    }
}

static int da267_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t temp[] = {reg, val};
    return luat_i2c_send(GSENSOR_I2C_ID, GSENSOR_I2C_ADDR, temp, 2, 1);
}
static int da267_read_reg(uint8_t reg, uint8_t *buff, uint8_t len)
{
    int result = 0;
    uint8_t temp[] = {reg};
    result = luat_i2c_send(GSENSOR_I2C_ID, GSENSOR_I2C_ADDR, temp, 1, 1);
    result |= luat_i2c_recv(GSENSOR_I2C_ID, GSENSOR_I2C_ADDR, buff, len);
    return result;
}

static int da267_check_id()
{
    uint8_t temp[1] = {REG_CHIP_ID};
    int result = luat_i2c_send(GSENSOR_I2C_ID, GSENSOR_I2C_ADDR, temp, 1, 1);
    if (result)
    {
        return result;
    }
    result = luat_i2c_recv(GSENSOR_I2C_ID, GSENSOR_I2C_ADDR, temp, 1);
    if (result || temp[0] != 0x13)
    {
        return result;
    }
    return 0;
}

int da267_init()
{
    luat_i2c_setup(GSENSOR_I2C_ID, 0);
    da267_write_reg(REG_SPI_CONFIG, 0x24);
    luat_rtos_task_sleep(20);
    da267_write_reg(REG_RESOLUTION_RANGE, 0x00);
    da267_write_reg(REG_MODE_AXIS, 0x34);
    da267_write_reg(REG_MODE_ODR, 0x06);
    luat_rtos_task_sleep(50);

    // Reduce power consumption
    if (GSENSOR_I2C_ADDR == 0x26)
        da267_write_reg(REG_SENS_COMP, 0x00);

    // init Active INTERRUPT
    da267_write_reg(REG_INT_SET1, 0x87);
    da267_write_reg(REG_ACTIVE_DUR, 0x00);
    da267_write_reg(REG_ACTIVE_X_THS, 0x05);
    da267_write_reg(REG_ACTIVE_Y_THS, 0x05);
    da267_write_reg(REG_ACTIVE_Z_THS, 0x05);
    da267_write_reg(REG_INT_MAP1, 0x04);

    // init step count
    da267_write_reg(REG_STEP_FILTER, 0x80);
    return 0;
}
int da267_read_step(uint16_t *step)
{
    uint16_t temp[2] = {0};
    int result = da267_read_reg(REG_STEPS_MSB, (uint8_t *)&temp, 2);
    if (result)
    {
        return result;
    }
    else
    {
        *step = ((temp[0] << 8 | temp[1]) / 2);
    }
    return 0;
}
void da267_run()
{
    if (da267_check_id())
    {
        return;
    }
    luat_event_t event = {0};
    while (1)
    {
        luat_rtos_event_recv(gsensor_task_handle, DA267_EVENT_INT, &event, NULL, 60 * 1000);
    }
    return;
}
int da267_close()
{
    luat_i2c_close(GSENSOR_I2C_ID);
    return 0;
}
void app_gsensor_run(void *params)
{
    while (1)
    {
        da267_init();
        da267_run();
        da267_close();
    }
}