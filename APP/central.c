/********************************** (C) COPYRIGHT *******************************
 * File Name          : central.c
 * Description        : Central role for the Trident wireless UART service.
 *                      The peer is selected by advertised service 0xFFF0.
 *********************************************************************************/

#include "CONFIG.h"
#include "central.h"
#include "peripheral.h"
#include "app_usb.h"

#define WUART_SERVICE_UUID                 0xFFF0
#define WUART_TX_UUID                      0xFFF1
#define WUART_RX_UUID                      0xFFF2

#define CENTRAL_DISCOVERY_MODE             DEVDISC_MODE_ALL
#define CENTRAL_SCAN_ACTIVE                TRUE
#define CENTRAL_SCAN_WHITELIST             FALSE
#define CENTRAL_LINK_HIGH_DUTY             FALSE
#define CENTRAL_LINK_WHITELIST             FALSE
#define CENTRAL_SCAN_DURATION              2400
#define CENTRAL_MIN_CONN_INTERVAL          6
#define CENTRAL_MAX_CONN_INTERVAL          8
#define CENTRAL_CONN_TIMEOUT               100
#define CENTRAL_CONNECT_TIMEOUT            3200
#define CENTRAL_DISCOVER_DELAY             160

#define CENTRAL_DISCOVER_EVT               0x0100
#define CENTRAL_ENABLE_NOTIFY_EVT          0x0200
#define CENTRAL_CONNECT_TIMEOUT_EVT        0x0400

typedef enum
{
    CENTRAL_LINK_IDLE,
    CENTRAL_LINK_SCANNING,
    CENTRAL_LINK_CONNECTING,
    CENTRAL_LINK_CONNECTED
} centralLinkState_t;

typedef enum
{
    CENTRAL_DISC_IDLE,
    CENTRAL_DISC_SERVICE,
    CENTRAL_DISC_CHARS,
    CENTRAL_DISC_TX_DESC
} centralDiscState_t;

static uint8_t centralTaskId = INVALID_TASK_ID;
static uint16_t centralConnHandle = GAP_CONNHANDLE_INIT;
static uint16_t centralMtu = ATT_MTU_SIZE;
static uint16_t centralSvcStartHandle;
static uint16_t centralSvcEndHandle;
static uint16_t centralTxValueHandle;
static uint16_t centralRxValueHandle;
static uint16_t centralTxNextDeclHandle;
static uint16_t centralTxCccdHandle;
static uint8_t centralPeerAddr[B_ADDR_LEN];
static uint8_t centralPeerAddrType;
static uint8_t centralPeerFound;
static uint8_t centralNotifyReady;
static centralLinkState_t centralLinkState = CENTRAL_LINK_IDLE;
static centralDiscState_t centralDiscState = CENTRAL_DISC_IDLE;

static void centralProcessTmosMsg(tmos_event_hdr_t *pMsg);
static void centralProcessGattMsg(gattMsgEvent_t *pMsg);
static void centralProcessDiscovery(gattMsgEvent_t *pMsg);
static void centralEventCB(gapRoleEvent_t *pEvent);
static void centralRssiCB(uint16_t connHandle, int8_t rssi);
static void centralDataLengthCB(uint16_t connHandle, uint16_t maxTxOctets, uint16_t maxRxOctets);
static void centralStartScan(void);
static void centralStartServiceDiscovery(void);
static uint8_t centralAdvertisesWirelessUart(const uint8_t *data, uint8_t length);
static void centralResetLink(void);
static uint8_t centralWriteNoRsp(const uint8_t *data, uint8_t length);
static void centralForwardNotification(const attHandleValueNoti_t *pNoti);

static gapCentralRoleCB_t centralRoleCallbacks = {
    centralRssiCB,
    centralEventCB,
    centralDataLengthCB
};

static gapBondCBs_t centralBondCallbacks = {
    NULL,
    NULL,
    NULL
};

