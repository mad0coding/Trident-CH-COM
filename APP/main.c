/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外设从机应用主函数及任务系统初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "app_usb.h"

// Macro
#define KEY_STATE			(!GPIOB_ReadPortPin(bRXD0))
#define LED_INV				GPIOA_InverseBits(bTXD1)
#define LED_ON				GPIOA_SetBits(bTXD1)
#define LED_OFF				GPIOA_ResetBits(bTXD1)
#define LED_SET(state)		do{(state) ? LED_ON : LED_OFF;}while(0)

#define SYS_MODE_COM2UART	1
#define SYS_MODE_BLE2UART	2
#define SYS_MODE_COM2BLE	3

// Global variables
uint8_t SysMode = SYS_MODE_COM2BLE;

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
		//LED_SET(KEY_STATE);
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main(void)
{
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
#if 1//#ifdef DEBUG
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA); // LED
	GPIOB_ModeCfg(bRXD0, GPIO_ModeIN_PU); // KEY

	GPIOA_SetBits(bTXD1_);
	GPIOB_ModeCfg(bTXD1_, GPIO_ModeOut_PP_5mA);
	GPIOB_ModeCfg(bRXD1_, GPIO_ModeIN_PU);
    //UART1_DefInit();
	UART1_BaudRateCfg(115200);
    R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开
    R8_UART1_LCR = RB_LCR_WORD_SZ; // 8bit数据
    R8_UART1_IER = RB_IER_TXD_EN | RB_IER_RECV_RDY | RB_IER_THR_EMPTY; // 使能输出 使能接收中断 发送完成中断
    R8_UART1_DIV = 1;
	R16_PIN_ALTERNATE |= RB_PIN_UART1; // 引脚重映射
	UART1_ByteTrigCfg(UART_4BYTE_TRIG); // FIFO触发点4字节
    //trigB = 7;
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART1_IRQn);
#endif
    PRINT("%s\n", VER_LIB);
	printf("Test.\n");
    CH59x_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    app_usb_init();
	for(int i = 0; i < 5; i++){
		LED_ON;
		mDelaymS(500);
		LED_OFF;
		mDelaymS(500);
	}
    Main_Circulation();
}


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
		len = UART1_RecvString(rxBuf);
		if(RingBufRxUART) mrb_write(RingBufRxUART, rxBuf, len);
		//UART1_SendString(rxBuf, len); // 环回

		break;

	case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
		LED_INV;
		break;

	case UART_II_MODEM_CHG: // 只支持串口0
		break;

	default:
		break;
	}
}

/******************************** endfile @ main ******************************/
