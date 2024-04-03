#ifndef __RTE_DEVICE_H
#define __RTE_DEVICE_H

/*  Peripheral IO Mode Select, Must Configure First !!!
    Note, when receiver works in DMA_MODE, interrupt is also enabled to transfer tailing bytes.
*/

#define POLLING_MODE            0x1
#define DMA_MODE                0x2
#define IRQ_MODE                0x3
#define UNILOG_MODE             0x4

#define RTE_UART0_TX_IO_MODE    POLLING_MODE
#define RTE_UART0_RX_IO_MODE    POLLING_MODE

#define RTE_UART1_TX_IO_MODE    POLLING_MODE
#define RTE_UART1_RX_IO_MODE    POLLING_MODE


#define RTE_SPI0_IO_MODE          DMA_MODE

#define RTE_SPI1_IO_MODE          DMA_MODE

#define I2C0_INIT_MODE          POLLING_MODE
#define I2C1_INIT_MODE          POLLING_MODE


// I2C0 (Inter-integrated Circuit Interface) [Driver_I2C0]
// Configuration settings for Driver_I2C0 in component ::Drivers:I2C
#define RTE_I2C0                        0

// { PAD_PIN31},  // 0 : gpio16  / 1 : UART1 RTS / 2: I2C0_SDA
// { PAD_PIN32},  // 0 : gpio17  / 1 : UART1 CTS / 2: I2C0_SCL
#define RTE_I2C0_SCL_BIT                32
#define RTE_I2C0_SCL_FUNC               PAD_MUX_ALT2

#define RTE_I2C0_SDA_BIT                31
#define RTE_I2C0_SDA_FUNC               PAD_MUX_ALT2

// DMA
//   Tx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_I2C0_DMA_TX_EN              0
#define RTE_I2C0_DMA_TX_REQID           DMA_REQUEST_I2C0_TX
//   Rx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_I2C0_DMA_RX_EN              0
#define RTE_I2C0_DMA_RX_REQID           DMA_REQUEST_I2C0_RX

// I2C1 (Inter-integrated Circuit Interface) [Driver_I2C1]
// Configuration settings for Driver_I2C1 in component ::Drivers:I2C
#define RTE_I2C1                        0

// { PAD_PIN20},  // 0 : gpio5  / 2 : I2C1 SCL
// { PAD_PIN19},  // 0 : gpio4  / 2 : I2C1 SDA
#define RTE_I2C1_SCL_BIT                20
#define RTE_I2C1_SCL_FUNC               PAD_MUX_ALT2

#define RTE_I2C1_SDA_BIT                19
#define RTE_I2C1_SDA_FUNC               PAD_MUX_ALT2

// DMA
//   Tx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_I2C1_DMA_TX_EN              1
#define RTE_I2C1_DMA_TX_REQID           DMA_REQUEST_I2C1_TX
//   Rx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_I2C1_DMA_RX_EN              1
#define RTE_I2C1_DMA_RX_REQID           DMA_REQUEST_I2C1_RX


// UART0 (Universal asynchronous receiver transmitter) [Driver_USART0]
// Configuration settings for Driver_USART0 in component ::Drivers:USART
#define RTE_UART0                       1
#define RTE_UART0_CTS_PIN_EN            0
#define RTE_UART0_RTS_PIN_EN            0

#if defined CHIP_EC718
// { PAD_PIN42},  // 0 : gpio36 / 3 : UART0 RTSn
// { PAD_PIN43},  // 0 : gpio37 / 3 : UART0 CTSn
// { PAD_PIN31},  // 0 : gpio16 / 1 : UART0 RXD
// { PAD_PIN32},  // 0 : gpio17 / 1 : UART0 TXD
#define RTE_UART0_RTS_BIT               42
#define RTE_UART0_RTS_FUNC              PAD_MUX_ALT3

#define RTE_UART0_CTS_BIT               43
#define RTE_UART0_CTS_FUNC              PAD_MUX_ALT3

#define RTE_UART0_RX_BIT                31
#define RTE_UART0_RX_FUNC               PAD_MUX_ALT1

