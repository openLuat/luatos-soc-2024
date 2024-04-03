#ifndef __SPI_LCD_COMMON_H__
#define __SPI_LCD_COMMON_H__
#include "common_api.h"

enum
{
	SPI_LCD_DRIVER_GC9306X,
	SPI_LCD_DRIVER_QTY,
};

typedef struct
{
	uint16_t w_max;
	uint16_t h_max;
	uint8_t spi_id;
	uint8_t cs_pin;
	uint8_t dc_pin;
	uint8_t rst_pin;
	uint8_t blk_pin;
	uint8_t direction;
    uint8_t x_offset;//偏移
    uint8_t y_offset;//偏移
}spi_lcd_ctrl_t;


typedef struct
{
	union
	{
		uint8_t *queue;	//data超过4个字节放地址
		uint8_t buf[4];	//不超过4个字节的，直接放值
	};
	uint16_t reboot_ms;
	uint8_t len;
	uint8_t cmd;
}spi_lcd_cmd_t;


#define SPI_LCD_CMD_READ_ID	(0x04)
#define SPI_LCD_CMD_READ_STATUS	(0x09)
#define SPI_LCD_CMD_SLEEP_MODE	(0x10)
#define SPI_LCD_CMD_WAKEUP_MODE	(0x11)
#define SPI_LCD_CMD_NORMAL_DISPLAY	(0x13)
#define SPI_LCD_CMD_INVERSION_OFF	(0x20)
#define SPI_LCD_CMD_INVERSION_ON	(0x21)
#define SPI_LCD_CMD_DISPLAY_OFF	(0x28)
#define SPI_LCD_CMD_DISPLAY_ON	(0x29)
#define SPI_LCD_CMD_COLUMN_ADDRESS_SET	(0x2a)
#define SPI_LCD_CMD_PAGE_ADDRESS_SET	(0x2b)
#define SPI_LCD_CMD_MEMORY_WRITE	(0x2c)
#define SPI_LCD_CMD_MEMORY_ACCESS_CONTROL	(0x36)

#define SPI_LCD_CMD_READ_ID4	(0xd3)
#define SPI_LCD_CMD_READ_ID1	(0xda)
#define SPI_LCD_CMD_READ_ID2	(0xdb)
#define SPI_LCD_CMD_READ_ID3	(0xdc)
int spi_lcd_detect(uint8_t sda_pin, uint8_t scl_pin, uint8_t cs_pin, uint8_t dc_pin, uint8_t rst_pin);
void spi_lcd_write(spi_lcd_ctrl_t *spi_lcd, uint8_t cmd, uint8_t *data, uint8_t len);
void spi_lcd_init_auto(spi_lcd_ctrl_t *spi_lcd, int driver_type);

void spi_lcd_full_blank(spi_lcd_ctrl_t *spi_lcd);
void spi_lcd_driver_gc9306x_init(spi_lcd_ctrl_t *spi_lcd);
#endif
