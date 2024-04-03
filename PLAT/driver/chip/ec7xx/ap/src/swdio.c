#include <stdint.h>
#include "swdio.h"
#include "gpio.h"
#include "pad.h"
#include "core_cm3.h"

#define SWD_DEBUG_MODE 0

#if SWD_DEBUG_MODE
#define SWIO_PAD_ADDR   21
#define SWIO_PORT_NUM   0
#define SWIO_PIN_NUM    6

#define SWCK_PAD_ADDR   22
#define SWCK_PORT_NUM   0
#define SWCK_PIN_NUM    7
#else
#if (defined CHIP_EC718)

#define SWIO_PAD_ADDR   0
#define SWIO_PORT_NUM   2
#define SWIO_PIN_NUM    8

#define SWCK_PAD_ADDR   0
#define SWCK_PORT_NUM   2
#define SWCK_PIN_NUM    9
#elif (defined CHIP_EC716)
#define SWIO_PAD_ADDR   0
#define SWIO_PORT_NUM   1
#define SWIO_PIN_NUM    5

#define SWCK_PAD_ADDR   0
#define SWCK_PORT_NUM   1
#define SWCK_PIN_NUM    6
#else

#endif


#endif

// Command Status Response Codes
#define HOST_COMMAND_OK         0x55
#define HOST_INVALID_COMMAND    0x80
#define HOST_COMMAND_FAILED     0x81
#define HOST_AP_TIMEOUT         0x82
#define HOST_WIRE_ERROR         0x83
#define HOST_ACK_FAULT          0x84
#define HOST_DP_NOT_CONNECTED   0x85

// Property SRST values
#define SRST_ASSERTED           0x1
#define SRST_DEASSERTED         0x0

#define DAP_RETRY_COUNT         255

//-----------------------------------------------------------------------------
// ARM Debug Interface Constants
//-----------------------------------------------------------------------------

// ARM CoreSight SWD-DP packet request values
#define SW_IDCODE_RD            0xA5
#define SW_ABORT_WR             0x81
#define SW_CTRLSTAT_RD          0x8D
#define SW_CTRLSTAT_WR          0xA9
#define SW_RESEND_RD            0x95
#define SW_SELECT_WR            0xB1
#define SW_RDBUFF_RD            0xBD

// ARM CoreSight SW-DP packet request masks
#define SW_REQ_PARK_START       0x81
#define SW_REQ_PARITY           0x20
#define SW_REQ_A32              0x18
#define SW_REQ_RnW              0x04
#define SW_REQ_APnDP            0x02

// ARM CoreSight SW-DP packet acknowledge values
#define SW_ACK_OK               0x4
#define SW_ACK_WAIT             0x2
#define SW_ACK_FAULT            0x1
#define SW_ACK_PARITY_ERR       0x1


// ARM CoreSight DAP command values
#define DAP_IDCODE_RD           0x02
#define DAP_ABORT_WR            0x00
#define DAP_CTRLSTAT_RD         0x06
#define DAP_CTRLSTAT_WR         0x04
#define DAP_SELECT_WR           0x08
#define DAP_RDBUFF_RD           0x0E
#define AP_IDR_RD               0x0F
#define AP_CSW_WR               0x01
#define AP_TAR_WR               0x05
#define AP_DRW_WR               0x0D
#define AP_DRW_RD               0x0F

// ARM CoreSight DAP command masks
#define DAP_CMD_RnW             0x2
#define DAP_CMD_APnDP           0x1
#define DAP_CMD_MASK            0x0F


// MEMAP register addresses
#define MEMAP_CSW  0x01
#define MEMAP_TAR  0x05
#define MEMAP_DRW_WR  0x0D
#define MEMAP_DRW_RD  0x0F

// CHIPAP register addresses
#define CHIPAP_CTRL1_WR     0x01
#define CHIPAP_CTRL2_WR     0x05
#define CHIPAP_ID_WR        0x0D
#define CHIPAP_ID_RD        0x0F

typedef unsigned char STATUS;


