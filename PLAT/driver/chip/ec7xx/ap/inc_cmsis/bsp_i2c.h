#ifndef  BSP_I2C_H
#define  BSP_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "ec7xx.h"
#include "bsp.h"


// I2C Driver state flags
#define I2C_FLAG_INIT        BIT(0)        // Driver initialized
#define I2C_FLAG_POWER       BIT(1)        // Driver power on
#define I2C_FLAG_SETUP       BIT(2)        // Master configured, clock set

// I2C IRQ
typedef const struct _I2C_IRQ {
  IRQn_Type             irq_num;         // I2C IRQ Number
  IRQ_Callback_t        cb_irq;
} I2C_IRQ;

// I2C PINS
typedef const struct _I2C_PIN {
  PIN               *pin_scl;                                //  SCL Pin identifier
  PIN               *pin_sda;                                //  SDA Pin identifier
} I2C_PINS;

typedef enum __I2C_STATE
{
    I2C_STATE_IDLE,
    I2C_STATE_TXDATA,
    I2C_STATE_RXDATA,
    I2C_STATE_END
} I2C_STATE;

// I2C Control Information
typedef struct {
  ARM_I2C_SignalEvent_t cb_event;           // Event callback
  ARM_I2C_STATUS        status;             // Status flags
  uint32_t              address;            // Address
  uint8_t               flags;              // Control and state flags
  bool                  pending;            // Transfer pending (no STOP)
  uint8_t               stalled;            // Stall mode status flags
  I2C_STATE             state;              // I2C FSM current state
  uint32_t              cnt;                // transfer count
  uint8_t              *data;               // data to transfer
  uint32_t              num;                // Number of bytes to transfer
  uint32_t              cmd_cnt;            // Cmd count
} I2C_CTRL;


// I2C Resources definition
typedef struct {
  I2C_TypeDef                 *reg;                  // I2C peripheral register interface
  I2C_PINS                     pins;                 // I2C PINS config
  I2C_IRQ*                     irq;                  // I2C IRQ
  I2C_CTRL                    *ctrl;                 // Run-Time control information
} const I2C_RESOURCES;

#ifdef __cplusplus
}
#endif

#endif /* BSP_I2C_H */
