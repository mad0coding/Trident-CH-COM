/********************************** (C) COPYRIGHT *******************************
 * File Name          : cmd.c
 * Description        : Transport-independent TCC command parser
 *******************************************************************************/

#include "CONFIG.h"
#include "cmd.h"
#include "UART.h"
#include "Storage.h"
#include "peripheral.h"

/* Defined by main.c. */
extern uint8_t SysMode;
extern uint8_t BleMode;

#define CMD_LINE_MAX_LENGTH  96U
#define CMD_MAX_TOKENS       4U

static char Cmd_Line[CMD_LINE_MAX_LENGTH + 1U];
static uint8_t Cmd_LineLength;
static uint8_t Cmd_LineOverflow;
static uint8_t Cmd_ModeActive;

static const uint8_t Cmd_DtrSequence[] = {0, 1, 1, 1};
static const uint8_t Cmd_RtsSequence[] = {1, 1, 0, 1};

#define CMD_FLOW_SEQUENCE_LENGTH ((uint8_t)sizeof(Cmd_DtrSequence))
#define CMD_FLOW_WINDOW_TICKS    (5UL * 1600UL)

static uint32_t Cmd_FlowStartTick;
static uint8_t Cmd_FlowIndex;
static uint8_t Cmd_FlowMatcherLocked;
static uint8_t Cmd_FlowMatcherInitialized;
static volatile uint8_t Cmd_BleConnected;
static uint8_t Cmd_EntryNoticePending;

static void Cmd_OutputText(const char *text);

/*********************************************************************
 * @fn      Cmd_SetMode
 *
 * @brief   Enable or disable command mode and discard a partial command.
 */
void Cmd_SetMode(uint8_t enable)
{
    uint8_t wasActive = Cmd_ModeActive;

    Cmd_ModeActive = enable ? 1U : 0U;
    Cmd_LineLength = 0;
    Cmd_LineOverflow = 0;

    if(Cmd_ModeActive && !wasActive)
    {
        Cmd_EntryNoticePending = 1U;
        Cmd_NotifyUsbReady();
    }
    else if(!Cmd_ModeActive)
    {
        Cmd_EntryNoticePending = 0U;
    }
}

/*********************************************************************
 * @fn      Cmd_IsActive
 *
 * @brief   Return whether command mode owns the input transport.
 */
uint8_t Cmd_IsActive(void)
{
    return Cmd_ModeActive;
}

void Cmd_NotifyUsbReady(void)
{
    if(Cmd_EntryNoticePending && Cmd_ModeActive && RingBufTxUSB)
    {
        Cmd_EntryNoticePending = 0U;
        Cmd_OutputText("Now in cmd mode.\r\n");
    }
}

void Cmd_FlowSequenceInit(void)
{
    Cmd_FlowStartTick = TMOS_GetSystemClock();
    Cmd_FlowIndex = 0U;
    Cmd_FlowMatcherLocked = Cmd_IsActive() ? 1U : 0U;
    Cmd_FlowMatcherInitialized = 1U;
    Cmd_BleConnected = 0U;
}

void Cmd_SetBleConnected(uint8_t connected)
{
    Cmd_BleConnected = connected ? 1U : 0U;

    if(connected)
    {
        Cmd_FlowMatcherLocked = 1U;
        Cmd_FlowIndex = 0U;
    }
}

