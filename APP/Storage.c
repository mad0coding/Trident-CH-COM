/********************************** (C) COPYRIGHT *******************************
 * File Name          : Storage.c
 * Description        : Application DataFlash storage implementation
 *******************************************************************************/

#include "CONFIG.h"
#include "Storage.h"
#include "UART.h"

/* Defined by main.c; it is deliberately stored as one byte. */
extern uint8_t SysMode;

#define STORAGE_OFFSET_HEAD       0U
#define STORAGE_OFFSET_VERSION    2U
#define STORAGE_OFFSET_LENGTH     4U
#define STORAGE_OFFSET_CHECKSUM   6U
#define STORAGE_OFFSET_BAUDRATE   8U
#define STORAGE_OFFSET_LINE       12U
#define STORAGE_OFFSET_SYSMODE    16U

__attribute__((aligned(4))) static uint8_t Storage_Buffer[STORAGE_DATAFLASH_BLOCK_SIZE];

static uint16_t Storage_ReadLe16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t Storage_ReadLe32(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static void Storage_WriteLe16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8);
}

static void Storage_WriteLe32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)(value >> 16);
    data[3] = (uint8_t)(value >> 24);
}

/* Fletcher-16 over the complete record except its two checksum bytes. */
static uint16_t Storage_Fletcher16(const uint8_t *data, uint16_t length)
{
    uint16_t index;
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;

    for(index = 0; index < length; index++)
    {
        if(index == STORAGE_OFFSET_CHECKSUM || index == (STORAGE_OFFSET_CHECKSUM + 1U))
        {
            continue;
        }

        sum1 = (sum1 + data[index]) % 255U;
        sum2 = (sum2 + sum1) % 255U;
    }

    return (uint16_t)((sum2 << 8) | sum1);
}

static void Storage_BuildRecord(void)
{
    uint16_t index;

    for(index = 0; index < STORAGE_DATAFLASH_BLOCK_SIZE; index++)
    {
        Storage_Buffer[index] = 0xFF;
    }

    Storage_WriteLe16(&Storage_Buffer[STORAGE_OFFSET_HEAD], STORAGE_RECORD_HEAD);
    Storage_WriteLe16(&Storage_Buffer[STORAGE_OFFSET_VERSION], STORAGE_RECORD_VERSION);
    Storage_WriteLe16(&Storage_Buffer[STORAGE_OFFSET_LENGTH], STORAGE_RECORD_LENGTH);
    Storage_WriteLe32(&Storage_Buffer[STORAGE_OFFSET_BAUDRATE], UART_GetBaudRate());
    Storage_Buffer[STORAGE_OFFSET_LINE] = UART_GetLineCoding();
    Storage_Buffer[13] = 0;
    Storage_Buffer[14] = 0;
    Storage_Buffer[15] = 0;
    Storage_Buffer[STORAGE_OFFSET_SYSMODE] = SysMode;
    Storage_WriteLe16(&Storage_Buffer[STORAGE_OFFSET_CHECKSUM],
                      Storage_Fletcher16(Storage_Buffer, STORAGE_RECORD_LENGTH));
}

/*********************************************************************
 * @fn      Storage_Init
 *
 * @brief   Load a valid configuration at boot. Invalid, erased, and unknown
 *          records are ignored so default settings remain active.
 *
 * @return  none
 */
void Storage_Init(void)
{
    (void)Storage_LoadConfig();
}

/*********************************************************************
 * @fn      Storage_LoadConfig
 *
 * @brief   Read, verify, and apply the saved UART configuration and SysMode.
 *
 * @return  0 on success, nonzero if no valid record is available
 */
uint8_t Storage_LoadConfig(void)
{
    uint16_t length;

    if(EEPROM_READ(STORAGE_DATAFLASH_ADDR, Storage_Buffer, STORAGE_DATAFLASH_BLOCK_SIZE) != 0)
    {
        return 1;
    }

    if(Storage_ReadLe16(&Storage_Buffer[STORAGE_OFFSET_HEAD]) != STORAGE_RECORD_HEAD ||
       Storage_ReadLe16(&Storage_Buffer[STORAGE_OFFSET_VERSION]) != STORAGE_RECORD_VERSION)
    {
        return 1;
    }

    length = Storage_ReadLe16(&Storage_Buffer[STORAGE_OFFSET_LENGTH]);
    if(length != STORAGE_RECORD_LENGTH || length > STORAGE_DATAFLASH_BLOCK_SIZE)
    {
        return 1;
    }

    if(Storage_ReadLe16(&Storage_Buffer[STORAGE_OFFSET_CHECKSUM]) !=
       Storage_Fletcher16(Storage_Buffer, length))
    {
        return 1;
    }

    UART_SetBaudRate(Storage_ReadLe32(&Storage_Buffer[STORAGE_OFFSET_BAUDRATE]));
    UART_SetLineCoding(Storage_Buffer[STORAGE_OFFSET_LINE]);
    SysMode = Storage_Buffer[STORAGE_OFFSET_SYSMODE];
    SysMode = 3; // manually set it temporarily. TEST CODE!!!
    return 0;
}

/*********************************************************************
 * @fn      Storage_SaveConfig
 *
 * @brief   Save the current UART configuration and SysMode to DataFlash.
 *
 * @return  0 on success, nonzero on erase or write failure
 */
uint8_t Storage_SaveConfig(void)
{
    uint32_t status;

    Storage_BuildRecord();
    status = EEPROM_ERASE(STORAGE_DATAFLASH_ADDR, STORAGE_DATAFLASH_BLOCK_SIZE);
    if(status != 0)
    {
        return (uint8_t)status;
    }

    status = EEPROM_WRITE(STORAGE_DATAFLASH_ADDR, Storage_Buffer, STORAGE_DATAFLASH_BLOCK_SIZE);
    return (uint8_t)status;
}