#define RTE_UART0_TX_BIT                32
#define RTE_UART0_TX_FUNC               PAD_MUX_ALT1
#elif defined CHIP_EC716

// { PAD_PIN14},  // 0 : gpio2 / 5 : UART0 RTSn
// { PAD_PIN15},  // 0 : gpio3 / 5 : UART0 CTSn
// { PAD_PIN18},  // 0 : gpio6 / 1 : UART0 RXD
// { PAD_PIN19},  // 0 : gpio7 / 1 : UART0 TXD
#define RTE_UART0_RTS_BIT               14
#define RTE_UART0_RTS_FUNC              PAD_MUX_ALT5

#define RTE_UART0_CTS_BIT               15
#define RTE_UART0_CTS_FUNC              PAD_MUX_ALT5

#define RTE_UART0_RX_BIT                18
#define RTE_UART0_RX_FUNC               PAD_MUX_ALT1

#define RTE_UART0_TX_BIT                19
#define RTE_UART0_TX_FUNC               PAD_MUX_ALT1

#endif
// DMA
//  Tx
//    Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_UART0_DMA_TX_REQID          DMA_REQUEST_USART0_TX
//  Rx
//    Channel    <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_UART0_DMA_RX_REQID          DMA_REQUEST_USART0_RX

// UART1 (Universal asynchronous receiver transmitter) [Driver_USART1]
// Configuration settings for Driver_USART1 in component ::Drivers:USART
#define RTE_UART1                       1
#define RTE_UART1_CTS_PIN_EN            0
#define RTE_UART1_RTS_PIN_EN            0

#if defined CHIP_EC718
// { PAD_PIN27},  // 0 : gpio12  / 2 : UART1 RTS
// { PAD_PIN28},  // 0 : gpio13  / 2 : UART1 CTS
// { PAD_PIN33},  // 0 : gpio18  / 1 : UART1 RXD
// { PAD_PIN34},  // 0 : gpio19  / 1 : UART1 TXD
#define RTE_UART1_RTS_BIT               27
#define RTE_UART1_RTS_FUNC              PAD_MUX_ALT2

#define RTE_UART1_CTS_BIT               28
#define RTE_UART1_CTS_FUNC              PAD_MUX_ALT2

#define RTE_UART1_RX_BIT                33
#define RTE_UART1_RX_FUNC               PAD_MUX_ALT1

#define RTE_UART1_TX_BIT                34
#define RTE_UART1_TX_FUNC               PAD_MUX_ALT1
#elif defined CHIP_EC716
// { PAD_PIN16},  // 0 : gpio4  / 5 : UART1 RTS
// { PAD_PIN17},  // 0 : gpio5  / 5 : UART1 CTS
// { PAD_PIN20},  // 0 : gpio8  / 1 : UART1 RXD
// { PAD_PIN21},  // 0 : gpio9  / 1 : UART1 TXD
#define RTE_UART1_RTS_BIT               16
#define RTE_UART1_RTS_FUNC              PAD_MUX_ALT5

#define RTE_UART1_CTS_BIT               17
#define RTE_UART1_CTS_FUNC              PAD_MUX_ALT5

#define RTE_UART1_RX_BIT                20
#define RTE_UART1_RX_FUNC               PAD_MUX_ALT1

#define RTE_UART1_TX_BIT                21
#define RTE_UART1_TX_FUNC               PAD_MUX_ALT1
#endif

// DMA
//   Tx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_UART1_DMA_TX_REQID          DMA_REQUEST_USART1_TX
//   Rx
//     Channel    <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_UART1_DMA_RX_REQID          DMA_REQUEST_USART1_RX

// UART2 (Universal asynchronous receiver transmitter) [Driver_USART2]
// Configuration settings for Driver_USART2 in component ::Drivers:USART
#define RTE_UART2                       0

// { PAD_PIN25},  // 0 : gpio10  / 3 : UART2 RXD
// { PAD_PIN26},  // 0 : gpio11  / 3 : UART2 TXD
#define RTE_UART2_RX_BIT                25
#define RTE_UART2_RX_FUNC               PAD_MUX_ALT3

#define RTE_UART2_TX_BIT                26
#define RTE_UART2_TX_FUNC               PAD_MUX_ALT3


