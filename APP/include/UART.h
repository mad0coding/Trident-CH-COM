/********************************** (C) COPYRIGHT *******************************
 * File Name          : UART.h
 * Description        : UART application helpers
 *******************************************************************************/

#ifndef APP_UART_H
#define APP_UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void UART1_Init(void);
void UART_SetBaudRate(uint32_t baudrate);
void UART_SetLineCoding(uint8_t format);
uint32_t UART_GetBaudRate(void);
uint8_t UART_GetLineCoding(void);

/*
 * Software modem-signal state for UART1.  A value of 1 represents a
 * physical high level.  DTR/RTS are outputs controlled by the USB host;
 * CTS/DSR/RI/DCD are inputs which application code can later update from
 * whichever GPIOs are assigned to them.
 */
extern volatile uint8_t UART_FlowDTR;
extern volatile uint8_t UART_FlowRTS;
extern volatile uint8_t UART_FlowCTS;
extern volatile uint8_t UART_FlowDSR;
extern volatile uint8_t UART_FlowRI;
extern volatile uint8_t UART_FlowDCD;

#define UART_FLOW_INPUT_CTS    0x01
#define UART_FLOW_INPUT_DSR    0x02
#define UART_FLOW_INPUT_RI     0x04
#define UART_FLOW_INPUT_DCD    0x08
#define UART_FLOW_INPUT_MASK   (UART_FLOW_INPUT_CTS | UART_FLOW_INPUT_DSR | \
                                UART_FLOW_INPUT_RI  | UART_FLOW_INPUT_DCD)

void UART_SetFlowOutputs(uint8_t dtr, uint8_t rts);
void UART_SetFlowInputs(uint8_t mask, uint8_t value);
uint8_t UART_GetFlowStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_UART_H */
