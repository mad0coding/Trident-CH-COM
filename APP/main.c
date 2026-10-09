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


// Global variables
uint8_t SysMode = SYS_MODE_COM2UART;

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
		//LED_SET(KEY_GET);
        LED_OFF;
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
	// 初始化串口0
	GPIOA_ModeCfg(bTXD0_, GPIO_ModeOut_PP_5mA);
	GPIOA_ModeCfg(bRXD0_, GPIO_ModeIN_PU);
	UART0_DefInit();
	R16_PIN_ALTERNATE |= RB_PIN_UART0; // 引脚重映射

//	// PWM测试代码
//	PWMX_CLKCfg(7); // 预分频 Fbase = Fsys/7
//	PWMX_16bit_CycleCfg(65000-1); // 16位数据宽度 Ncyc = RB_PWM_CYC_VALUE + 1
//	PWMX_16bit_ACTOUT(CH_PWM4, 0, High_Level, ENABLE); // PWM4 - 0%占空比
//	GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeOut_PP_20mA); // BUZZ
//	R32_PWM_REG_CYCLE = 65000 / (12*0 + 1);
//	R16_PWM4_DATA = 300;
//	while(1);
	
    PRINT("%s\n", VER_LIB);
	printf("Test.\n");
    CH59x_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    app_usb_init();
//	for(int i = 0; i < 5; i++){
//		LED_ON;
//		mDelaymS(500);
//		LED_OFF;
//		mDelaymS(500);
//	}
    Main_Circulation();
}

/******************************** endfile @ main ******************************/