void Central_Init(void)
{
    uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8_t mitm = FALSE;
    uint8_t bonding = FALSE;
    uint8_t ioCapabilities = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;

    centralTaskId = TMOS_ProcessEventRegister(Central_ProcessEvent);
    /* app_usb.c, UART.c and cmd.c intentionally keep using this shared task
     * identifier regardless of the active BLE role. */
    Peripheral_TaskID = centralTaskId;

    GAP_SetParamValue(TGAP_DISC_SCAN, CENTRAL_SCAN_DURATION);
    GAP_SetParamValue(TGAP_CONN_EST_INT_MIN, CENTRAL_MIN_CONN_INTERVAL);
    GAP_SetParamValue(TGAP_CONN_EST_INT_MAX, CENTRAL_MAX_CONN_INTERVAL);
    GAP_SetParamValue(TGAP_CONN_EST_SUPERV_TIMEOUT, CENTRAL_CONN_TIMEOUT);
    GAPBondMgr_SetParameter(GAPBOND_CENT_PAIRING_MODE, sizeof(pairMode), &pairMode);
    GAPBondMgr_SetParameter(GAPBOND_CENT_MITM_PROTECTION, sizeof(mitm), &mitm);
    GAPBondMgr_SetParameter(GAPBOND_CENT_BONDING_ENABLED, sizeof(bonding), &bonding);
    GAPBondMgr_SetParameter(GAPBOND_CENT_IO_CAPABILITIES, sizeof(ioCapabilities), &ioCapabilities);

    GATT_InitClient();
    GATT_RegisterForInd(centralTaskId);
    Bridge_Init();
    tmos_set_event(centralTaskId, SBP_START_DEVICE_EVT);
}

