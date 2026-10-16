/********************************** (C) COPYRIGHT *******************************
 * File Name          : UART.c
 * Description        : UART application helpers
 *******************************************************************************/

#include "CONFIG.h"
#include "UART.h"
#include "app_usb.h"
#include "cmd.h"
#include "peripheral.h"

/*
 * These are deliberately not connected to CH592 GPIOs here.  Board-specific
 * code can map them to pins later without changing the USB protocol layer.
 * All modem signals use logical semantics: 1=Asserted, 0=Deasserted.
 * The input defaults are Deasserted; CH340 active-low status encoding then
 * produces the idle 0x9F, 0x88 reply.
 */
volatile uint8_t UART_FlowDTR = 0;
volatile uint8_t UART_FlowRTS = 0;
volatile uint8_t UART_FlowCTS = 0;
volatile uint8_t UART_FlowDSR = 0;
volatile uint8_t UART_FlowRI = 0;
volatile uint8_t UART_FlowDCD = 0;

#define UART_DEFAULT_BAUDRATE     115200UL
#define UART_DEFAULT_LINE_CODING  0xC3

static uint32_t UART_BaudRate = UART_DEFAULT_BAUDRATE;
static uint8_t UART_LineCoding = UART_DEFAULT_LINE_CODING;

/*********************************************************************
 * @fn      UART1_Init
 *
 * @brief   Reserved for application UART1 initialization.
 *
 * @return  none
 */
void UART1_Init(void)
{
	GPIOA_SetBits(bTXD1_);
	GPIOB_ModeCfg(bTXD1_, GPIO_ModeOut_PP_5mA);
	GPIOB_ModeCfg(bRXD1_, GPIO_ModeIN_PU);
	//UART1_DefInit();
	UART_SetBaudRate(UART_DEFAULT_BAUDRATE);
	R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开
	R8_UART1_LCR = RB_LCR_WORD_SZ; // 8bit数据
	R8_UART1_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY | RB_IER_THR_EMPTY; // 使能输出 使能接收中断 发送完成中断
	R8_UART1_DIV = 1;
	R16_PIN_ALTERNATE |= RB_PIN_UART1; // 引脚重映射
	UART1_ByteTrigCfg(UART_4BYTE_TRIG); // FIFO触发点4字节
	//trigB = 7;
	UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
	PFIC_EnableIRQ(UART1_IRQn);
}

/*********************************************************************
 * @fn      UART_SetBaudRate
 *
 * @brief   Apply a CH340 baudrate setting to UART1.
 *
 * @param   baudrate - CH340 baudrate
 *
 * @return  none
 */
void UART_SetBaudRate(uint32_t baudrate)
{
	if(baudrate == 0)
	{
		baudrate = UART_DEFAULT_BAUDRATE;
	}

	UART1_BaudRateCfg(baudrate);
	UART_BaudRate = baudrate;
}

uint32_t UART_GetBaudRate(void)
{
	return UART_BaudRate;
}

uint8_t UART_GetLineCoding(void)
{
	return UART_LineCoding;
}

/*********************************************************************
 * @fn      UART_SetFlowOutputs
 *
 * @brief   Convert CH340 active-low DTR#/RTS# request bits to logical
 *          modem states, then update DTR and RTS.
 *
 * @param   dtr - raw DTR# bit; zero means DTR is Asserted
 * @param   rts - raw RTS# bit; zero means RTS is Asserted
 *
 * @return  none
 */
void UART_SetFlowOutputs(uint8_t dtr, uint8_t rts)
{
	uint8_t newDtr = dtr ? 0U : 1U; // USB Driver's 0 = DTR# Low = DTR Asserted
	uint8_t newRts = rts ? 0U : 1U;

	if((newDtr != UART_FlowDTR) || (newRts != UART_FlowRTS))
	{
		UART_FlowDTR = newDtr;
		UART_FlowRTS = newRts;
		Cmd_CheckFlowSequence(newDtr, newRts);
	}
}

/*********************************************************************
 * @fn      UART_SetFlowInputs
 *
 * @brief   Set selected logical modem-input states and notify the USB host if the
 *          CH340-visible state changed.  Mask/value bits map as follows:
 *          bit0=CTS, bit1=DSR, bit2=RI, bit3=DCD.  A value bit of one
 *          represents an Asserted input.  GPIO mapping may invert an
 *          active-low electrical signal before calling this function.
 *
 * @param   mask  - inputs to change; zero bits leave their inputs unchanged
 * @param   value - new logical states for the selected inputs
 *
 * @return  none
 */