void Cmd_CheckFlowSequence(uint8_t dtr, uint8_t rts)
{
    uint32_t elapsed;

    if((!Cmd_FlowMatcherInitialized) || Cmd_FlowMatcherLocked ||
       Cmd_BleConnected || Cmd_IsActive())
    {
        return;
    }

    elapsed = TMOS_GetSystemClock() - Cmd_FlowStartTick;
    if((Cmd_FlowIndex == 0U) && (elapsed >= CMD_FLOW_WINDOW_TICKS))
    {
        Cmd_FlowMatcherLocked = 1U;
        return;
    }

    dtr = dtr ? 1U : 0U;
    rts = rts ? 1U : 0U;

    if((dtr == Cmd_DtrSequence[Cmd_FlowIndex]) &&
       (rts == Cmd_RtsSequence[Cmd_FlowIndex]))
    {
        Cmd_FlowIndex++;
        if(Cmd_FlowIndex >= CMD_FLOW_SEQUENCE_LENGTH)
        {
            Cmd_FlowMatcherLocked = 1U;
            Cmd_FlowIndex = 0U;
            Cmd_SetMode(1U);
        }
        return;
    }

    Cmd_FlowIndex = 0U;
    if(elapsed >= CMD_FLOW_WINDOW_TICKS)
    {
        Cmd_FlowMatcherLocked = 1U;
    }
    else if((dtr == Cmd_DtrSequence[0]) && (rts == Cmd_RtsSequence[0]))
    {
        Cmd_FlowIndex = 1U;
    }
}

static uint8_t Cmd_ToLower(uint8_t c)
{
    if((c >= 'A') && (c <= 'Z'))
    {
        return (uint8_t)(c + ('a' - 'A'));
    }

    return c;
}

static uint8_t Cmd_TokenEqual(const char *left, const char *right)
{
    while((*left != '\0') && (*right != '\0'))
    {
        if(Cmd_ToLower((uint8_t)*left) != Cmd_ToLower((uint8_t)*right))
        {
            return 0;
        }
        left++;
        right++;
    }

    return (*left == '\0') && (*right == '\0');
}

static uint8_t Cmd_ParseUnsigned(const char *text, uint32_t *value)
{
    uint32_t result = 0;
    uint8_t digit;

    if((*text == '\0') || (value == 0))
    {
        return 0;
    }

    while(*text != '\0')
    {
        if((*text < '0') || (*text > '9'))
        {
            return 0;
        }

        digit = (uint8_t)(*text - '0');
        if(result > ((0xFFFFFFFFUL - digit) / 10UL))
        {
            return 0;
        }
        result = result * 10UL + digit;
        text++;
    }

    *value = result;
    return 1;
}

static void Cmd_OutputText(const char *text)
{
    uint16_t length = 0;

    while(text[length] != '\0')
    {
        length++;
    }
    Cmd_Output((const uint8_t *)text, length);
}

static void Cmd_OutputUnsigned(uint32_t value)
{
    char text[12];
    int16_t index = (int16_t)sizeof(text) - 1;

    text[index--] = '\n';
    text[index--] = '\r';
    do
    {
        text[index--] = (char)('0' + (value % 10UL));
        value /= 10UL;
    } while(value != 0UL);

    Cmd_Output((const uint8_t *)&text[index + 1],
               (uint16_t)(sizeof(text) - index - 1));
}

static uint8_t Cmd_GetDataBits(uint8_t lineCoding)
{
    return (uint8_t)((lineCoding & 0x03U) + 5U);
}

static char Cmd_GetParity(uint8_t lineCoding)
{
    switch((lineCoding >> 3) & 0x07U)
    {
        case 0: return 'N';
        case 1: return 'O';
        case 3: return 'E';
        case 5: return 'M';
        case 7: return 'S';
        default: return 'N';
    }
}