uint16_t Central_ProcessEvent(uint8_t task_id, uint16_t events)
{
    (void)task_id;

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg = tmos_msg_receive(centralTaskId);
        if(pMsg != NULL)
        {
            centralProcessTmosMsg((tmos_event_hdr_t *)pMsg);
            tmos_msg_deallocate(pMsg);
        }
        return events ^ SYS_EVENT_MSG;
    }

    if(events & SBP_START_DEVICE_EVT)
    {
        GAPRole_CentralStartDevice(centralTaskId, &centralBondCallbacks, &centralRoleCallbacks);
        return events ^ SBP_START_DEVICE_EVT;
    }

    if(events & CENTRAL_CONNECT_TIMEOUT_EVT)
    {
        if(centralLinkState == CENTRAL_LINK_CONNECTING)
        {
            GAPRole_TerminateLink(INVALID_CONNHANDLE);
        }
        return events ^ CENTRAL_CONNECT_TIMEOUT_EVT;
    }

    if(events & CENTRAL_DISCOVER_EVT)
    {
        centralStartServiceDiscovery();
        return events ^ CENTRAL_DISCOVER_EVT;
    }

    if(events & CENTRAL_ENABLE_NOTIFY_EVT)
    {
        attWriteReq_t req;

        if((centralConnHandle != GAP_CONNHANDLE_INIT) && (centralTxCccdHandle != 0))
        {
            req.cmd = FALSE;
            req.sig = FALSE;
            req.handle = centralTxCccdHandle;
            req.len = 2;
            req.pValue = GATT_bm_alloc(centralConnHandle, ATT_WRITE_REQ, req.len, NULL, 0);
            if(req.pValue != NULL)
            {
                req.pValue[0] = LO_UINT16(GATT_CLIENT_CFG_NOTIFY);
                req.pValue[1] = HI_UINT16(GATT_CLIENT_CFG_NOTIFY);
                if(GATT_WriteCharValue(centralConnHandle, &req, centralTaskId) != SUCCESS)
                {
                    GATT_bm_free((gattMsg_t *)&req, ATT_WRITE_REQ);
                    tmos_start_task(centralTaskId, CENTRAL_ENABLE_NOTIFY_EVT, 32);
                }
            }
            else
            {
                tmos_start_task(centralTaskId, CENTRAL_ENABLE_NOTIFY_EVT, 32);
            }
        }
        return events ^ CENTRAL_ENABLE_NOTIFY_EVT;
    }

    if(events & SBP_BLE_TX_EVT)
    {
        if(!RingBufTxBLE || !centralNotifyReady)
        {
            return events ^ SBP_BLE_TX_EVT;
        }

        while(mrb_len(RingBufTxBLE))
        {
            uint8_t buf[BLE_BUFF_MAX_LEN - 3];
            uint16_t len = mrb_len(RingBufTxBLE);
            uint16_t maxLength = centralMtu - 3;

            if(maxLength > sizeof(buf)) maxLength = sizeof(buf);
            if(len > maxLength) len = maxLength;
            if(mrb_copy(RingBufTxBLE, buf, len) != len) break;
            if(centralWriteNoRsp(buf, len) != SUCCESS)
            {
                tmos_start_task(centralTaskId, SBP_BLE_TX_EVT, 2);
                break;
            }
            mrb_del(RingBufTxBLE, len);
        }
        if(mrb_len(RingBufTxBLE) < 1024)
        {
            R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_R_RES) | UEP_R_RES_ACK;
        }
        return events ^ SBP_BLE_TX_EVT;
    }

    if(events & SBP_USB_TX_EVT)
    {
        if(RingBufTxUSB && mrb_len(RingBufTxUSB))
        {
            USBSendData();
            tmos_start_task(centralTaskId, SBP_USB_TX_EVT, 5);
        }
        return events ^ SBP_USB_TX_EVT;
    }

    if(events & SBP_UART_TX_EVT)
    {
        uint8_t dataByte;

        if(!RingBufTxUART) return events ^ SBP_UART_TX_EVT;
        while(R8_UART1_TFC != UART_FIFO_SIZE && mrb_read(RingBufTxUART, &dataByte, 1) == 1)
        {
            R8_UART1_THR = dataByte;
        }
        if(!MRB_empty(RingBufTxUART))
        {
            tmos_start_task(centralTaskId, SBP_UART_TX_EVT, 1);
        }
        if(mrb_len(RingBufTxUART) < 1024)
        {
            R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_R_RES) | UEP_R_RES_ACK;
        }
        return events ^ SBP_UART_TX_EVT;
    }

    return 0;
}

static void centralProcessTmosMsg(tmos_event_hdr_t *pMsg)
{
    if(pMsg->event == GATT_MSG_EVENT)
    {
        centralProcessGattMsg((gattMsgEvent_t *)pMsg);
    }
}

static void centralProcessGattMsg(gattMsgEvent_t *pMsg)
{
    if(pMsg->connHandle != centralConnHandle)
    {
        GATT_bm_free(&pMsg->msg, pMsg->method);
        return;
    }

    if(pMsg->method == ATT_MTU_UPDATED_EVENT)
    {
        centralMtu = pMsg->msg.mtuEvt.MTU;
        PRINT("Central MTU: %d\n", centralMtu);
    }
    else if(pMsg->method == ATT_HANDLE_VALUE_NOTI)
    {
        centralForwardNotification(&pMsg->msg.handleValueNoti);
    }
    else if((pMsg->method == ATT_WRITE_RSP) ||
            ((pMsg->method == ATT_ERROR_RSP) && (pMsg->msg.errorRsp.reqOpcode == ATT_WRITE_REQ)))
    {
        if((pMsg->method == ATT_WRITE_RSP) && (centralTxCccdHandle != 0) && !centralNotifyReady)
        {
            centralNotifyReady = TRUE;
            PRINT("Wireless UART ready\n");
            if(RingBufTxBLE && mrb_len(RingBufTxBLE))
            {
                tmos_set_event(centralTaskId, SBP_BLE_TX_EVT);
            }
        }
        else if(!centralNotifyReady)
        {
            tmos_start_task(centralTaskId, CENTRAL_ENABLE_NOTIFY_EVT, 32);
        }
    }
    else if(centralDiscState != CENTRAL_DISC_IDLE)
    {
        centralProcessDiscovery(pMsg);
    }

    GATT_bm_free(&pMsg->msg, pMsg->method);
}