void swdioSetOutput(void);
void swdioSetInput(void);
bool swdioGetInput(void);
void swdioSetHigh(void);
void swdioSetLow(void);
void swdckSetOutput(void);
void swdckSetHigh(void);
void swdckSetLow(void);
STATUS swdResponse(uint8_t SW_Ack);
void swdDAPRead(uint8_t cnt, uint8_t DAP_Addr, uint32_t * read_data);
void swdDAPWrite(uint8_t cnt, uint8_t DAP_Addr, uint32_t * write_data, bool final);
uint8_t swdRequest(uint8_t DAP_Addr);
bool    swdCalcDataParity(void);
uint8_t swdShiftPacket(uint8_t request, uint8_t retry);
void    swdShiftByteOut(uint8_t byte);
uint8_t swdShiftByteIn(void);
void    swdShiftReset(void);
extern void delay_us(uint32_t us);
extern void GPR_apSetCPSWDEnable(bool en);


#define SWDIO_SET_HIGH()        swdioSetHigh()
#define SWDIO_SET_LOW()         swdioSetLow()
#define SWDIO_SET_INPUT()       swdioSetInput()
#define SWDIO_SET_OUTPUT()      swdioSetOutput()
#define SWDIO_GET_VALUE()       swdioGetInput()
#define SWDCLK_SET_HIGH()       swdckSetHigh()
#define SWDCLK_SET_LOW()        swdckSetLow()
#define SWDCLK_SET_OUTPUT()     swdckSetOutput()
#define DELAY()                 delay_us(1)


//-----------------------------------------------------------------------------
// Variables Declarations
//-----------------------------------------------------------------------------
volatile uint32_t io_word;
volatile uint8_t io_byte;

// Even parity lookup table, holds even parity result for a 4-bit value.
const uint8_t even_parity[] =
{
    0x00, 0x10, 0x10, 0x00,
    0x10, 0x00, 0x00, 0x10,
    0x10, 0x00, 0x00, 0x10,
    0x00, 0x10, 0x10, 0x00
};

void swdioSetOutput(void)
{
    GpioPinConfig_t config;
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 0;

    GPIO_pinConfig(SWIO_PORT_NUM, SWIO_PIN_NUM, &config);
}

void swdioSetInput(void)
{
    GpioPinConfig_t config;
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.initOutput = 0;

    GPIO_pinConfig(SWIO_PORT_NUM, SWIO_PIN_NUM, &config);
}

bool swdioGetInput(void)
{
    return GPIO_pinRead(SWIO_PORT_NUM, SWIO_PIN_NUM);
}

void swdioSetHigh(void)
{
    GPIO_pinWrite(SWIO_PORT_NUM, 1<<SWIO_PIN_NUM, 1<<SWIO_PIN_NUM);
}

void swdioSetLow(void)
{
    GPIO_pinWrite(SWIO_PORT_NUM, 1<<SWIO_PIN_NUM, 0);
}


void swdckSetOutput(void)
{
    GpioPinConfig_t config;
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 0;

    GPIO_pinConfig(SWCK_PORT_NUM, SWCK_PIN_NUM, &config);
}

void swdckSetHigh(void)
{
    GPIO_pinWrite(SWCK_PORT_NUM, 1<<SWCK_PIN_NUM, 1<<SWCK_PIN_NUM);
}

void swdckSetLow(void)
{
    GPIO_pinWrite(SWCK_PORT_NUM, 1<<SWCK_PIN_NUM, 0);
}

void swdInit(void)
{
#if SWD_DEBUG_MODE
    PadConfig_t padConfig;
    PAD_getDefaultConfig(&padConfig);

    // gpio7 paddr=22
    padConfig.mux = PAD_MUX_ALT0;
    padConfig.pullSelect = PAD_PULL_INTERNAL;
    padConfig.pullUpEnable = PAD_PULL_UP_ENABLE;
    padConfig.pullDownEnable = PAD_PULL_DOWN_DISABLE;

    PAD_setPinConfig(SWIO_PAD_ADDR, &padConfig);
    PAD_setPinConfig(SWCK_PAD_ADDR, &padConfig);
#else
    GPR_apSetCPSWDEnable(true);
#endif
}


void swdDeinit(void)
{
#if SWD_DEBUG_MODE
#else
    GPR_apSetCPSWDEnable(false);
#endif
}


