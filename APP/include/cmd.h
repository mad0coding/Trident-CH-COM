/********************************** (C) COPYRIGHT *******************************
 * File Name          : cmd.h
 * Description        : Transport-independent command-line interface
 *******************************************************************************/

#ifndef APP_CMD_H
#define APP_CMD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Enable or disable command mode.  Disabling also discards a partial line. */
void Cmd_SetMode(uint8_t enable);

/* Returns non-zero while command mode owns the selected input transport. */
uint8_t Cmd_IsActive(void);

/* Initialize and feed the boot-time DTR/RTS command-entry sequence. */
void Cmd_FlowSequenceInit(void);
void Cmd_SetBleConnected(uint8_t connected);
void Cmd_CheckFlowSequence(uint8_t dtr, uint8_t rts);
void Cmd_NotifyUsbReady(void);

/* Feed received command data one byte at a time (for example from UART). */
void Cmd_InputByte(uint8_t byte);

/* Feed a received buffer.  It is intentionally equivalent to repeatedly
 * calling Cmd_InputByte(), so a buffer need not contain a whole command. */
void Cmd_Input(const uint8_t *data, uint16_t length);

/*
 * Command response output.  Command mode currently routes it to USB.
 */
void Cmd_Output(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* APP_CMD_H */