static void centralProcessDiscovery(gattMsgEvent_t *pMsg)
{
    uint8_t complete = FALSE;

    if((pMsg->method == ATT_ERROR_RSP) || (pMsg->hdr.status == bleProcedureComplete))
    {
        complete = TRUE;
    }

    if(centralDiscState == CENTRAL_DISC_SERVICE)
    {
        if((pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP) && (pMsg->msg.findByTypeValueRsp.numInfo > 0))
        {
            centralSvcStartHandle = ATT_ATTR_HANDLE(pMsg->msg.findByTypeValueRsp.pHandlesInfo, 0);
            centralSvcEndHandle = ATT_GRP_END_HANDLE(pMsg->msg.findByTypeValueRsp.pHandlesInfo, 0);
        }
        if(complete)
        {
            if(centralSvcStartHandle != 0)
            {
                centralDiscState = CENTRAL_DISC_CHARS;
                GATT_DiscAllChars(centralConnHandle, centralSvcStartHandle, centralSvcEndHandle, centralTaskId);
            }
            else
            {
                PRINT("Wireless UART service not found\n");
                centralDiscState = CENTRAL_DISC_IDLE;
                GAPRole_TerminateLink(centralConnHandle);
            }
        }
    }
    else if(centralDiscState == CENTRAL_DISC_CHARS)
    {
        if((pMsg->method == ATT_READ_BY_TYPE_RSP) && (pMsg->msg.readByTypeRsp.numPairs > 0))
        {
            uint8_t i;
            uint8_t pairLength = pMsg->msg.readByTypeRsp.len;
            uint8_t *pair = pMsg->msg.readByTypeRsp.pDataList;

            for(i = 0; i < pMsg->msg.readByTypeRsp.numPairs; i++, pair += pairLength)
            {
                uint16_t declHandle;
                uint16_t valueHandle;
                uint16_t uuid;

                if(pairLength < 7) continue;
                declHandle = BUILD_UINT16(pair[0], pair[1]);
                valueHandle = BUILD_UINT16(pair[3], pair[4]);
                uuid = BUILD_UINT16(pair[5], pair[6]);
                if(uuid == WUART_TX_UUID)
                {
                    centralTxValueHandle = valueHandle;
                }
                else if(uuid == WUART_RX_UUID)
                {
                    centralRxValueHandle = valueHandle;
                    centralTxNextDeclHandle = declHandle;
                }
            }
        }
        if(complete)
        {
            if((centralTxValueHandle != 0) && (centralRxValueHandle != 0))
            {
                uint16_t descEnd = centralTxNextDeclHandle ? (centralTxNextDeclHandle - 1) : centralSvcEndHandle;
                centralDiscState = CENTRAL_DISC_TX_DESC;
                GATT_DiscAllCharDescs(centralConnHandle, centralTxValueHandle + 1, descEnd, centralTaskId);
            }
            else
            {
                PRINT("Wireless UART characteristics not found\n");
                centralDiscState = CENTRAL_DISC_IDLE;
                GAPRole_TerminateLink(centralConnHandle);
            }
        }
    }
    else if(centralDiscState == CENTRAL_DISC_TX_DESC)
    {
        if((pMsg->method == ATT_FIND_INFO_RSP) && (pMsg->msg.findInfoRsp.format == 0x01))
        {
            uint8_t i;
            uint8_t *info = pMsg->msg.findInfoRsp.pInfo;
            for(i = 0; i < pMsg->msg.findInfoRsp.numInfo; i++, info += 4)
            {
                if(BUILD_UINT16(info[2], info[3]) == GATT_CLIENT_CHAR_CFG_UUID)
                {
                    centralTxCccdHandle = BUILD_UINT16(info[0], info[1]);
                    break;
                }
            }
        }
        if(complete)
        {
            centralDiscState = CENTRAL_DISC_IDLE;
            if(centralTxCccdHandle != 0)
            {
                tmos_set_event(centralTaskId, CENTRAL_ENABLE_NOTIFY_EVT);
            }
            else
            {
                PRINT("Wireless UART TX CCCD not found\n");
                GAPRole_TerminateLink(centralConnHandle);
            }
        }
    }
}

