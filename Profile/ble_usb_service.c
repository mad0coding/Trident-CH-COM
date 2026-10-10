/********************************** (C) COPYRIGHT *******************************
 * File Name          : ble_usb_service.c
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
#include "stdint.h"
#include "ble_usb_service.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED    8

#define BLE_USB_TX_VALUE_HANDLE       2
#define BLE_USB_RX_VALUE_HANDLE       5
#define BLE_USB_CTRL_VALUE_HANDLE     7
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// ble_usb GATT Profile Service UUID
const uint8_t ble_usb_ServiceUUID[ATT_BT_UUID_SIZE] =
    {0xf0, 0xff};

// Characteristic rx uuid
const uint8_t ble_usb_RxCharUUID[ATT_BT_UUID_SIZE] =
    {0xf2, 0xff};

// Characteristic tx uuid
const uint8_t ble_usb_TxCharUUID[ATT_BT_UUID_SIZE] =
    {0xf1, 0xff};

// Characteristic debug/control uuid
const uint8_t ble_usb_CtrlCharUUID[ATT_BT_UUID_SIZE] =
    {0xf3, 0xff};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static ble_usb_ProfileChangeCB_t ble_usb_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Profile Service attribute
static const gattAttrType_t ble_usb_Service = {ATT_BT_UUID_SIZE, ble_usb_ServiceUUID};

// RX: Central -> device serial data
static uint8_t ble_usb_RxCharProps = GATT_PROP_WRITE_NO_RSP | GATT_PROP_WRITE;

// Characteristic 1 Value
static uint8_t ble_usb_RxCharValue[BLE_USB_RX_BUFF_SIZE];
//static uint8_t ble_usb_RxCharValue[1];

// TX: Device -> Central serial data
static uint8_t ble_usb_TxCharProps = GATT_PROP_NOTIFY;

// Characteristic 2 Value
static uint8_t ble_usb_TxCharValue = 0;

static gattCharCfg_t ble_usb_TxCCCD[4];

// DBG/CTRL: Small command and status buffer.
static uint8_t  ble_usb_CtrlCharProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint8_t  ble_usb_CtrlCharValue[BLE_USB_CTRL_BUFF_SIZE];
static uint16_t ble_usb_CtrlCharLen = 0;

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t ble_usb_ProfileAttrTbl[] = {
    // Wireless UART Service (0xFFF0)
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                       /* permissions */
        0,                                      /* handle */
        (uint8_t *)&ble_usb_Service              /* pValue */
    },

    // TX Characteristic Declaration (0xFFF1)
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ble_usb_TxCharProps},

    // TX Characteristic Value
    {
        {ATT_BT_UUID_SIZE, ble_usb_TxCharUUID},
        0,
        0,
        (uint8_t *)&ble_usb_TxCharValue},

    // TX Client Characteristic Configuration
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8_t *)ble_usb_TxCCCD},

    // RX Characteristic Declaration (0xFFF2)
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ble_usb_RxCharProps},

    // RX Characteristic Value
    {
        {ATT_BT_UUID_SIZE, ble_usb_RxCharUUID},
        GATT_PERMIT_WRITE,
        0,
        &ble_usb_RxCharValue[0]},

    // DBG/CTRL Characteristic Declaration (0xFFF3)
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &ble_usb_CtrlCharProps},

    // DBG/CTRL Characteristic Value
    {
        {ATT_BT_UUID_SIZE, ble_usb_CtrlCharUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        ble_usb_CtrlCharValue},

};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t ble_usb_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                     uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t ble_usb_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                      uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static void ble_usb_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
gattServiceCBs_t ble_usb_ProfileCBs = {
    ble_usb_ReadAttrCB,  // Read callback function pointer
    ble_usb_WriteAttrCB, // Write callback function pointer
    NULL                  // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      ble_usb_AddService
 *
 * @brief   Initializes the wireless UART service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t ble_usb_add_service(ble_usb_ProfileChangeCB_t cb)
{
    uint8_t status = SUCCESS;

    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, ble_usb_TxCCCD);
    // Register with Link DB to receive link status change callback
    linkDB_Register(ble_usb_HandleConnStatusCB);

    //    ble_usb_TxCCCD.connHandle = INVALID_CONNHANDLE;
    //    ble_usb_TxCCCD.value = 0;
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(ble_usb_ProfileAttrTbl,
                                         GATT_NUM_ATTRS(ble_usb_ProfileAttrTbl),
                                         GATT_MAX_ENCRYPT_KEY_SIZE,
                                         &ble_usb_ProfileCBs);
    if(status != SUCCESS)
        PRINT("Add ble usb service failed!\n");
    ble_usb_AppCBs = cb;

    return (status);
}

