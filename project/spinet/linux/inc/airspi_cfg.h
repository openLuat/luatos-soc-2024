#ifndef __AIRSPI_CFG_H__
#define __AIRSPI_CFG_H__


#define SPINET_CFG_SPI_DEVICE "/dev/spidev0.0"
#define SPINET_CFG_TUN_DEVICE "/dev/net/tun"
#define SPINET_CFG_SPI_CS_PIN 0
#define SPINET_CFG_SPI_IRQ_PIN 0
#define SPINET_CFG_SPI_SPEED 10000000

#define SPINET_CFG_BUFF_SIZE 4096

#define SPINET_LOG printf
#define SPINET_LOG_END "\n"

#endif
