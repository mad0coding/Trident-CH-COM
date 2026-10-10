/********************************** (C) COPYRIGHT *******************************
 * File Name          : central.h
 * Description        : Wireless UART central application.
 *********************************************************************************/

#ifndef CENTRAL_H
#define CENTRAL_H

#ifdef __cplusplus
extern "C" {
#endif

extern void Central_Init(void);
extern uint16_t Central_ProcessEvent(uint8_t task_id, uint16_t events);

#ifdef __cplusplus
}
#endif

#endif
