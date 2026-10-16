/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_usb.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/01/19
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "CONFIG.h"
#include "gattprofile.h"
#include "stdint.h"
#include "ble_usb_service.h"
#include "app_usb.h"
#include "peripheral.h"
#include "UART.h"
#include "cmd.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
uint8_t DevConfig, Ready;
uint8_t SetupReqCode;
UINT16 SetupReqLen;
const uint8_t *pDescr;

#define DevEP0SIZE  0x40
// 设备描述符
const uint8_t MyDevDescr[] = { 0x12,0x01,0x10,0x01,0xFF,0x00,0x00,DevEP0SIZE,
                             0x86,0x1A,0x23,0x75,0x63,0x02,0x00,0x02,
                             0x00,0x01 };
// 配置描述符
const uint8_t MyCfgDescr[] = {   0x09,0x02,0x27,0x00,0x01,0x01,0x00,0x80,0xf0,              //配置描述符，接口描述符,端点描述符
                                 0x09,0x04,0x00,0x00,0x03,0xff,0x01,0x02,0x00,
                                 0x07,0x05,0x82,0x02,0x20,0x00,0x00,                        //批量上传端点
                                 0x07,0x05,0x02,0x02,0x20,0x00,0x00,                        //批量下传端点
                                 0x07,0x05,0x81,0x03,0x08,0x00,0x01};                       //中断上传端点
// 语言描述符
const uint8_t MyLangDescr[] = { 0x04, 0x03, 0x09, 0x04 };
// 厂家信息
const uint8_t MyManuInfo[] = { 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };
// 产品信息
const uint8_t MyProdInfo[] = { 0x0C, 0x03, 'C', 0, 'H', 0, '3', 0, '4', 0, '0', 0 }; // 暂时由CH59x改为CH340
/*产品描述符*/
const uint8_t StrDesc[28] =
{
  0x1C,0x03,0x55,0x00,0x53,0x00,0x42,0x00,
  0x32,0x00,0x2E,0x00,0x30,0x00,0x2D,0x00,
  0x53,0x00,0x65,0x00,0x72,0x00,0x69,0x00,
  0x61,0x00,0x6C,0x00
};

const uint8_t Return1[2] = {0x31,0x00};
const uint8_t Return2[2] = {0xC3,0x00};
/* CH340 active-low modem-state reply: bit6=RTS#, bit5=DTR#,
 * bits3..0=DCD#/RI#/DSR#/CTS#. */
static uint8_t Ch340ModemState[2] = {0x9F, 0x88};
static uint8_t Ch340ReportedModemState = 0xFF;
static volatile uint8_t Ch340ModemStatusPending = 1;

/*********************************************************************
 * LOCAL VARIABLES
 */

/******** 用户自定义分配端点RAM ****************************************/
__attribute__((aligned(4)))  uint8_t EP0_Databuf[64 + 64 + 64];    //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4)))  uint8_t EP1_Databuf[64 + 64];    //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4)))  uint8_t EP2_Databuf[64 + 64];    //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4)))  uint8_t EP3_Databuf[64 + 64];    //ep3_out(64)+ep3_in(64)

/*********************************************************************
 * @fn      Ch340UpdateModemState
 *
 * @brief   Refresh the two-byte status returned by the CH340 status query.
 *
 * @return  none
 */
static void Ch340UpdateModemState(void)
{
    Ch340ModemState[0] = UART_GetFlowStatus();
    Ch340ModemState[1] = 0x88;
}

/*********************************************************************
 * @fn      Ch340QueueModemStatus
 *
 * @brief   Send a CH340-compatible interrupt-endpoint notification whenever
 *          the combined modem status changes.  The endpoint is armed only
 *          after configuration and only when its previous IN transfer ended.
 *
 * @return  none
 */
