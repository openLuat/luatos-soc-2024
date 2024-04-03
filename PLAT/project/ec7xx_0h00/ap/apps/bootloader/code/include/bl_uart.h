#ifndef BL_UART_H
#define BL_UART_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct uartFrameFormat
{
    uint32_t dataBits    : 8;   // valid value: 8 or 7
    uint32_t parity      : 8;   // valid value: 0(NONE), 1(ODD), 2(EVEN)
    uint32_t stopBits    : 8;   // valid value: 1 or 2
    uint32_t flowControl : 8;   // valid value: 0(NONE), 1(rts), 2(cts), 3(rts & cts)
} uartPortFrameFormat_t;

/**
  \fn          void UART_init(uint32_t instance, uartPortFrameFormat_t* format, uint32_t baudrate)
  \brief       Initialize UART with specific baudrate
  \param[in]   instance         UART instance number (0, 1, ...)
  \param[in]   format           Pointer to UART frame format configuration
  \param[in]   baudrate         The desired baudrate
 */
void UART_init(uint32_t instance, uartPortFrameFormat_t* format, uint32_t baudrate);

/**
  \fn          void UART_Deinit(uint32_t instance)
  \brief       De-initialize UART
  \param[in]   instance         UART instance number (0, 1, ...)
 */
void UART_Deinit(uint32_t instance);

/**
  \fn          uint32_t UART_send(uint32_t instance, const uint8_t *data, uint32_t num, uint32_t timeout_us)
  \brief       Start sending data to USART transmitter in polling way
  \param[in]   instance    UART instance number (0, 1, ...)
  \param[in]   data        Pointer to buffer with data to send to USART transmitter
  \param[in]   num         Number of data items to send
  \param[in]   timeout_us  timeout value in unit of us
  \return      num of data items sent in the internal of timeout
 */
uint32_t UART_send(uint32_t instance, const uint8_t *data, uint32_t num, uint32_t timeout_us);

/**
  \fn          uint32_t UART_receive(uint32_t instance, uint8_t *data, uint32_t num, uint32_t timeout_us)
  \brief       Start receiving data from USART receiver in polling way
  \param[in]   instance    UART instance number (0, 1, ...)
  \param[out]  data        Pointer to buffer for data to receive from USART receiver
  \param[in]   num         Number of data items to receive
  \param[in]   timeout_us  timeout value in unit of us
  \return      num of data items received in the internal of timeout
 */
uint32_t UART_receive(uint32_t instance, uint8_t *data, uint32_t num, uint32_t timeout_us);

/** \} */

#ifdef __cplusplus
}
#endif

#endif /* BL_UART_H */
