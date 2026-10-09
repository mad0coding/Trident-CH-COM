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

#ifdef __cplusplus
}
#endif

#endif /* APP_UART_H */