static void Cmd_ExecuteGet(const char *item)
{
    uint8_t lineCoding = UART_GetLineCoding();
    char response[12] = "parity X\r\n";

    if(Cmd_TokenEqual(item, "baudrate"))
    {
        Cmd_OutputText("baudrate ");
        Cmd_OutputUnsigned(UART_GetBaudRate());
    }
    else if(Cmd_TokenEqual(item, "databits"))
    {
        Cmd_OutputText("databits ");
        Cmd_OutputUnsigned(Cmd_GetDataBits(lineCoding));
    }
    else if(Cmd_TokenEqual(item, "parity"))
    {
        response[7] = Cmd_GetParity(lineCoding);
        Cmd_OutputText(response);
    }
    else if(Cmd_TokenEqual(item, "stopbits"))
    {
        Cmd_OutputText("stopbits ");
        Cmd_OutputUnsigned((lineCoding & 0x04U) ? 2U : 1U);
    }
    else if(Cmd_TokenEqual(item, "sysmode"))
    {
        Cmd_OutputText("sysmode ");
        Cmd_OutputUnsigned(SysMode);
    }
    else if(Cmd_TokenEqual(item, "blemode"))
    {
        Cmd_OutputText("blemode ");
        Cmd_OutputUnsigned(BleMode);
    }
    else
    {
        Cmd_OutputText("ERR item\r\n");
    }
}

static void Cmd_ExecuteSet(const char *item, const char *valueText)
{
    uint32_t value;
    uint8_t lineCoding;
    uint8_t parity;

    if(!Cmd_ParseUnsigned(valueText, &value) && !Cmd_TokenEqual(item, "parity"))
    {
        Cmd_OutputText("ERR value\r\n");
        return;
    }

    lineCoding = UART_GetLineCoding();
    if(Cmd_TokenEqual(item, "baudrate"))
    {
        if(value == 0UL)
        {
            Cmd_OutputText("ERR value\r\n");
            return;
        }
        UART_SetBaudRate(value);
    }
    else if(Cmd_TokenEqual(item, "databits"))
    {
        if((value < 5U) || (value > 8U))
        {
            Cmd_OutputText("ERR value\r\n");
            return;
        }
        UART_SetLineCoding((uint8_t)((lineCoding & (uint8_t)~0x03U) | (value - 5U)));
    }
    else if(Cmd_TokenEqual(item, "parity"))
    {
        if((valueText[1] != '\0'))
        {
            Cmd_OutputText("ERR value\r\n");
            return;
        }

        switch(Cmd_ToLower((uint8_t)valueText[0]))
        {
            case 'n': parity = 0U; break;
            case 'o': parity = 1U; break;
            case 'e': parity = 3U; break;
            case 'm': parity = 5U; break;
            case 's': parity = 7U; break;
            default:
                Cmd_OutputText("ERR value\r\n");
                return;
        }
        UART_SetLineCoding((uint8_t)((lineCoding & (uint8_t)~0x38U) | (parity << 3)));
    }
    else if(Cmd_TokenEqual(item, "stopbits"))
    {
        if(value == 1U)
        {
            UART_SetLineCoding((uint8_t)(lineCoding & (uint8_t)~0x04U));
        }
        else if(value == 2U)
        {
            /* UART_SetLineCoding rejects 5 data bits + 2 stops (1.5 stops). */
            UART_SetLineCoding((uint8_t)(lineCoding | 0x04U));
        }
        else
        {
            Cmd_OutputText("ERR value\r\n");
            return;
        }
    }
    else if(Cmd_TokenEqual(item, "sysmode"))
    {
        if((value < SYS_MODE_COM2UART) || (value > SYS_MODE_COM2BLE))
        {
            Cmd_OutputText("ERR value\r\n");
            return;
        }
        SysMode = (uint8_t)value;
    }
    else if(Cmd_TokenEqual(item, "blemode"))
    {
        if(value == BLE_MODE_PERIPHERAL)
        {
            BleMode = BLE_MODE_PERIPHERAL;
        }
        else if(value == BLE_MODE_CENTRAL)
        {
            BleMode = BLE_MODE_CENTRAL;
        }
        else
        {
            Cmd_OutputText("ERR value\r\n");
            return;
        }
    }
    else
    {
        Cmd_OutputText("ERR item\r\n");
        return;
    }

    Cmd_OutputText("OK\r\n");
}

