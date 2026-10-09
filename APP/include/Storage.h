/********************************** (C) COPYRIGHT *******************************
 * File Name          : Storage.h
 * Description        : Application DataFlash storage interface
 *******************************************************************************/

#ifndef _STORAGE_H
#define _STORAGE_H

#include <stdint.h>

/* DataFlash relative address. BLE SNV occupies 0x7E00-0x7EFF. */
#define STORAGE_DATAFLASH_ADDR       0x0000U
#define STORAGE_DATAFLASH_BLOCK_SIZE 256U

/* Fixed, little-endian record header. */
#define STORAGE_RECORD_HEAD          0x55AAU
#define STORAGE_RECORD_VERSION       0x0001U
#define STORAGE_RECORD_HEADER_SIZE   8U
#define STORAGE_RECORD_LENGTH        17U

/* Read, verify, and apply the saved configuration when it is valid. */
void Storage_Init(void);

/* Returns 0 on success; does not modify DataFlash when loading. */
uint8_t Storage_LoadConfig(void);

/* Erases the dedicated 256-byte page, then writes the current configuration. */
uint8_t Storage_SaveConfig(void);

#endif /* _STORAGE_H */