// DMA
//   Tx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_UART2_DMA_TX_REQID          DMA_REQUEST_USART2_TX
//   Rx
//     Channel    <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_UART2_DMA_RX_REQID          DMA_REQUEST_USART2_RX

// SPI0 (Serial Peripheral Interface) [Driver_SPI0]
// Configuration settings for Driver_SPI0 in component ::Drivers:SPI
#define RTE_SPI0                        0

// { PAD_PIN23},  // 0 : gpio8  / 1 : SPI0 SSn
// { PAD_PIN24},  // 0 : gpio9  / 1 : SPI0 MOSI
// { PAD_PIN25},  // 0 : gpio10 / 1 : SPI0 MISO
// { PAD_PIN26},  // 0 : gpio11 / 1 : SPI0 SCLK
#define RTE_SPI0_SSN_BIT                23
#define RTE_SPI0_SSN_FUNC               PAD_MUX_ALT1

#define RTE_SPI0_MOSI_BIT               24
#define RTE_SPI0_MOSI_FUNC              PAD_MUX_ALT1

#define RTE_SPI0_MISO_BIT               25
#define RTE_SPI0_MISO_FUNC              PAD_MUX_ALT1

#define RTE_SPI0_SCLK_BIT               26
#define RTE_SPI0_SCLK_FUNC              PAD_MUX_ALT1

#define RTE_SPI0_SSN_GPIO_INSTANCE      0
#define RTE_SPI0_SSN_GPIO_INDEX         8

// DMA
//   Tx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_SPI0_DMA_TX_REQID           DMA_REQUEST_SPI0_TX

//   Rx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_SPI0_DMA_RX_REQID           DMA_REQUEST_SPI0_RX

// SPI1 (Serial Peripheral Interface) [Driver_SPI1]
// Configuration settings for Driver_SPI1 in component ::Drivers:SPI
#define RTE_SPI1                        0

// { PAD_PIN27},  // 0 : gpio12  / 1 : SPI1 SSn
// { PAD_PIN28},  // 0 : gpio13  / 1 : SPI1 MOSI
// { PAD_PIN29},  // 0 : gpio14  / 1 : SPI1 MISO
// { PAD_PIN30},  // 0 : gpio15  / 1 : SPI1 SCLK
// { PAD_PIN31},  // 0 : gpio16  / 4 : SPI1 SSn1
#define RTE_SPI1_SSN_BIT                27
#define RTE_SPI1_SSN_FUNC               PAD_MUX_ALT1

#define RTE_SPI1_MOSI_BIT               28
#define RTE_SPI1_MOSI_FUNC              PAD_MUX_ALT1

#define RTE_SPI1_MISO_BIT               29
#define RTE_SPI1_MISO_FUNC              PAD_MUX_ALT1

#define RTE_SPI1_SCLK_BIT               30
#define RTE_SPI1_SCLK_FUNC              PAD_MUX_ALT1

#define RTE_SPI1_SSN_GPIO_INSTANCE      0
#define RTE_SPI1_SSN_GPIO_INDEX         12

#define RTE_SPI1_SSN1_BIT               31
#define RTE_SPI1_SSN1_FUNC              PAD_MUX_ALT4

// DMA
//   Tx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_SPI1_DMA_TX_REQID           DMA_REQUEST_SPI1_TX

//   Rx
//     Channel     <0=>0 <1=>1 <2=>2 <3=>3 <4=>4 <5=>5 <6=>6 <7=>7
#define RTE_SPI1_DMA_RX_REQID           DMA_REQUEST_SPI1_RX


// PWM0 Controller [Driver_PWM0]
// Configuration settings for Driver_PWM0 in component ::Drivers:PWM
#define RTE_PWM                         1

#define EFUSE_INIT_MODE POLLING_MODE
#define L2CTLS_INIT_MODE POLLING_MODE

#define FLASH_BARE_RW_MODE 1

//bootloader do not use log in ic
#define IC_PHY_LOG_DISABLE 1

#ifdef FEATURE_FOTA_USBURC_ENABLE
#define RTE_USB_EN 1
#endif

#endif  /* __RTE_DEVICE_H */
