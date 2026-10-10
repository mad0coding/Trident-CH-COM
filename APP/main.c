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
#include "peripheral.h"
#include "app_usb.h"
#include "Storage.h"
#include "UART.h"
#include "cmd.h"


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
		//LED_SET(KEY_GET);
        LED_OFF;
		// LED_SET(UART_FlowDTR); // display flow ctrl out signal
		// if(KEY_GET) UART_SetFlowInputs(UART_FLOW_INPUT_CTS, UART_FLOW_INPUT_CTS); // set flow ctrl in signal
		// else UART_SetFlowInputs(UART_FLOW_INPUT_CTS, ~UART_FLOW_INPUT_CTS);
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

	GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA); // LED
	GPIOB_ModeCfg(bRXD0, GPIO_ModeIN_PU); // KEY

	Cmd_SetMode(KEY_GET); // read the key at init stage to enter cmd mode

	UART1_Init(); // 初始化串口1
	// 初始化串口0
	GPIOA_ModeCfg(bTXD0_, GPIO_ModeOut_PP_5mA);
	GPIOA_ModeCfg(bRXD0_, GPIO_ModeIN_PU);
	UART0_DefInit();
	R16_PIN_ALTERNATE |= RB_PIN_UART0; // 引脚重映射

	Storage_Init(); // 加载有效的上电串口配置和工作模式

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