static void centralEventCB(gapRoleEvent_t *pEvent)
{
    switch(pEvent->gap.opcode)
    {
        case GAP_DEVICE_INIT_DONE_EVENT:
            centralStartScan();
            break;

        case GAP_DEVICE_INFO_EVENT:
            if((centralLinkState == CENTRAL_LINK_SCANNING) && !centralPeerFound &&
               centralAdvertisesWirelessUart(pEvent->deviceInfo.pEvtData, pEvent->deviceInfo.dataLen))
            {
                tmos_memcpy(centralPeerAddr, pEvent->deviceInfo.addr, B_ADDR_LEN);
                centralPeerAddrType = pEvent->deviceInfo.addrType;
                centralPeerFound = TRUE;
                GAPRole_CentralCancelDiscovery();
            }
            break;

        case GAP_DEVICE_DISCOVERY_EVENT:
            if(centralLinkState == CENTRAL_LINK_SCANNING)
            {
                if(centralPeerFound)
                {
                    centralLinkState = CENTRAL_LINK_CONNECTING;
                    GAPRole_CentralEstablishLink(CENTRAL_LINK_HIGH_DUTY,
                                                 CENTRAL_LINK_WHITELIST,
                                                 centralPeerAddrType,
                                                 centralPeerAddr);
                    tmos_start_task(centralTaskId, CENTRAL_CONNECT_TIMEOUT_EVT, CENTRAL_CONNECT_TIMEOUT);
                }
                else
                {
                    centralStartScan();
                }
            }
            break;

        case GAP_LINK_ESTABLISHED_EVENT:
            tmos_stop_task(centralTaskId, CENTRAL_CONNECT_TIMEOUT_EVT);
            if(pEvent->gap.hdr.status == SUCCESS)
            {
                attExchangeMTUReq_t req = { .clientRxMTU = BLE_BUFF_MAX_LEN - 4 };
                centralLinkState = CENTRAL_LINK_CONNECTED;
                centralConnHandle = pEvent->linkCmpl.connectionHandle;
                centralMtu = ATT_MTU_SIZE;
                centralNotifyReady = FALSE;
                GATT_ExchangeMTU(centralConnHandle, &req, centralTaskId);
                tmos_start_task(centralTaskId, CENTRAL_DISCOVER_EVT, CENTRAL_DISCOVER_DELAY);
                PRINT("Central connected\n");
            }
            else
            {
                centralStartScan();
            }
            break;

        case GAP_LINK_TERMINATED_EVENT:
            centralResetLink();
            centralStartScan();
            break;

        default:
            break;
    }
}

static void centralRssiCB(uint16_t connHandle, int8_t rssi)
{
    (void)connHandle;
    (void)rssi;
}

static void centralDataLengthCB(uint16_t connHandle, uint16_t maxTxOctets, uint16_t maxRxOctets)
{
    (void)connHandle;
    (void)maxTxOctets;
    (void)maxRxOctets;
}

static void centralStartScan(void)
{
    centralPeerFound = FALSE;
    centralLinkState = CENTRAL_LINK_SCANNING;
    GAPRole_CentralStartDiscovery(CENTRAL_DISCOVERY_MODE, CENTRAL_SCAN_ACTIVE, CENTRAL_SCAN_WHITELIST);
    PRINT("Central scanning for FFF0\n");
}

