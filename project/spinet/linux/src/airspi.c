

#include <fcntl.h>
#include "stdint.h"
#include "stdio.h"
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h> 
#include <poll.h>


#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/types.h>
//#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#include <linux/gpio.h>

#include "airspi.h"

int g_spi_fd = 0;
int g_tun_fd = 0;
uint8_t g_spi_rx[SPINET_CFG_BUFF_SIZE] = {0};
uint8_t g_spi_tx[SPINET_CFG_BUFF_SIZE] = {0};
uint8_t g_tun_rx[SPINET_CFG_BUFF_SIZE] = {0};
uint8_t g_tun_tx[SPINET_CFG_BUFF_SIZE] = {0};

static int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open(SPINET_CFG_TUN_DEVICE, O_RDWR)) < 0 ) {
       SPINET_LOG("open %s error\n", SPINET_CFG_TUN_DEVICE);
       return -1;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
       close(fd);
       SPINET_LOG("ioctl /dev/net/tun err=%d\n", err);
       return err;
    }
    strcpy(dev, ifr.ifr_name);
    g_tun_fd = fd;
    return 0;
}

#ifdef CONFIG_GPIOLIB
static int gpio_init(void) {
    int ret = 0;
    if (SPINET_CFG_SPI_CS_PIN > 0){
        if (!gpio_is_valid(SPINET_CFG_SPI_CS_PIN)) {
            SPINET_LOG("invalid spi cs pin %d\n", SPINET_CFG_SPI_CS_PIN);
            return -1;
        }
        gpio_request(SPINET_CFG_SPI_CS_PIN, "spinet_cs");
        ret = gpio_direction_output(SPINET_CFG_SPI_CS_PIN, 1);
        if (ret) {
            SPINET_LOG("set spi cs pin %d direction error\n", SPINET_CFG_SPI_CS_PIN);
            return -1;
        }
    }
    if (SPINET_CFG_SPI_IRQ_PIN > 0) {
        if (!gpio_is_valid(SPINET_CFG_SPI_IRQ_PIN)) {
            SPINET_LOG("invalid spi irq pin %d\n", SPINET_CFG_SPI_IRQ_PIN);
            return -1;
        }
        gpio_request(SPINET_CFG_SPI_IRQ_PIN, "spinet_irq");
        ret = gpio_direction_input(SPINET_CFG_SPI_IRQ_PIN);
        if (ret) {
            SPINET_LOG("set spi irq pin %d direction error\n", SPINET_CFG_SPI_IRQ_PIN);
            return -1;
        }
    }
    return 0;
}
#endif

static int spi_init(void) {
    g_spi_fd = open(SPINET_CFG_SPI_DEVICE, O_RDWR);
    if (!g_spi_fd) {
        SPINET_LOG("open %s error\n", SPINET_CFG_SPI_DEVICE);
        return -2;
    }
    return 0;
}

static int spinet_xfer2(int fd, uint8_t *tx, uint8_t *rx, int len)
{
    struct spi_ioc_transfer xfer[1] = {0};
    int ret = 0;

    xfer[0].tx_buf = (unsigned long)tx;
    xfer[0].rx_buf = (unsigned long)rx;
    xfer[0].len = len;
    xfer[0].cs_change = 1;
    xfer[0].speed_hz = SPINET_CFG_SPI_SPEED;
    xfer[0].bits_per_word = 8;
    xfer[0].delay_usecs = 0;

    ret = ioctl(g_spi_fd, SPI_IOC_MESSAGE(1), xfer);
    if (ret < 1) {
        SPINET_LOG("spi_xfer error %d\n", ret);
        return ret;
    }

    // 处理SPI读到的数据

    return 0;
}

static int main_loop(void) {
    // 首先, 初始化一个 tun 设备
    char devname[16] = {0};
    int retval = 0;
    struct pollfd  pfds[2] = {0};

    pfds[0].fd = g_tun_fd;
    pfds[0].events = POLLIN;

    while (1)
    {
        // 读取tun设备中的数据
        retval = poll(pfds, 1, 1000);
        if (retval < 0) {
            SPINET_LOG("poll tun fd error\n");
            break;
        }
        if (retval == 0) {
            continue;
        }

        if (pfds[0].revents & POLLIN) {
            int len = read(g_tun_fd, g_tun_tx, sizeof(g_tun_tx));
            if (len > 0) {
                SPINET_LOG("read tun fd %d bytes\n", len);
                // TODO 发送数据到SPI设备
            }
        }
        
        // 检查中断引脚的状态, 看看从机是否有数据下行
        #ifdef CONFIG_GPIOLIB
        if (SPI_IRQ_PIN == 0 || gpio_get_value(SPI_IRQ_PIN) == 1)
        #endif
        {
            // TODO 从SPI设备读取数据
            memset(g_spi_tx, 0, sizeof(g_spi_tx));
            spinet_xfer2(g_spi_fd, g_spi_tx, g_spi_rx, sizeof(g_spi_tx));
        }
        // usleep(100);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    // 首先, 初始化一个 tun 设备
    char devname[16] = {0};
    int             retval;
    fd_set          rfds;
    struct timeval  tv;
    
    retval = tun_alloc(devname);
    if (retval)
        return retval;

    // 然后, 打开spi设备
    retval = spi_init();
    if (retval)
        return retval;

    retval = main_loop();

    return retval;
}