void swdSendByte(uint32_t data, uint8_t len)
{
    uint8_t i=0;
    SWDCLK_SET_OUTPUT();
    SWDIO_SET_OUTPUT();
    SWDCLK_SET_LOW();
    for(i=0; i<len; i++)
    {
        if(data & 0x1)
            SWDIO_SET_HIGH();
        else
            SWDIO_SET_LOW();

        data>>=1;
        DELAY();
        SWDCLK_SET_HIGH();
        DELAY();
        SWDCLK_SET_LOW();
    }
    DELAY();
}


void swdConnect(void)
{
    uint32_t idcode;
    // Select the Serial Wire Debug Port
    // Skip this switch sequence if the device does not have the swj_dp port
    // Serial Wire + JTAG
    swdShiftReset();
    swdShiftByteOut(0x9E);
    swdShiftByteOut(0xE7);

    // Reset the line and return the 32-bit ID code
    swdShiftReset();
    swdShiftByteOut(0);

    swdDAPRead(0, DAP_IDCODE_RD, &idcode);
}


STATUS swdClearErrors(void)
{
    uint8_t ack;

    // First read the DP-CSR register and send the value to the host.
    swdShiftPacket(SW_CTRLSTAT_RD, 1);

    // Clear all error/sticky bits by writing to the abort register.
    io_word = 0x1E;
    swdShiftPacket(SW_ABORT_WR, 1);

    // Read the DP-CSR register again and send the results to the host.
    ack = swdShiftPacket(SW_CTRLSTAT_RD, 1);
    swdShiftByteOut(0);

    return swdResponse(ack);
}


STATUS swdResponse(uint8_t SW_Ack)
{
    switch (SW_Ack)
    {
    case SW_ACK_OK:     return HOST_COMMAND_OK;
    case SW_ACK_WAIT:   return HOST_AP_TIMEOUT;
    case SW_ACK_FAULT:  return HOST_ACK_FAULT;
    default:            return HOST_WIRE_ERROR;
    }
}


void swdDAPRead(uint8_t cnt, uint8_t DAP_Addr, uint32_t * read_data)
{
    uint8_t req;

    // Format the packet request header
    req = swdRequest(DAP_Addr);

    // Shift the first packet and if DP access, send the results
    swdShiftPacket(req, 0);
    if (!(req & SW_REQ_APnDP))
    {
        *read_data = io_word;
        read_data++;
    }

    // Perform the requested number of reads
    for (; cnt != 0; cnt--)
    {
        swdShiftPacket(req, 0);
        *read_data = io_word;
        read_data++;
    }

    // For AP access, get and send results of the last read
    if (req & SW_REQ_APnDP)
    {

        swdShiftPacket(SW_RDBUFF_RD, 0);
        *read_data = io_word;
        read_data++;
    }

    delay_us(200);
}


void swdDAPWrite(uint8_t cnt, uint8_t DAP_Addr, uint32_t * write_data, bool final)
{
    uint8_t req;

    // Format the packet request header
    req = swdRequest(DAP_Addr);

    // Perform the requested number of writes
    do
    {
        io_word = *write_data;
        write_data++;

        swdShiftPacket(req, 0);
    }
    while (cnt-- != 0);

    delay_us(200);
}


uint8_t swdRequest(uint8_t DAP_Addr)
{
    uint8_t req = 0;

    // Convert the DAP address into a SWD packet request value
    req = DAP_Addr & DAP_CMD_MASK;      // mask off the bank select bits
    req = req | even_parity[req];       // compute and add parity bit
    req = req << 1;                     // move address/parity bits
    req = req | SW_REQ_PARK_START;      // add start and park bits
    return req;
}


bool swdCalcDataParity(void)
{
    uint8_t parity;

    // Calculate column parity, reducing down to 4 columns
    parity  = io_word & 0xFF;
    parity ^= (io_word>>8) & 0xFF;
    parity ^= (io_word>>16) & 0xFF;
    parity ^= (io_word>>24) & 0xFF;
    parity ^= parity >> 4;

    // Use lookup table to get parity on 4 remaining bits. The cast (bit)
    // converts any non-zero value to 1.
    return (bool)even_parity[parity & 0xF];
}


void swdShiftReset(void)
{
    uint8_t i;

    SWDCLK_SET_OUTPUT();

    // Drive SWDIO high
    SWDIO_SET_OUTPUT();
    SWDIO_SET_HIGH();

    // Complete 64 SWCLK cycles
    for (i = 64; i != 0; i--)
    {
        SWDCLK_SET_HIGH();
        SWDCLK_SET_LOW();
    }
}


