#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "definitions.h"
#include "wifi.h"
#include "config/default/driver/spi/drv_spi_definitions.h"
#include "network.h"
#include "config/default/library/tcpip/tcpip_mac.h"
#include "config/default/library/tcpip/tcpip_mac_object.h"
#include "config/default/library/tcpip/src/link_list.h"
#define MAC_SHORT_DELAY 2
#define MAC_LONG_DELAY 2

struct mac_request {
    struct mac_request *next;
    TCPIP_MAC_ADDR addr;
};

enum spi_state {
    SPI_INIT = 0,
    SPI_HW_RESET_START,
    SPI_HW_RESET_WAIT,
    SPI_HW_RESET_END,
    SPI_SEND_RESET,
    SPI_SEND_MAC,
    SPI_SEND_SSID,
    SPI_SEND_PSK,
    SPI_ASK_STATUS,
    SPI_GET_STATUS,
    SPI_IDLE,
    SPI_GET_PLEN,
    SPI_RX_PLEN,
    SPI_ASK_PACKET,
    SPI_GET_PACKET,
};


enum wifi_command_result {
    WIFI_CMD_IDLE = 0,
    WIFI_CMD_PROCESSING,
    WIFI_CMD_DONE,
};

enum wifi_command_state {
    WIFI_CMD_STATE_IDLE = 0,
    WIFI_CMD_STATE_SEND,
};

enum wifi_command_mode {
    WIFI_READ = 0,
    WIFI_WRITE
};


#define WIFI_MAX_RX_BUFFERS 2
#define WIFI_RX_BUFFER_SIZE SPI_MAX_SIZE

enum wifi_main_state {
    WIFI_STATE_INITIALIZE = 0,
    WIFI_STATE_RESET_WAIT,
    WIFI_STATE_RESET_DONE,
    WIFI_STATE_SEND_MAC,
    WIFI_STATE_WAIT_MAC,
    WIFI_STATE_SEND_SSID,
    WIFI_STATE_WAIT_SSID,
    WIFI_STATE_SEND_PSK,
    WIFI_STATE_WAIT_PSK,
    WIFI_STATE_RUN,
    
    WIFI_STATE_SEND_STATUS,
    WIFI_STATE_WAIT_STATUS,
    WIFI_STATE_GET_STATUS,
    WIFI_STATE_HAVE_STATUS,
    

    WIFI_STATE_SEND_RBSTAT,
    WIFI_STATE_WAIT_RBSTAT,
    WIFI_STATE_GET_SIZE,
    WIFI_STATE_HAVE_SIZE,
    WIFI_STATE_SEND_READ,
    WIFI_STATE_WAIT_READ,
    WIFI_STATE_GET_READ,
    WIFI_STATE_HAVE_READ,
    
    WIFI_STATE_QUEUE_READ,
    
    WIFI_STATE_SEND_WRITE,
    WIFI_STATE_WAIT_WRITE
    
};




struct wifi_data {
    TCPIP_MAC_MODULE_CTRL stackConfig;
    
    TCPIP_MAC_PARAMETERS stackParameters;
    TCPIP_MAC_EVENT eventMask;
    TCPIP_MAC_EVENT currentEvents;

    PROTECTED_SINGLE_LIST rxFreePackets;
    PROTECTED_SINGLE_LIST rxWaitingForPickupPackets;
    PROTECTED_SINGLE_LIST txPendingPackets;
    PROTECTED_SINGLE_LIST macSetRequests;
    
    DRV_HANDLE spiBus;
    
    enum wifi_main_state mainState;
    
    bool stackConfigReady;
    
    DRV_SPI_TRANSFER_HANDLE transfer;
    uint8_t spibuf[SPI_MAX_SIZE];
    
    struct {
        uint32_t stateDelay;
        uint32_t queryStatus;
    } timers;
    
    wifi_status_t phyStatus;

    struct {
        uint32_t rxPackets;
        uint32_t txPackets;
        uint32_t rxTime;
        uint32_t txTime;
        uint32_t rtTime;
    } stats;

    TCPIP_MAC_PACKET *pkt;
    uint32_t txnTimeout;
};

struct wifi_data wifi_data;


void wifi_set_event(struct wifi_data *data, TCPIP_MAC_EVENT events) {
    data->currentEvents |= events;
    data->stackConfig.eventF(events, data->stackConfig.eventParam);
}