void UART_SetFlowInputs(uint8_t mask, uint8_t value)
{
	uint8_t oldStatus = UART_GetFlowStatus();

	mask &= UART_FLOW_INPUT_MASK;
	if(mask & UART_FLOW_INPUT_CTS) UART_FlowCTS = (value & UART_FLOW_INPUT_CTS) ? 1 : 0;
	if(mask & UART_FLOW_INPUT_DSR) UART_FlowDSR = (value & UART_FLOW_INPUT_DSR) ? 1 : 0;
	if(mask & UART_FLOW_INPUT_RI)  UART_FlowRI = (value & UART_FLOW_INPUT_RI) ? 1 : 0;
	if(mask & UART_FLOW_INPUT_DCD) UART_FlowDCD = (value & UART_FLOW_INPUT_DCD) ? 1 : 0;

	if(oldStatus != UART_GetFlowStatus())
	{
		USB_NotifyFlowStatus();
	}
}

/*********************************************************************
 * @fn      UART_GetFlowStatus
 *
 * @brief   Encode logical modem states in the active-low CH340 status byte.
 *          CTS/DSR/RI/DCD occupy bits 0..3 respectively; a zero status bit
 *          represents an Asserted signal.
 *
 * @return  CH340 modem-status byte
 */
uint8_t UART_GetFlowStatus(void)
{
	uint8_t status = 0x90;

	if(!UART_FlowRTS) status |= 0x40;
	if(!UART_FlowDTR) status |= 0x20;
	if(!UART_FlowCTS) status |= 0x01;
	if(!UART_FlowDSR) status |= 0x02;
	if(!UART_FlowRI)  status |= 0x04;
	if(!UART_FlowDCD) status |= 0x08;

	return status;
}

/*********************************************************************
 * @fn      UART_SetLineCoding
 *
 * @brief   Apply a CH340 line-format byte to UART1.  Unsupported
 *          formats fall back to the default 8 data bits, no parity,
 *          and 1 stop bit (8N1).
 *
 * @param   format - CH340 format byte from request 40 9A 18 25 xx 00
 *
 * @return  none
 */
void UART_SetLineCoding(uint8_t format)
{
	uint8_t lcr;
	uint8_t dataBits = format & 0x03;
	uint8_t parity = (format >> 3) & 0x07;

	/* CH340 reserves bits 7:6 as 1 and does not support 5 data bits
	with two stop bits (which would be UART 1.5-stop-bit mode). */
	if((format & 0xC0) != 0xC0 || (dataBits == 0 && (format & 0x04)))
	{
		R8_UART1_LCR = RB_LCR_WORD_SZ;
		UART_LineCoding = UART_DEFAULT_LINE_CODING;
		return;
	}

	lcr = dataBits;
	if(format & 0x04)
	{
		lcr |= RB_LCR_STOP_BIT;
	}

	switch(parity)
	{
		case 0: /* no parity */
			break;
		case 1: /* odd parity */
			lcr |= RB_LCR_PAR_EN;
			break;
		case 3: /* even parity */
			lcr |= RB_LCR_PAR_EN | (1 << 4);
			break;
		case 5: /* mark parity */
			lcr |= RB_LCR_PAR_EN | (2 << 4);
			break;
		case 7: /* space parity */
			lcr |= RB_LCR_PAR_EN | (3 << 4);
			break;
		default:
			R8_UART1_LCR = RB_LCR_WORD_SZ;
			UART_LineCoding = UART_DEFAULT_LINE_CODING;
			return;
	}

	R8_UART1_LCR = lcr;
	UART_LineCoding = format;
}

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1 interrupt
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
	uint8_t rxBuf[8];
	uint8_t len;

	switch(UART1_GetITFlag())
	{
		case UART_II_LINE_STAT: // 线路状态错误
			UART1_GetLinSTA();
			break;

		case UART_II_RECV_RDY: // 数据达到设置触发点
		case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
			LED_ON;
			len = UART1_RecvString(rxBuf);
			if(!RingBufRxUART) break;
			mrb_write(RingBufRxUART, rxBuf, len);
			//UART1_SendString(rxBuf, len); // 环回
			if(RingBufTxUSB) tmos_set_event(Peripheral_TaskID, SBP_USB_TX_EVT);
			else if(RingBufTxBLE) tmos_set_event(Peripheral_TaskID, SBP_BLE_TX_EVT);
			break;

		case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
			LED_ON;
			if(RingBufTxUART && !MRB_empty(RingBufTxUART))
			{
				tmos_set_event(Peripheral_TaskID, SBP_UART_TX_EVT); // 触发发送任务
			}
			break;

		case UART_II_MODEM_CHG: // 只支持串口0
		default:
			break;
	}
}