/*********************************************************************
 * @fn          ble_usb_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
static bStatus_t ble_usb_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                     uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            *pLen = 2;
            tmos_memcpy(pValue, pAttr->pValue, 2);
        }
        else if(uuid == BUILD_UINT16(ble_usb_CtrlCharUUID[0], ble_usb_CtrlCharUUID[1]))
        {
            if(offset > ble_usb_CtrlCharLen)
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
            else
            {
                *pLen = MIN(maxLen, ble_usb_CtrlCharLen - offset);
                tmos_memcpy(pValue, &ble_usb_CtrlCharValue[offset], *pLen);
            }
        }
        else
        {
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }
    return (status);
}

/*********************************************************************
 * @fn      ble_usb_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 *
 * @return  Success or Failure
 */

static bStatus_t ble_usb_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                      uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    //uint8_t notifyApp = 0xFF;
    // If attribute permissions require authorization to write, return error
    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                    offset, GATT_CLIENT_CFG_NOTIFY);
            if(status == SUCCESS && ble_usb_AppCBs)
            {
                uint16_t         charCfg = BUILD_UINT16(pValue[0], pValue[1]);
                ble_usb_evt_t evt;

                evt.type = (charCfg == GATT_CFG_NO_OPERATION) ? BLE_USB_EVT_TX_NOTI_DISABLED : BLE_USB_EVT_TX_NOTI_ENABLED;
                ble_usb_AppCBs(connHandle, &evt);
            }
        }

        if(pAttr->handle == ble_usb_ProfileAttrTbl[BLE_USB_RX_VALUE_HANDLE].handle)
        {
            if(ble_usb_AppCBs)
            {
                ble_usb_evt_t evt;
                evt.type = BLE_USB_EVT_BLE_DATA_RECIEVED;
                evt.data.length = (uint16_t)len;
                evt.data.p_data = pValue;
                ble_usb_AppCBs(connHandle, &evt);
            }
        }
        else if(pAttr->handle == ble_usb_ProfileAttrTbl[BLE_USB_CTRL_VALUE_HANDLE].handle)
        {
            if((offset != 0) || (len > BLE_USB_CTRL_BUFF_SIZE))
            {
                status = ATT_ERR_INVALID_VALUE_SIZE;
            }
            else
            {
                ble_usb_CtrlCharLen = len;
                tmos_memcpy(ble_usb_CtrlCharValue, pValue, len);

                if(ble_usb_AppCBs)
                {
                    ble_usb_evt_t evt;
                    evt.type = BLE_USB_EVT_CTRL_DATA_RECEIVED;
                    evt.data.length = len;
                    evt.data.p_data = pValue;
                    ble_usb_AppCBs(connHandle, &evt);
                }
            }
        }
    }
    //    else
    //    {
    //        // 128-bit UUID
    //        if(pAttr->handle == ble_usb_ProfileAttrTbl[RAWPASS_RX_VALUE_HANDLE].handle)
    //        {
    //            if(ble_usb_AppCBs) {
    //                ble_usb_evt_t evt;
    //                evt.type = BLE_usb_EVT_BLE_DATA_RECIEVED;
    //                evt.data.length = (uint16_t)len;
    //                evt.data.p_data = pValue;
    //                ble_usb_AppCBs(connHandle,&evt);
    //            }
    //        }
    //    }
    return (status);
}

/*********************************************************************
 * @fn          ble_usb_HandleConnStatusCB
 *
 * @brief       ble_usb link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void ble_usb_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // Make sure this is not loopback connection
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            //ble_usb_TxCCCD[0].value = 0;
            GATTServApp_InitCharCfg(connHandle, ble_usb_TxCCCD);
        }
    }
}

uint8_t ble_usb_notify_is_ready(uint16_t connHandle)
{
    return (GATT_CLIENT_CFG_NOTIFY == GATTServApp_ReadCharCfg(connHandle, ble_usb_TxCCCD));
}
/*********************************************************************
 * @fn          ble_usb_notify
 *
 * @brief
 *
 * @param       connHandle - connection handle
 * @param       pNoti - pointer to notification structure
 *
 * @return      Success or Failure
 */
bStatus_t ble_usb_notify(uint16_t connHandle, attHandleValueNoti_t *pNoti, uint8_t taskId)
{
    //uint16_t value = ble_usb_TxCCCD[0].value;
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, ble_usb_TxCCCD);
    // If notifications enabled
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        // Set the handle
        pNoti->handle = ble_usb_ProfileAttrTbl[BLE_USB_TX_VALUE_HANDLE].handle;

        // Send the Indication
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

/*********************************************************************
*********************************************************************/