void wifi_rx_packet_ack(TCPIP_MAC_PACKET* pkt,  const void* param) {
    struct wifi_data *data = (struct wifi_data *)param;
    TCPIP_Helper_ProtSglListTailAdd(&data->rxFreePackets, (SGL_LIST_NODE *)pkt);
}

volatile bool spi_transfer_complete = false;

void spi_event_handler( DRV_SPI_TRANSFER_EVENT event, DRV_SPI_TRANSFER_HANDLE transferHandle, uintptr_t context ) {
    
    switch (event) {
        case DRV_SPI_TRANSFER_EVENT_PENDING:
//            DBG("P");
            break;
        case DRV_SPI_TRANSFER_EVENT_COMPLETE:
//            DBG("C");
            spi_transfer_complete = true;
            break;
        case DRV_SPI_TRANSFER_EVENT_HANDLE_EXPIRED:
//            DBG("X");
            break;
        case DRV_SPI_TRANSFER_EVENT_ERROR:
//            DBG("E");
            break;
        case DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID:
//            DBG("I");
            break;
    }
    
}


bool wifi_set_mac_ctl(SYS_MODULE_OBJ object, const TCPIP_MAC_MODULE_CTRL * init) {
    TCPIP_MAC_PACKET * pkt;
    struct wifi_data * pDrvInst = ( struct wifi_data *)object;
    
    memcpy(&pDrvInst->stackConfig, init, sizeof(TCPIP_MAC_MODULE_CTRL));
    memcpy(&pDrvInst->stackParameters.ifPhyAddress.v, init->ifPhyAddress.v, sizeof(TCPIP_MAC_ADDR));
    pDrvInst->stackParameters.macType = TCPIP_MAC_TYPE_WLAN;
    pDrvInst->stackParameters.processFlags = TCPIP_MAC_PROCESS_FLAG_NONE;
    pDrvInst->stackConfigReady = true;

    pDrvInst->spiBus = DRV_SPI_Open(0, DRV_IO_INTENT_READWRITE);
    if (pDrvInst->spiBus == DRV_HANDLE_INVALID) {
        return false;
    }

    DRV_SPI_TRANSFER_SETUP setup;

    setup.baudRateInHz = 10000000;
    setup.dataBits = DRV_SPI_DATA_BITS_8;
    setup.chipSelect = SYS_PORT_PIN_RD11;
    setup.clockPhase = DRV_SPI_CLOCK_PHASE_VALID_TRAILING_EDGE;
    setup.clockPolarity = DRV_SPI_CLOCK_POLARITY_IDLE_HIGH;
    setup.csPolarity = DRV_SPI_CS_POLARITY_ACTIVE_LOW;
    DRV_SPI_TransferSetup(pDrvInst->spiBus, &setup);

    DRV_SPI_TransferEventHandlerSet(pDrvInst->spiBus, spi_event_handler, 0);
    
    pDrvInst->mainState = WIFI_STATE_INITIALIZE;
    
    int count;
    for (count = 0; count < WIFI_MAX_RX_BUFFERS; count++) {
        pkt = (*pDrvInst->stackConfig.pktAllocF)(WIFI_RX_BUFFER_SIZE, WIFI_RX_BUFFER_SIZE, TCPIP_MAC_PKT_FLAG_STATIC);
        if(pkt == NULL) {
//            SYS_ASSERT(false, "ENC28J60: could not allocate packets");
            break;
        }
        pkt->ackFunc = &wifi_rx_packet_ack;
        pkt->ackParam = pDrvInst;
        TCPIP_Helper_ProtSglListTailAdd(&pDrvInst->rxFreePackets, (SGL_LIST_NODE*)pkt);
    }

    
    if(count != WIFI_MAX_RX_BUFFERS) {   // failed
        while((pkt = (TCPIP_MAC_PACKET *)(TCPIP_Helper_ProtSglListHeadRemove(&pDrvInst->rxFreePackets))) != NULL)
        {
            (*pDrvInst->stackConfig.pktFreeF)(pkt);
        }
        return false;
    }

    return true;
}