static void Cmd_Execute(char *line)
{
    char *tokens[CMD_MAX_TOKENS];
    uint8_t count = 0;
    char *cursor = line;

    while(*cursor != '\0')
    {
        while((*cursor == ' ') || (*cursor == '\t'))
        {
            cursor++;
        }
        if(*cursor == '\0')
        {
            break;
        }
        if(count >= CMD_MAX_TOKENS)
        {
            Cmd_OutputText("ERR syntax\r\n");
            return;
        }

        tokens[count++] = cursor;
        while((*cursor != '\0') && (*cursor != ' ') && (*cursor != '\t'))
        {
            cursor++;
        }
        if(*cursor != '\0')
        {
            *cursor++ = '\0';
        }
    }

    if((count == 3U) && Cmd_TokenEqual(tokens[0], "tcc") &&
       Cmd_TokenEqual(tokens[1], "get"))
    {
        Cmd_ExecuteGet(tokens[2]);
    }
    else if((count == 4U) && Cmd_TokenEqual(tokens[0], "tcc") &&
            Cmd_TokenEqual(tokens[1], "set"))
    {
        Cmd_ExecuteSet(tokens[2], tokens[3]);
    }
    else if((count == 2U) && Cmd_TokenEqual(tokens[0], "tcc") &&
            Cmd_TokenEqual(tokens[1], "save"))
    {
        if(Storage_SaveConfig() == 0U)
        {
            Cmd_OutputText("OK\r\n");
        }
        else
        {
            Cmd_OutputText("ERR storage\r\n");
        }
    }
    else if((count == 2U) && Cmd_TokenEqual(tokens[0], "tcc") &&
            Cmd_TokenEqual(tokens[1], "exit"))
    {
        /* Queue confirmation before releasing the USB command transport. */
        Cmd_OutputText("OK\r\n");
        Cmd_SetMode(0);
    }
    else
    {
        Cmd_OutputText("ERR syntax\r\n");
    }
}

/*********************************************************************
 * @fn      Cmd_InputByte
 *
 * @brief   Feed one input byte into the command-line parser.
 *          A CR or LF terminates a command.  CRLF therefore runs it once.
 */
void Cmd_InputByte(uint8_t byte)
{
    if((byte == '\r') || (byte == '\n'))
    {
        if(Cmd_LineOverflow)
        {
            Cmd_OutputText("ERR line too long\r\n");
        }
        else if(Cmd_LineLength != 0U)
        {
            Cmd_Line[Cmd_LineLength] = '\0';
            Cmd_Execute(Cmd_Line);
        }

        Cmd_LineLength = 0;
        Cmd_LineOverflow = 0;
        return;
    }

    if(Cmd_LineOverflow)
    {
        return;
    }
    if(Cmd_LineLength >= CMD_LINE_MAX_LENGTH)
    {
        Cmd_LineOverflow = 1;
        return;
    }

    Cmd_Line[Cmd_LineLength++] = (char)byte;
}

/*********************************************************************
 * @fn      Cmd_Input
 *
 * @brief   Feed an arbitrary input buffer into the command-line parser.
 */
void Cmd_Input(const uint8_t *data, uint16_t length)
{
    uint16_t index;

    if(data == 0)
    {
        return;
    }

    for(index = 0; index < length; index++)
    {
        Cmd_InputByte(data[index]);
    }
}

/*********************************************************************
 * @fn      Cmd_Output
 *
 * @brief   Command response output hook.  Add the selected USB/UART/BLE
 *          routing here after the command transport is decided.
 */
void Cmd_Output(const uint8_t *data, uint16_t length)
{
    if(!Cmd_ModeActive || (data == 0) || (length == 0U) || !RingBufTxUSB)
    {
        return;
    }

    /* RingBufTxUSB is consumed by the normal USB TX task.  The command
     * parser may run from the USB ISR, just like the existing USB RX path. */
    if(mrb_write(RingBufTxUSB, data, length) == length)
    {
        tmos_set_event(Peripheral_TaskID, SBP_USB_TX_EVT);
    }
}