uint8_t swdShiftPacket(uint8_t request, uint8_t retry)
{
    uint8_t ack, limit, i;
    bool iob_0;

    // If retry parameter is zero, use the default value instead
    if (retry == 0)
    {
        retry = DAP_RETRY_COUNT;
    }
    limit = retry;

    // While waiting, do request phase (8-bit request, turnaround, 3-bit ack)
    do
    {
        // Shift out the 8-bit packet request
        swdShiftByteOut(request);

        // Turnaround cycle makes SWDIO an input
        SWDIO_SET_INPUT();

        SWDCLK_SET_HIGH();

        DELAY();

        SWDCLK_SET_LOW();

        // Shift in the 3-bit acknowledge response
        io_byte = 0;
        for(uint8_t i=0; i<3; i++)
        {
            io_byte = io_byte<<1;

            io_byte |= SWDIO_GET_VALUE();

            SWDCLK_SET_HIGH();

            DELAY();

            SWDCLK_SET_LOW();
        }
        ack = io_byte;

        // Check if we need to retry the request
        if ((ack == SW_ACK_WAIT) && --retry)
        {
            // Delay an increasing amount with each retry
            for (i=retry; i < limit; i++);
        }
        else
        {
            break;  // Request phase complete (or timeout)
        }
    }
    while (TRUE);

    // If the request was accepted, do the data transfer phase (turnaround if
    // writing, 32-bit data, and parity)
    if (ack == SW_ACK_OK)
    {
        if (request & SW_REQ_RnW)
        {
            // Swap endian order while shifting in 32-bits of data
            io_word = 0;
            for(uint8_t i=0; i<4; i++)
            {
                io_word |= (swdShiftByteIn() << (i*8));
            }

            // Shift in the parity bit
            iob_0 = SWDIO_GET_VALUE();

            SWDCLK_SET_HIGH();

            DELAY();

            SWDCLK_SET_LOW();

            // Check for parity error
            if (iob_0 ^ swdCalcDataParity())
            {
                ack = SW_ACK_PARITY_ERR;
            }

        }
        else
        {
            // Turnaround cycle makes SWDIO an output
            SWDIO_SET_OUTPUT();

            SWDCLK_SET_HIGH();

            DELAY();

            SWDCLK_SET_LOW();

            // Swap endian order while shifting out 32-bits of data
            swdShiftByteOut((io_word>>0)&0xFF);
            swdShiftByteOut((io_word>>8)&0xFF);
            swdShiftByteOut((io_word>>16)&0xFF);
            swdShiftByteOut((io_word>>24)&0xFF);

            // Shift out the parity bit
            if(swdCalcDataParity())
                SWDIO_SET_HIGH();
            else
                SWDIO_SET_LOW();

            SWDCLK_SET_HIGH();

            DELAY();

            SWDCLK_SET_LOW();
        }
    }
    // TODO: Add error (FAULT, line, parity) handling here?  RESEND on parity error?

    // Turnaround or idle cycle, always leave SWDIO an output
    SWDIO_SET_OUTPUT(); SWDIO_SET_LOW();

    swdShiftByteOut(0);

    return ack;
}


void swdShiftByteOut(uint8_t byte)
{
    swdSendByte(byte, 8);
}

uint8_t swdShiftByteIn(void)
{
    uint8_t i;
    uint32_t tmp = 0;

    SWDIO_SET_INPUT();
    DELAY();
    for(i=0; i<8; i++)
    {
        if(SWDIO_GET_VALUE())
        {
            tmp |= (1<<i);
        }
        SWDCLK_SET_HIGH();
        DELAY();
        SWDCLK_SET_LOW();
        DELAY();
    }
    return tmp;
}


void swdConfigInit(void)
{
    uint32_t data;

    data = 0x0000001E;         // abort
    swdDAPWrite(0, DAP_ABORT_WR, &data, true);

    // Write the CTRLSTAT register to enable the debug hardware
    data = 0x50000000;
    swdDAPWrite(0, DAP_CTRLSTAT_WR, &data, true);

    data = 0;
    swdDAPWrite(0, DAP_SELECT_WR, &data, true);       // sel bank 0

    data = 0x23000002;
    swdDAPWrite(0, AP_CSW_WR, &data, true);   // csw = 0x23000002

}