SYS_MODULE_OBJ wifi_initialize(const SYS_MODULE_INDEX index, const SYS_MODULE_INIT * const init) {
    const TCPIP_MAC_INIT * const ptr = (const TCPIP_MAC_INIT *)init;
    
    TCPIP_Helper_ProtSglListInitialize(&wifi_data.rxFreePackets);
    TCPIP_Helper_ProtSglListInitialize(&wifi_data.rxWaitingForPickupPackets);
    TCPIP_Helper_ProtSglListInitialize(&wifi_data.txPendingPackets);
    TCPIP_Helper_ProtSglListInitialize(&wifi_data.macSetRequests);

    SYS_MODULE_OBJ pDrvInst = (SYS_MODULE_OBJ) &wifi_data;


    if (pDrvInst == SYS_MODULE_OBJ_INVALID) {
        return SYS_MODULE_OBJ_INVALID;
    }
    if(!wifi_set_mac_ctl(pDrvInst, (const TCPIP_MAC_MODULE_CTRL *)ptr->macControl)) {
        return SYS_MODULE_OBJ_INVALID;
    }
    return pDrvInst;
}

SYS_STATUS wifi_status(SYS_MODULE_OBJ object) {

    struct wifi_data *data = (struct wifi_data *)object;
    
    switch (data->mainState) {
        case WIFI_STATE_INITIALIZE:
        case WIFI_STATE_RESET_WAIT:
        case WIFI_STATE_RESET_DONE:
        case WIFI_STATE_SEND_MAC:
        case WIFI_STATE_WAIT_MAC:
        case WIFI_STATE_SEND_SSID:
        case WIFI_STATE_WAIT_SSID:
        case WIFI_STATE_SEND_PSK:
        case WIFI_STATE_WAIT_PSK:
            return SYS_STATUS_UNINITIALIZED;
        case WIFI_STATE_RUN:
            return SYS_STATUS_READY;
            
        case WIFI_STATE_SEND_STATUS:
        case WIFI_STATE_WAIT_STATUS:
        case WIFI_STATE_GET_STATUS:
        case WIFI_STATE_HAVE_STATUS:
        case WIFI_STATE_SEND_RBSTAT:
        case WIFI_STATE_WAIT_RBSTAT:
        case WIFI_STATE_SEND_READ:
        case WIFI_STATE_WAIT_READ:
        case WIFI_STATE_GET_SIZE:
        case WIFI_STATE_HAVE_SIZE:
        case WIFI_STATE_GET_READ:
        case WIFI_STATE_HAVE_READ:
        case WIFI_STATE_QUEUE_READ:     
        case WIFI_STATE_SEND_WRITE:
        case WIFI_STATE_WAIT_WRITE:
            return SYS_STATUS_READY;
//            return SYS_STATUS_BUSY;
    }
    return SYS_STATUS_ERROR;
}

bool spi_complete() {
    if (spi_transfer_complete) {
        spi_transfer_complete = false;
        return true;
    }
    return false;
}

void wifi_tasks(SYS_MODULE_OBJ object) {
    struct wifi_data *data = (struct wifi_data *)object;
    uint32_t now;
    uint8_t *gather;
    TCPIP_MAC_DATA_SEGMENT *dseg;
    int len;
        
//    static uint32_t dbgts = 0;    
//    if (xTaskGetTickCount() - dbgts > 1000) {
//        dbgts = xTaskGetTickCount();
//        DBG("[%d]\r\n", data->mainState);
//    }
    
    
    switch (data->mainState) {
        case WIFI_STATE_INITIALIZE:
            ESP_EN_Set();
            ESP_EN_OutputEnable();
            ESP_EN_Clear();
            data->mainState = WIFI_STATE_RESET_WAIT;
            data->timers.stateDelay = xTaskGetTickCount();
            break;

        case WIFI_STATE_RESET_WAIT:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < 100) break;
            ESP_EN_Set();
            data->timers.stateDelay = xTaskGetTickCount();
            data->mainState = WIFI_STATE_RESET_DONE;
            break;
            
        case WIFI_STATE_RESET_DONE:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < 1000) break;
            ESP_EN_InputEnable();
            data->mainState = WIFI_STATE_SEND_MAC;
            break;
            
        case WIFI_STATE_SEND_MAC:
            data->spibuf[0] = REG_ADDMAC;
            memcpy(&data->spibuf[1], data->stackParameters.ifPhyAddress.v, sizeof(TCPIP_MAC_ADDR));
            //DBG("W");
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, sizeof(TCPIP_MAC_ADDR) + 1, &data->transfer);
            data->mainState = WIFI_STATE_WAIT_MAC;
            break;
            
        case WIFI_STATE_WAIT_MAC:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_SEND_MAC;