static void Ch340QueueModemStatus(void)
{
    uint8_t status = UART_GetFlowStatus();

    if(status != Ch340ReportedModemState)
    {
        Ch340ModemStatusPending = 1;
    }

    if(!DevConfig || !Ch340ModemStatusPending ||
       (R8_UEP1_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK)
    {
        return;
    }

    pEP1_IN_DataBuf[0] = 0x08;
    pEP1_IN_DataBuf[1] = status;
    pEP1_IN_DataBuf[2] = status;
    pEP1_IN_DataBuf[3] = 0x88;
    DevEP1_IN_Deal(4);
    Ch340ReportedModemState = status;
    Ch340ModemStatusPending = 0;
}

/*********************************************************************
 * @fn      USB_NotifyFlowStatus
 *
 * @brief   Immediately try to submit a changed modem-input state through
 *          the CH340 interrupt endpoint.  It is safe to call before USB is
 *          configured; in that case the notification is deferred.
 *
 * @return  none
 */
void USB_NotifyFlowStatus(void)
{
    Ch340QueueModemStatus();
}

/*********************************************************************
 * @fn      USB_ResendFlowStatus
 *
 * @brief   Force the current modem-input state to be sent on the next
 *          available endpoint-1 IN transfer, even if it was reported before.
 *          The CH340 baud-rate request uses this as the port-open cue.
 *
 * @return  none
 */
void USB_ResendFlowStatus(void)
{
    Ch340ModemStatusPending = 1;
    Ch340QueueModemStatus();
}

/*********************************************************************
 * @fn      Ch340DecodeBaudRate
 *
 * @brief   Convert the CH340 vendor baud-rate code to baud.  Unmapped
 *          values fall back to the safe default of 115200 bps.
 */
static uint32_t Ch340DecodeBaudRate(uint8_t code0, uint8_t code1)
{
    switch(((uint16_t)code1 << 8) | code0)
    {
        case 0x9680: return 110;
        case 0xD980: return 300;
        case 0x6481: return 600;
        case 0xB281: return 1200;
        case 0xD981: return 2400;
        case 0x6482: return 4800;
        case 0xB282: return 9600;
        case 0xCC82: return 14400;
        case 0xD982: return 19200;
        case 0x6483: return 38400;
        case 0x9583: return 56000;
        case 0x9883: return 57600;
        case 0xCC83: return 115200;
        case 0xD183: return 128000;
        case 0xE683: return 230400;
        case 0xE983: return 256000;
        case 0xF383: return 460800;
        case 0xF483: return 512000;
        case 0xF683: return 600000;
        case 0xF883: return 750000;
        case 0xF387: return 921600;
        default:     return 115200; // use 115200 for unknown setting
    }
}

/*********************************************************************
 * @fn      Ch340PrintLineCoding
 *
 * @brief   Decode and print the line format carried by CH340 request
 *          40 9A 18 25 xx 00 00 00, then apply it to UART1.
 */
static void Ch340PrintLineCoding(uint8_t format)
{
    const char *parity = "unknown";
    uint8_t parityCode = (format >> 3) & 0x07;

    switch(parityCode)
    {
        case 0: parity = "none";  break;
        case 1: parity = "odd";   break;
        case 3: parity = "even";  break;
        case 5: parity = "mark";  break;
        case 7: parity = "space"; break;
        default:                  break;
    }

    UART_SetLineCoding(format);
    // printf("CH340 line: %u data, %u stop, %s parity (raw=%02X)\r\n",
    //        5 + (format & 0x03), (format & 0x04) ? 2 : 1, parity, format);
}

/*********************************************************************
 * @fn      Ch340HandleVendorOut
 *
 * @brief   Record the CH340 vendor control requests sent by the host.
 *          Physical UART configuration and modem GPIOs are deliberately
 *          not changed in this diagnostic stage.
 */
static void Ch340HandleVendorOut(PUSB_SETUP_REQ request)
{
    uint8_t value;
    uint32_t baudrate;

    if(request->bRequest == 0xA4)
    {
        value = (uint8_t)request->wValue;
        if((value & 0x9F) == 0x9F)
        {
            UART_SetFlowOutputs(value & 0x20, value & 0x40);
            // printf("CH340 flow: RTS=%s, DTR=%s (raw=%02X)\r\n",
            //        UART_FlowRTS ? "ASSERT" : "DEASSERT",
            //        UART_FlowDTR ? "ASSERT" : "DEASSERT", value);
        }
        return;
    }

    if(request->bRequest != 0x9A)
    {
        return;
    }

    if(request->wValue == 0x1312)
    {
        baudrate = Ch340DecodeBaudRate((uint8_t)request->wIndex,
                                       (uint8_t)(request->wIndex >> 8));
        UART_SetBaudRate(baudrate); // set UART baudrate
        USB_ResendFlowStatus();
        // if(baudrate != 0)
        // {
        //     printf("CH340 baud: %lu (raw=%02X %02X)\r\n", (unsigned long)baudrate,
        //            (uint8_t)request->wIndex, (uint8_t)(request->wIndex >> 8));
        // }
        // else
        // {
        //     printf("CH340 baud: unknown (raw=%02X %02X)\r\n",
        //            (uint8_t)request->wIndex, (uint8_t)(request->wIndex >> 8));
        // }
    }
    else if(request->wValue == 0x2518)
    {
        Ch340PrintLineCoding((uint8_t)request->wIndex);
    }
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      app_usb_init
 *
 * @brief   初始化usb
 *
 * @return  none
 */
void app_usb_init()
{
    pEP0_RAM_Addr = EP0_Databuf;
    pEP1_RAM_Addr = EP1_Databuf;
    pEP2_RAM_Addr = EP2_Databuf;
    pEP3_RAM_Addr = EP3_Databuf;

    USB_DeviceInit();
    PFIC_EnableIRQ( USB_IRQn );
}

/*********************************************************************
 * @fn      USBSendData
 *
 * @brief   发送数据给主机
 *
 * @return  none
 */
uint8_t USBSendData(void)
{
	if(!RingBufTxUSB) return SUCCESS;
	if((R8_UEP2_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK)
	{
		return FAILURE;
	}
	if(mrb_len(RingBufTxUSB) > 32)
	{
		mrb_read(RingBufTxUSB, pEP2_IN_DataBuf, 32);
		DevEP2_IN_Deal(32);
	}
	else
	{
		uint8_t len = mrb_len(RingBufTxUSB);
		mrb_read(RingBufTxUSB, pEP2_IN_DataBuf, len);
		DevEP2_IN_Deal(len);
	}
	return SUCCESS;
}

/*********************************************************************
 * @fn      DevEP1_OUT_Deal
 *
 * @brief   端点1数据处理
 *
 * @return  none
 */
void DevEP1_OUT_Deal( uint8_t l )
{ /* 用户可自定义 */
}

/*********************************************************************
 * @fn      DevEP2_OUT_Deal
 *
 * @brief   端点2数据处理
 *
 * @return  none
 */
void DevEP2_OUT_Deal( uint8_t l )
{ /* 用户可自定义 */
  if(Cmd_IsActive())
  {
    /* Command mode owns USB RX; do not place command bytes in a forwarding
     * buffer or trigger UART/BLE transmission. */
    Cmd_Input(pEP2_OUT_DataBuf, l);
    return;
  }

  if(!RingBufRxUSB) return;
	if(mrb_write(RingBufRxUSB, pEP2_OUT_DataBuf, l) != l){ // 读取USB数据到缓存
		PRINT("RingBufBLE ERR \n");
	}
	if(mrb_len(RingBufRxUSB) > 1024){ // 缓存使用过半 实际只留一个USB包的空间即可 但这里保守处理
		R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~MASK_UEP_R_RES ) | UEP_R_RES_NAK; // 设为忙响应
	}
  if(RingBufTxBLE) tmos_start_task(Peripheral_TaskID, SBP_BLE_TX_EVT, /*32*/2); // 准备触发发送任务
  else if(RingBufTxUART) tmos_set_event(Peripheral_TaskID, SBP_UART_TX_EVT); // 触发发送任务
}

/*********************************************************************
 * @fn      DevEP3_OUT_Deal
 *
 * @brief   端点3数据处理
 *
 * @return  none
 */
void DevEP3_OUT_Deal( uint8_t l )
{ /* 用户可自定义 */
}

/*********************************************************************
 * @fn      DevEP4_OUT_Deal
 *
 * @brief   端点4数据处理
 *
 * @return  none
 */
void DevEP4_OUT_Deal( uint8_t l )
{ /* 用户可自定义 */
}

/*********************************************************************
 * @fn      USB_DevTransProcess
 *
 * @brief   USB 传输处理函数
 *
 * @return  none
 */
void USB_DevTransProcess( void )
{
  uint8_t len, chtype;
  uint8_t intflag, errflag = 0;

  intflag = R8_USB_INT_FG;
  if ( intflag & RB_UIF_TRANSFER )
  {
    if ( ( R8_USB_INT_ST & MASK_UIS_TOKEN ) != MASK_UIS_TOKEN )    // 非空闲
    {
      switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP ) )
      // 分析操作令牌和端点号
      {
        case UIS_TOKEN_IN :
        {
          switch ( SetupReqCode )
          {
            case USB_GET_DESCRIPTOR :
              len = SetupReqLen >= DevEP0SIZE ?
                  DevEP0SIZE : SetupReqLen;    // 本次传输长度
              memcpy( pEP0_DataBuf, pDescr, len ); /* 加载上传数据 */
              SetupReqLen -= len;
              pDescr += len;
              R8_UEP0_T_LEN = len;
              R8_UEP0_CTRL ^= RB_UEP_T_TOG;                             // 翻转
              break;
            case USB_SET_ADDRESS :
              R8_USB_DEV_AD = ( R8_USB_DEV_AD & RB_UDA_GP_BIT ) | SetupReqLen;
              R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
              break;
            default :
              R8_UEP0_T_LEN = 0;                                      // 状态阶段完成中断或者是强制上传0长度数据包结束控制传输
              R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
              break;
          }
        }
          break;

        case UIS_TOKEN_OUT :
        {
            len = R8_USB_RX_LEN;
            /* End of a control-read transfer: return EP0 to its idle state. */
            R8_UEP0_T_LEN = 0;
            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        }
          break;

        case UIS_TOKEN_OUT | 1 :
        {
          if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
          {                       // 不同步的数据包将丢弃
            len = R8_USB_RX_LEN;
            DevEP1_OUT_Deal( len );
          }
        }
          break;

        case UIS_TOKEN_IN | 1 :
          R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
          break;

        case UIS_TOKEN_OUT | 2 :
        {
          if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
          {                       // 不同步的数据包将丢弃
            len = R8_USB_RX_LEN;
            DevEP2_OUT_Deal( len );
          }
        }
          break;

        case UIS_TOKEN_IN | 2 :
          R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
          break;

        case UIS_TOKEN_OUT | 3 :
        {
          if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
          {                       // 不同步的数据包将丢弃
            len = R8_USB_RX_LEN;
            DevEP3_OUT_Deal( len );
          }
        }
          break;

        case UIS_TOKEN_IN | 3 :
          R8_UEP3_CTRL = ( R8_UEP3_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
          break;

        case UIS_TOKEN_OUT | 4 :
        {
          if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
          {
            R8_UEP4_CTRL ^= RB_UEP_R_TOG;
            len = R8_USB_RX_LEN;
            DevEP4_OUT_Deal( len );
          }
        }
          break;

        case UIS_TOKEN_IN | 4 :
          R8_UEP4_CTRL ^= RB_UEP_T_TOG;
          R8_UEP4_CTRL = ( R8_UEP4_CTRL & ~MASK_UEP_T_RES ) | UEP_T_RES_NAK;
          break;

        default :
          break;
      }
      R8_USB_INT_FG = RB_UIF_TRANSFER;
    }
    if ( R8_USB_INT_ST & RB_UIS_SETUP_ACT )                  // Setup包处理
    {
      R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
      SetupReqLen = pSetupReqPak->wLength;
      SetupReqCode = pSetupReqPak->bRequest;
      chtype = pSetupReqPak->bRequestType;

      len = 0;
      errflag = 0;
      /*
       * On CH592, R8_USB_RX_LEN is not reliable while RB_UIS_SETUP_ACT is
       * asserted; it can read as zero for a valid eight-byte SETUP packet.
       * Do not use it to validate a SETUP transaction here.
       */
      if ( ( pSetupReqPak->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
      {
        if( pSetupReqPak->bRequestType == 0xC0 )
        {
          if(SetupReqCode==0x5F)
          {
            pDescr = Return1;
            len = sizeof(Return1);
          }
          else if(SetupReqCode==0x95)
          {
            if(pSetupReqPak->wValue == 0x2518)
            {
              pDescr = Return2;
              len = sizeof(Return2);
            }
            else if(pSetupReqPak->wValue == 0x0706)
            {
              Ch340UpdateModemState();
              pDescr = Ch340ModemState;
              len = sizeof(Ch340ModemState);
            }
            else // can't use errflag = 0xFF; so just use Return2 here instead of random data
            {
              pDescr = Return2;
              len = sizeof(Return2);
            }
          }
          else
          {
            errflag = 0xFF;
          }
          if(errflag == 0)
          {
            if(SetupReqLen > len)
            {
              SetupReqLen = len;
            }
            memcpy(pEP0_DataBuf,pDescr,len);
          }
        }
        else if(pSetupReqPak->bRequestType == 0x40)
        {
          /* CH340 host-to-device requests use the SETUP packet only. */
          Ch340HandleVendorOut(pSetupReqPak);
          len = 0;
        }
        else
        {
          len = 0;
        }
      }
      else /* 标准请求 */
      {
        switch ( SetupReqCode )
        {
          case USB_GET_DESCRIPTOR :
          {
            switch ( ( ( pSetupReqPak->wValue ) >> 8 ) )
            {
              case USB_DESCR_TYP_DEVICE :
              {
                pDescr = MyDevDescr;
                len = sizeof(MyDevDescr);
              }
                break;

              case USB_DESCR_TYP_CONFIG :
              {
                pDescr = MyCfgDescr;
                len = sizeof(MyCfgDescr);
              }
                break;

              case USB_DESCR_TYP_REPORT :
                /* This is a vendor interface, not HID; no report descriptor exists. */
                errflag = 0xFF;
                break;

              case USB_DESCR_TYP_STRING :
              {
                switch ( ( pSetupReqPak->wValue ) & 0xff )
                {
                  case 1 :
                    pDescr = MyManuInfo;
                    len = MyManuInfo[0];
                    break;
                  case 2 :
                    pDescr = StrDesc;
                    len = StrDesc[0];
                    break;
                  case 0 :
                    pDescr = MyLangDescr;
                    len = MyLangDescr[0];
                    break;
                  default :
                    errflag = 0xFF;                               // 不支持的字符串描述符
                    break;
                }
              }
                break;

              default :
                errflag = 0xff;
                break;
            }
            if(errflag == 0)
            {
              if ( SetupReqLen > len )
                SetupReqLen = len;      //实际需上传总长度
              len = ( SetupReqLen >= DevEP0SIZE ) ?
                  DevEP0SIZE : SetupReqLen;
              memcpy( pEP0_DataBuf, pDescr, len );
              pDescr += len;
            }
          }
            break;

          case USB_SET_ADDRESS :
            SetupReqLen = ( pSetupReqPak->wValue ) & 0xff;
            break;

          case USB_GET_CONFIGURATION :
            pEP0_DataBuf[0] = DevConfig;
            if ( SetupReqLen > 1 )
              SetupReqLen = 1;
            break;

          case USB_SET_CONFIGURATION :
            if((pSetupReqPak->wValue & 0xff) > 1)
            {
              errflag = 0xFF;
            }
            else
            {
              DevConfig = ( pSetupReqPak->wValue ) & 0xff;
              if(!DevConfig)
              {
                Ch340ReportedModemState = 0xFF;
                Ch340ModemStatusPending = 1;
              }
            }
            break;

          case USB_CLEAR_FEATURE :
          {
            if ( ( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )    // 端点
            {
              switch ( ( pSetupReqPak->wIndex ) & 0xff )
              {
                case 0x82 :
                  R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~( RB_UEP_T_TOG | MASK_UEP_T_RES ) ) | UEP_T_RES_NAK;
                  break;
                case 0x02 :
                  R8_UEP2_CTRL = ( R8_UEP2_CTRL & ~( RB_UEP_R_TOG | MASK_UEP_R_RES ) ) | UEP_R_RES_ACK;
                  break;
                case 0x81 :
                  R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~( RB_UEP_T_TOG | MASK_UEP_T_RES ) ) | UEP_T_RES_NAK;
                  break;
                case 0x01 :
                  R8_UEP1_CTRL = ( R8_UEP1_CTRL & ~( RB_UEP_R_TOG | MASK_UEP_R_RES ) ) | UEP_R_RES_ACK;
                  break;
                default :
                  errflag = 0xFF;                                 // 不支持的端点
                  break;
              }
            }
            else
              errflag = 0xFF;
          }
            break;

          case USB_GET_INTERFACE :
            pEP0_DataBuf[0] = 0x00;
            if ( SetupReqLen > 1 )
              SetupReqLen = 1;
            break;

          case USB_GET_STATUS :
            pEP0_DataBuf[0] = 0x00;
            pEP0_DataBuf[1] = 0x00;
            if ( SetupReqLen > 2 )
              SetupReqLen = 2;
            break;

          default :
            errflag = 0xff;
            break;
        }
      }
      if ( errflag == 0xff )        // 错误或不支持
      {
//                  SetupReqCode = 0xFF;
        R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;    // STALL
      }
      else
      {
        if ( chtype & 0x80 )     // 上传
        {
          len = ( SetupReqLen > DevEP0SIZE ) ?
              DevEP0SIZE : SetupReqLen;
          SetupReqLen -= len;
        }
        else
          len = 0;        // 下传
        R8_UEP0_T_LEN = len;
        R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;    // 默认数据包是DATA1
      }

      R8_USB_INT_FG = RB_UIF_TRANSFER;
    }
    Ch340QueueModemStatus();
  }
  else if ( intflag & RB_UIF_BUS_RST )
  {
    R8_USB_DEV_AD = 0;
    DevConfig = 0;
    Ready = 0;
    Ch340ReportedModemState = 0xFF;
    Ch340ModemStatusPending = 1;
    R8_UEP1_T_LEN = 0;
    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_USB_INT_FG = RB_UIF_BUS_RST;
  }
  else if ( intflag & RB_UIF_SUSPEND )
  {
    if ( R8_USB_MIS_ST & RB_UMS_SUSPEND )
    {
      ;
    }    // 挂起
    else
    {
      ;
    }               // 唤醒
    R8_USB_INT_FG = RB_UIF_SUSPEND;
  }
  else
  {
    R8_USB_INT_FG = intflag;
  }
}

/*********************************************************************
 * @fn      USB_IRQHandler
 *
 * @brief   USB中断函数
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void USB_IRQHandler( void ) /* USB中断服务程序,使用寄存器组1 */
{
  USB_DevTransProcess();
}


/*********************************************************************
*********************************************************************/