void swdReadIDValue(uint32_t *idcode, uint32_t *idr)
{
    uint32_t data;

    swdDAPRead(0, DAP_IDCODE_RD, idcode);

    data = 0xF0;
    swdDAPWrite(0, DAP_SELECT_WR, &data, true);       // sel bank F

    swdDAPRead(0, DAP_IDCODE_RD, idr);

}

bool swdHalt(void)
{
    uint32_t data;

    data = 0;
    swdDAPWrite(0, DAP_SELECT_WR, &data, true);       // sel bank 0

    data = 0xE000EDF0;     //CoreDebug->DHCSR
    swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0xE000EDF0

    data = 0xA05F0001;              // debug_en = 1
    swdDAPWrite(0, AP_DRW_WR, &data, true);

    data = 0xE000EDF0;     //CoreDebug->DHCSR
    swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0xE000EDF0

    data = 0xA05F0003;              // halt
    swdDAPWrite(0, AP_DRW_WR, &data, true);

    data = 0xE000EDF0;     //CoreDebug->DHCSR
    swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0xE000EDF0

    data = 0;
    swdDAPRead(0, AP_DRW_RD, &data);

    if(data & 0x20000)
        return true;                //halt complete
    else
        return false;

}


void swdGetRegsInfo(swdCPRegs_t *cpDumpRegs)
{
    uint32_t data;
    uint8_t j;
    uint32_t *pRegs = (uint32_t *)cpDumpRegs;

    for(uint8_t i = 0; i<19; i++)
    {
        j = 5;

        data = 0xE000EDF4;     //CoreDebug->DCRSR
        swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0xE000EDF0

        data = i;     //CoreDebug->DCRSR  reg0-reg15 xpsr msp psp
        swdDAPWrite(0, AP_DRW_WR, &data, true);   // tar = 0x0

        data = 0xE000EDF0;     //CoreDebug->DHCSR
        swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0x0

        do
        {
            data = 0;     //CoreDebug->DHCSR
            swdDAPRead(0, AP_DRW_RD, &data);   // tar = 0xE000EDF0

            if(data&0x10000)    //s_regrdy = 1
                break;

        }while(j--);

        data = 0xE000EDF8;     //CoreDebug->DCRDR
        swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0x0

        swdDAPRead(0, AP_DRW_RD, (uint32_t *)(pRegs+i));   // tar = 0xE000EDF0

    }


    j = 5;
    data = 0xE000EDF4;     //CoreDebug->DCRSR
    swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0xE000EDF0

    data = 0x14;     //CoreDebug->DCRSR  con
    swdDAPWrite(0, AP_DRW_WR, &data, true);   // tar = 0x0

    data = 0xE000EDF0;     //CoreDebug->DHCSR
    swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0x0

    do
    {
        data = 0xE000EDF0;     //CoreDebug->DHCSR
        swdDAPRead(0, AP_DRW_RD, &data);   // tar = 0xE000EDF0

        if(data&0x10000)    //s_regrdy = 1
            break;

    }while(j--);

    data = 0xE000EDF8;     //CoreDebug->DCRDR
    swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0x0

    swdDAPRead(0, AP_DRW_RD, &data);   // tar = 0xE000EDF0

    cpDumpRegs->control = (data>>24)&0xff;
    cpDumpRegs->faultmask = (data>>16)&0xff;
    cpDumpRegs->basepri = (data>>8)&0xff;
    cpDumpRegs->primask = (data>>0)&0xff;

}



void swdStep(void)
{
    uint32_t data;

    data = 0xE000EDF0;     //CoreDebug->DHCSR
    swdDAPWrite(0, AP_TAR_WR, &data, true);   // tar = 0xE000EDF0

    data = 0xA05F0004;              // step
    swdDAPWrite(0, AP_DRW_WR, &data, true);

}


void swdCPCoreRegsDump(swdCPRegs_t *cpDumpRegs)
{
    uint32_t idcode, idr;

    swdInit();

    swdConnect();

    swdConfigInit();

    swdReadIDValue(&idcode, &idr);

    swdHalt();

    swdGetRegsInfo(cpDumpRegs);

    swdDeinit();

}