//                break;
//            }
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_SEND_SSID;
            }
            break;
            
        case WIFI_STATE_SEND_SSID:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->spibuf[0] = REG_SSID;
            len = strlen(wifi_settings.ssid);
            memcpy(&data->spibuf[1], wifi_settings.ssid, len);
            //DBG("W");
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, len + 1, &data->transfer);
            data->mainState = WIFI_STATE_WAIT_SSID;
            break;
            
        case WIFI_STATE_WAIT_SSID:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_SEND_SSID;
//                break;
//            }
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_SEND_PSK;
            }
            
        case WIFI_STATE_SEND_PSK:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->spibuf[0] = REG_PSK;
            len = strlen(wifi_settings.psk);
            memcpy(&data->spibuf[1], wifi_settings.psk, len);
            //DBG("W");
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, len + 1, &data->transfer);
            data->mainState = WIFI_STATE_WAIT_PSK;
            break;
            
        case WIFI_STATE_WAIT_PSK:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_SEND_PSK;
//                break;
//            }
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->mainState = WIFI_STATE_RUN;
                data->stackParameters.processFlags = TCPIP_MAC_PROCESS_FLAG_NONE;
                data->stackParameters.macType = TCPIP_MAC_TYPE_ETH;
                data->stackParameters.linkMtu = TCPIP_MAC_LINK_MTU_ETH;
                
                data->timers.queryStatus = xTaskGetTickCount();
            }
            break;
            
            
            
            
            
            
        case WIFI_STATE_RUN:
            now = xTaskGetTickCount();
            
            if ((now - data->timers.queryStatus) >= 10000) {
                data->timers.queryStatus = now;
                data->mainState = WIFI_STATE_SEND_STATUS;
                break;
            }

            if (ESP_INT_Get() == 0) {
                data->stats.rtTime = data->stats.rxTime = xTaskGetTickCount();
              //  DBG("R:%u\r\n", data->stats.rxTime);
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_SEND_RBSTAT;
                break;
            }


            if (!TCPIP_Helper_ProtSglListIsEmpty(&data->txPendingPackets)) {
                data->stats.txTime = xTaskGetTickCount();
//                DBG("W:%u\r\n", data->stats.txTime);
                data->mainState = WIFI_STATE_SEND_WRITE;
                break;
            }



            break;
            
            
            
            
            
            
            
            
        case WIFI_STATE_SEND_STATUS:

            data->spibuf[0] = REG_STATUS;            
            //DBG("W");
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, 1, &data->transfer);
            data->mainState = WIFI_STATE_WAIT_STATUS;
            break;
            
        case WIFI_STATE_WAIT_STATUS:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_SEND_STATUS;
//                break;
//            }
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_GET_STATUS;
            }
            break;
            
        case WIFI_STATE_GET_STATUS:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            //DBG("R");
            DRV_SPI_ReadTransferAdd(data->spiBus, data->phyStatus.val, sizeof(wifi_status_t), &data->transfer);
            data->mainState = WIFI_STATE_HAVE_STATUS;
            break;

        case WIFI_STATE_HAVE_STATUS:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_GET_STATUS;
//                break;
//            }
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->phyStatus.nextsize = TCPIP_Helper_ntohs(data->phyStatus.nextsize);
                data->phyStatus.rssi = TCPIP_Helper_ntohl(data->phyStatus.rssi);
                data->mainState = WIFI_STATE_RUN;

            }
            break;


            
            
            
 
            
            
            
        case WIFI_STATE_SEND_RBSTAT:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->spibuf[0] = REG_RPACKET;
            //DBG("W");
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, 1, &data->transfer);
            data->mainState = WIFI_STATE_WAIT_RBSTAT;
            break;

        case WIFI_STATE_WAIT_RBSTAT:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_SEND_READ;
//                break;
//            }
//
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_GET_SIZE;
            }
            break;
        case WIFI_STATE_GET_SIZE:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_LONG_DELAY) break;
            //DBG("R");
            DRV_SPI_ReadTransferAdd(data->spiBus, data->spibuf, 2, &data->transfer);
            data->mainState = WIFI_STATE_HAVE_SIZE;
            break;
        case WIFI_STATE_HAVE_SIZE:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_GET_SIZE;