static void centralStartServiceDiscovery(void)
{
    uint8_t uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(WUART_SERVICE_UUID), HI_UINT16(WUART_SERVICE_UUID) };

    if(centralConnHandle == GAP_CONNHANDLE_INIT) return;
    centralSvcStartHandle = 0;
    centralSvcEndHandle = 0;
    centralTxValueHandle = 0;
    centralRxValueHandle = 0;
    centralTxNextDeclHandle = 0;
    centralTxCccdHandle = 0;
    centralDiscState = CENTRAL_DISC_SERVICE;
    GATT_DiscPrimaryServiceByUUID(centralConnHandle, uuid, ATT_BT_UUID_SIZE, centralTaskId);
}

static uint8_t centralAdvertisesWirelessUart(const uint8_t *data, uint8_t length)
{
    uint8_t offset = 0;

    while(offset < length)
    {
        uint8_t fieldLength = data[offset];
        uint8_t i;

        if(fieldLength == 0) break;
        if((uint16_t)offset + fieldLength >= length) break;
        if((data[offset + 1] == GAP_ADTYPE_16BIT_MORE) || (data[offset + 1] == GAP_ADTYPE_16BIT_COMPLETE))
        {
            for(i = offset + 2; (uint16_t)i + 1 < (uint16_t)offset + fieldLength + 1; i += 2)
            {
                if(BUILD_UINT16(data[i], data[i + 1]) == WUART_SERVICE_UUID) return TRUE;
            }
        }
        offset += fieldLength + 1;
    }
    return FALSE;
}

static void centralResetLink(void)
{
    centralLinkState = CENTRAL_LINK_IDLE;
    centralDiscState = CENTRAL_DISC_IDLE;
    centralConnHandle = GAP_CONNHANDLE_INIT;
    centralMtu = ATT_MTU_SIZE;
    centralNotifyReady = FALSE;
    centralSvcStartHandle = 0;
    centralSvcEndHandle = 0;
    centralTxValueHandle = 0;
    centralRxValueHandle = 0;
    centralTxNextDeclHandle = 0;
    centralTxCccdHandle = 0;
    tmos_stop_task(centralTaskId, CENTRAL_DISCOVER_EVT);
    tmos_stop_task(centralTaskId, CENTRAL_ENABLE_NOTIFY_EVT);
}

static uint8_t centralWriteNoRsp(const uint8_t *data, uint8_t length)
{
    attWriteReq_t req;
    bStatus_t status;

    if(!centralNotifyReady || (centralRxValueHandle == 0) || (length == 0)) return FAILURE;
    req.cmd = TRUE;
    req.sig = FALSE;
    req.handle = centralRxValueHandle;
    req.len = length;
    req.pValue = GATT_bm_alloc(centralConnHandle, ATT_WRITE_CMD, length, NULL, 0);
    if(req.pValue == NULL) return bleMemAllocError;
    tmos_memcpy(req.pValue, data, length);
    status = GATT_WriteNoRsp(centralConnHandle, &req);
    if(status != SUCCESS)
    {
        GATT_bm_free((gattMsg_t *)&req, ATT_WRITE_CMD);
    }
    return status;
}

static void centralForwardNotification(const attHandleValueNoti_t *pNoti)
{
    if((pNoti->handle != centralTxValueHandle) || !RingBufRxBLE) return;
    if(mrb_write(RingBufRxBLE, pNoti->pValue, pNoti->len) != pNoti->len)
    {
        PRINT("RingBufRx BLE ERR\n");
    }
    if(RingBufTxUSB) tmos_set_event(centralTaskId, SBP_USB_TX_EVT);
    else if(RingBufTxUART) tmos_set_event(centralTaskId, SBP_UART_TX_EVT);
}
