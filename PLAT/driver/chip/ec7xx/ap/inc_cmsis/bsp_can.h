#ifndef  BSP_CAN_H
#define  BSP_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "ec7xx.h"
#include "Driver_CAN.h"
#include "bsp.h"


// CAN Driver state flags
#define CAN_FLAG_INIT                           BIT(0)        // Driver initialized
#define CAN_FLAG_POWER                          BIT(1)        // Driver power on
#define CAN_FLAG_SETUP                          BIT(2)        // Master configured, clock set


#define CAN_MODE_ONESHOT                        BIT(0)
#define CAN_MODE_ABORT_RETRANS_ON_NACK          BIT(1)

// CAN IRQ
typedef const struct _CAN_IRQ {
  IRQn_Type             irq_num;           // CAN IRQ Number
  IRQ_Callback_t        cb_irq;
} CAN_IRQ;

// CAN PINS
typedef const struct _CAN_PIN {
  const PIN               *pin_tx;
  const PIN               *pin_rx;
  const PIN               *pin_stb;
} CAN_PINS;

typedef struct
{
    uint32_t busError;                     // bus error count
    uint32_t arbLost;                      // arb lost count
    uint8_t  arbLostLoc;                   // location of arb lost
    uint8_t  ecc;                          // last bus error code captured
    uint8_t  txErrorCount;                 // tec
    uint8_t  rxErrorCount;                 // rec

    uint32_t errorWarning;                 // error warning count
    uint32_t errorPassive;                 // error passive count
    uint32_t busOff;                       // bus off count
} CAN_STATISTICS;

typedef enum __CAN_STATE
{
    CAN_STATE_ERROR_ACTIVE = 0,
    CAN_STATE_ERROR_WARNING = 1,
    CAN_STATE_ERROR_PASSIVE = 2,
    CAN_STATE_BUS_OFF = 3,
    CAN_STATE_STOPPED = 4,
    CAN_STATE_SLEEPING = 5,
    CAN_STATE_MAX
} CAN_STATE;

// CAN Control Information
typedef struct {
  ARM_CAN_SignalUnitEvent_t   unit_event_cb;        // Event callback
  ARM_CAN_SignalObjectEvent_t object_event_cb;      // Event Callback
  CAN_STATISTICS              stat;                 // Statistics
  CAN_STATE                   state;                // Current state
  uint8_t                     lastErrorCode;        // Last error code captured
  uint8_t                     flags;                // Control and state flags
  uint8_t                     mode;                 // bitmap for controller mode
  uint32_t                    pending;              // Used for sync issue
} CAN_CTRL;


// CAN Resources definition
typedef struct {
  CAN_TypeDef                 *reg;                  // CAN peripheral register interface
  CAN_PINS                     pins;                 // CAN PINS config
  CAN_IRQ*                     irq;                  // CAN IRQ
  CAN_CTRL                    *ctrl;                 // Run-Time control information
} const CAN_RESOURCES;

#ifdef __cplusplus
}
#endif

#endif /* BSP_CAN_H */