//                break;
//            }
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->phyStatus.nextsize = (data->spibuf[0] << 8) | data->spibuf[1];
                //DBG("S:%u\r\n", data->phyStatus.nextsize);
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_SEND_READ;
            }
            break;

            
            
        case WIFI_STATE_SEND_READ:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->spibuf[0] = REG_RPACKET;
            //DBG("W");
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, 1, &data->transfer);
            data->mainState = WIFI_STATE_WAIT_READ;
            break;

        case WIFI_STATE_WAIT_READ:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_SEND_READ;
//                break;
//            }
//
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_GET_READ;
            }
            break;            
            
        case WIFI_STATE_GET_READ:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_LONG_DELAY) break;
            //DBG("R");
            DRV_SPI_ReadTransferAdd(data->spiBus, data->spibuf, data->phyStatus.nextsize + 2, &data->transfer);
            data->mainState = WIFI_STATE_HAVE_READ;
            break;
            
        case WIFI_STATE_HAVE_READ:
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_ERROR) {
//                data->mainState = WIFI_STATE_GET_READ;
//                break;
//            }
//            if (DRV_SPI_TransferStatusGet(data->transfer) == DRV_SPI_TRANSFER_EVENT_COMPLETE) {
            if (spi_complete()) {
                int l = (data->spibuf[0] << 8) | data->spibuf[1];
                if (l != data->phyStatus.nextsize) {
                    data->mainState = WIFI_STATE_RUN;
                    break;
                }
                data->mainState = WIFI_STATE_QUEUE_READ;                
            }
            break;
            
        case WIFI_STATE_QUEUE_READ:
            if (TCPIP_Helper_ProtSglListIsEmpty(&data->rxFreePackets)) {
                break;
            }
            data->pkt = (TCPIP_MAC_PACKET *)TCPIP_Helper_ProtSglListHeadRemove(&data->rxFreePackets);
            memcpy(data->pkt->pDSeg->segLoad, &data->spibuf[2], data->phyStatus.nextsize);
            data->pkt->pDSeg->segLen = data->phyStatus.nextsize;
            if (
                    (data->spibuf[2] == 0xFF) &&
                    (data->spibuf[3] == 0xFF) &&
                    (data->spibuf[4] == 0xFF) &&
                    (data->spibuf[5] == 0xFF) &&
                    (data->spibuf[6] == 0xFF) &&
                    (data->spibuf[7] == 0xFF)) {
                data->pkt->pktFlags |= TCPIP_MAC_PKT_FLAG_BCAST;                
            } else if ((data->spibuf[2] & 0x01) == 0x01) {
                data->pkt->pktFlags |= TCPIP_MAC_PKT_FLAG_MCAST;
            } else { 
                data->pkt->pktFlags |= TCPIP_MAC_PKT_FLAG_UNICAST;
            }
            TCPIP_Helper_ProtSglListTailAdd(&data->rxWaitingForPickupPackets, (SGL_LIST_NODE *)data->pkt);
            wifi_set_event(data, TCPIP_MAC_EV_RX_DONE);
            data->stats.rxPackets++;
            data->mainState = WIFI_STATE_RUN;
//            DBG("r:%u (%u) = %u\r\n", xTaskGetTickCount(), xTaskGetTickCount() - data->stats.rxTime, xTaskGetTickCount() - data->stats.rtTime);
            break;
            
            
            
            
            
            
            
            
        case WIFI_STATE_SEND_WRITE:
            data->pkt = (TCPIP_MAC_PACKET *)TCPIP_Helper_ProtSglListHeadRemove(&data->txPendingPackets);
            data->spibuf[0] = REG_WPACKET;
            gather = data->spibuf + 1;
            dseg = data->pkt->pDSeg;
            len = 0;
            while (dseg) {
                memcpy(gather, dseg->segLoad, dseg->segLen);
                gather += dseg->segLen;
                len += dseg->segLen;
                dseg = dseg->next;
            }
//            DBG("PKTLEN:%d\r\n", len+1);
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, len+1, &data->transfer);
            data->mainState = WIFI_STATE_WAIT_WRITE;
            break;
            
        case WIFI_STATE_WAIT_WRITE:
            if (spi_complete()) {

            //            
//            
//            spi_res = DRV_SPI_TransferStatusGet(data->transfer);
//            
//            switch (spi_res) {
//                case DRV_SPI_TRANSFER_EVENT_ERROR:
//                    DBG("DRV_SPI_TRANSFER_EVENT_ERROR\r\n");
//                    data->mainState = WIFI_STATE_RUN;
//                    break;
//                case DRV_SPI_TRANSFER_EVENT_COMPLETE:
                    data->currentEvents |= TCPIP_MAC_EV_TX_DONE;
                    if (!data->phyStatus.link) {
                        data->pkt->ackRes = (int8_t)TCPIP_MAC_PKT_ACK_LINK_DOWN;
                    } else {
                        data->pkt->ackRes = (int8_t)TCPIP_MAC_PKT_ACK_TX_OK;
                    }

                    data->pkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
                    data->stackConfig.pktAckF(data->pkt, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_MODULE_MAC_EXTERNAL);
                    data->mainState = WIFI_STATE_RUN;
//                    DBG("w:%u (%u)\r\n", xTaskGetTickCount(), xTaskGetTickCount() - data->stats.txTime);
//                    break;
//                case DRV_SPI_TRANSFER_EVENT_PENDING:
//                    break;
//                case DRV_SPI_TRANSFER_EVENT_HANDLE_EXPIRED:
//                    DBG("DRV_SPI_TRANSFER_EVENT_HANDLE_EXPIRED\r\n");
//                    data->mainState = WIFI_STATE_RUN;
//                    break;
//                case DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID:
//                    DBG("DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID\r\n");
//                    data->mainState = WIFI_STATE_RUN;
//                    break;
            }
            break;
    }
}

DRV_HANDLE wifi_open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT intent) {
    return (DRV_HANDLE)&wifi_data;
}

void wifi_close(DRV_HANDLE handle) {
}

bool wifi_link_check(DRV_HANDLE hMac) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    return data->phyStatus.link;
}


TCPIP_MAC_RES wifi_add_mac(DRV_HANDLE hMac, const TCPIP_MAC_ADDR* DestMACAddr) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    struct mac_request *req = malloc(sizeof(struct mac_request));
    memcpy(req->addr.v, DestMACAddr->v, 6);
    TCPIP_Helper_ProtSglListTailAdd(&data->macSetRequests, (SGL_LIST_NODE*)req);
    return TCPIP_MAC_RES_OK;
}
bool wifi_power_mode(DRV_HANDLE hMac, TCPIP_MAC_POWER_MODE pwrMode) {
    return false;
}

TCPIP_MAC_RES wifi_tx(DRV_HANDLE hMac, TCPIP_MAC_PACKET * pkt) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    if (data == NULL) {
        return TCPIP_MAC_RES_OP_ERR;
    }

    TCPIP_Helper_ProtSglListTailAdd(&data->txPendingPackets, (SGL_LIST_NODE*)pkt);
    pkt->pktFlags |= TCPIP_MAC_PKT_FLAG_QUEUED;
    return TCPIP_MAC_RES_OK;
}

TCPIP_MAC_PACKET* wifi_rx(DRV_HANDLE hMac, TCPIP_MAC_RES* pRes, TCPIP_MAC_PACKET_RX_STAT* pPktStat) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    if (data == NULL) {
        return NULL;
    }
    if (TCPIP_Helper_ProtSglListIsEmpty(&data->rxWaitingForPickupPackets)) {
        return NULL;
    }
    TCPIP_MAC_PACKET *pkt = (TCPIP_MAC_PACKET *)TCPIP_Helper_ProtSglListHeadRemove(&data->rxWaitingForPickupPackets);
    //decode_packet(pkt->pDSeg->segLoad, pkt->pDSeg->segLen);

    return pkt;
}

TCPIP_MAC_RES wifi_process(DRV_HANDLE hMac) {
    return TCPIP_MAC_RES_OP_ERR;
}

TCPIP_MAC_RES wifi_get_statistics(DRV_HANDLE hMac, TCPIP_MAC_RX_STATISTICS* pRxStatistics, TCPIP_MAC_TX_STATISTICS* pTxStatistics) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    if (data == NULL) return TCPIP_MAC_RES_OP_ERR;

    if (pRxStatistics != NULL)
    {
        pRxStatistics->nRxOkPackets = data->stats.rxPackets;
        pRxStatistics->nRxPendBuffers = WIFI_MAX_RX_BUFFERS - TCPIP_Helper_ProtSglListCount(&data->rxFreePackets);
        pRxStatistics->nRxSchedBuffers = TCPIP_Helper_ProtSglListCount(&data->rxFreePackets);
        pRxStatistics->nRxErrorPackets = 0xffffffffUL;      //not implemented
        pRxStatistics->nRxFragmentErrors = 0xffffffffUL;    //not implemented
        pRxStatistics->nRxBuffNotAvailable = 0xffffffffUL;  //not implemented
    }
    if (pTxStatistics != NULL)
    {
        pTxStatistics->nTxOkPackets = data->stats.txPackets;
        pTxStatistics->nTxPendBuffers = TCPIP_Helper_ProtSglListCount(&data->txPendingPackets);
        pTxStatistics->nTxErrorPackets = 0xffffffffUL; //not implemented
        pTxStatistics->nTxQueueFull = 0xffffffffUL; //not implemented                                                                                   
    }                                                                                                                                                   
                                                                                                                                                        
    return TCPIP_MAC_RES_OK;                                                                                                                            
}

TCPIP_MAC_RES wifi_get_parameters(DRV_HANDLE hMac, TCPIP_MAC_PARAMETERS* pMacParams) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    if (data == NULL) return TCPIP_MAC_RES_OP_ERR;
    (void)memcpy(pMacParams, &data->stackParameters, sizeof(TCPIP_MAC_PARAMETERS));

    pMacParams->linkMtu = 1500;
    pMacParams->processFlags = TCPIP_MAC_PROCESS_FLAG_RX;
    pMacParams->macType = TCPIP_MAC_TYPE_WLAN;

    return TCPIP_MAC_RES_OK;
}

TCPIP_MAC_RES wifi_register_get_statistics(DRV_HANDLE hMac, TCPIP_MAC_STATISTICS_REG_ENTRY* pRegEntries, size_t nEntries, size_t* pHwEntries) {
    return TCPIP_MAC_RES_OP_ERR;
}
size_t wifi_get_config(DRV_HANDLE hMac, void* configBuff, size_t buffSize, size_t* pConfigSize) {
    return 0;
}

bool wifi_set_event_mask(DRV_HANDLE hMac, TCPIP_MAC_EVENT macEvents, bool enable) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    if (data == NULL) return false;
    data->eventMask = macEvents;
    return true;
}

bool wifi_event_acknowledge(DRV_HANDLE hMac, TCPIP_MAC_EVENT macEvents) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    if (data == NULL) return false;

    data->currentEvents &= ~macEvents;

    if (!TCPIP_Helper_ProtSglListIsEmpty(&data->rxWaitingForPickupPackets))
    {
        data->currentEvents |= data->eventMask & TCPIP_MAC_EV_RX_DONE;
    }

    return true;
}

TCPIP_MAC_EVENT wifi_get_pending_events(DRV_HANDLE hMac) {
    struct wifi_data *data = (struct wifi_data *)hMac;
    if (data == NULL) return false;
    return data->currentEvents;
}

TCPIP_MAC_OBJECT wifiMacObject = {
    .macId = TCPIP_MODULE_MAC_EXTERNAL,
    .macType = TCPIP_MAC_TYPE_WLAN,
    .macName = "ESP32",
    .MAC_Initialize =                   &wifi_initialize,
    .MAC_Deinitialize =                 NULL,
    .MAC_Reinitialize =                 NULL,
    .MAC_Status =                       &wifi_status,
    .MAC_Tasks =                        &wifi_tasks,
    .MAC_Open =                         &wifi_open,
    .MAC_Close =                        &wifi_close,
    .MAC_LinkCheck =                    &wifi_link_check,
    .MAC_RxFilterHashTableEntrySet =    &wifi_add_mac,
    .MAC_PowerMode =                    &wifi_power_mode,
    .MAC_PacketTx =                     &wifi_tx,
    .MAC_PacketRx =                     &wifi_rx,
    .MAC_Process =                      &wifi_process,
    .MAC_StatisticsGet =                &wifi_get_statistics,
    .MAC_ParametersGet =                &wifi_get_parameters,
    .MAC_RegisterStatisticsGet =        &wifi_register_get_statistics,
    .MAC_ConfigGet =                    &wifi_get_config,
    .MAC_EventMaskSet =                 &wifi_set_event_mask,
    .MAC_EventAcknowledge =             &wifi_event_acknowledge,
    .MAC_EventPendingGet =              &wifi_get_pending_events
};