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
    SPI_INIT                    = 0,
    SPI_HW_RESET_START          = 1,
    SPI_HW_RESET_WAIT           = 2,
    SPI_HW_RESET_END            = 3,
    SPI_SEND_RESET              = 4,
    SPI_SEND_MAC                = 5,
    SPI_SEND_SSID               = 6,
    SPI_SEND_PSK                = 7,
    SPI_ASK_STATUS              = 8,
    SPI_GET_STATUS              = 9,
    SPI_IDLE                    = 10,
    SPI_GET_PLEN                = 11,
    SPI_RX_PLEN                 = 12,
    SPI_ASK_PACKET              = 13,
    SPI_GET_PACKET              = 14,
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


#define WIFI_MAX_RX_BUFFERS 4
#define WIFI_RX_BUFFER_SIZE SPI_MAX_SIZE

enum wifi_main_state {
    WIFI_STATE_INITIALIZE   = 0,
    WIFI_STATE_RESET_WAIT   = 1,
    WIFI_STATE_RESET_DONE   = 2,
    WIFI_STATE_SEND_MAC     = 3,
    WIFI_STATE_WAIT_MAC     = 4,
    WIFI_STATE_SEND_SSID    = 5,
    WIFI_STATE_WAIT_SSID    = 6,
    WIFI_STATE_SEND_PSK     = 7,
    WIFI_STATE_WAIT_PSK     = 8,
    WIFI_STATE_RUN          = 9,
    
    WIFI_STATE_SEND_STATUS  = 10,
    WIFI_STATE_WAIT_STATUS  = 11,
    WIFI_STATE_GET_STATUS   = 12,
    WIFI_STATE_HAVE_STATUS  = 13,
    

    WIFI_STATE_START_READ   = 14,
    WIFI_STATE_SEND_READ    = 15,
    WIFI_STATE_WAIT_READ    = 16,
    WIFI_STATE_GET_READ     = 17,
    WIFI_STATE_HAVE_READ    = 18,
    
    WIFI_STATE_QUEUE_READ   = 19,
    
    WIFI_STATE_START_WRITE  = 20,
    WIFI_STATE_SEND_WRITE   = 21,
    WIFI_STATE_WAIT_WRITE   = 22
    
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
    enum wifi_main_state afterStatus;
    bool stackConfigReady;
    
    DRV_SPI_TRANSFER_HANDLE transfer;
    uint8_t spibuf[SPI_MAX_SIZE];
    int transferSize;

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

    setup.baudRateInHz = WIFI_SPI_BITRATE;
    setup.dataBits = DRV_SPI_DATA_BITS_8;
    setup.chipSelect = SYS_PORT_PIN_RD11;
    setup.clockPhase = DRV_SPI_CLOCK_PHASE_VALID_TRAILING_EDGE;
    setup.clockPolarity = DRV_SPI_CLOCK_POLARITY_IDLE_HIGH;
    setup.csPolarity = DRV_SPI_CS_POLARITY_ACTIVE_LOW;
    DRV_SPI_TransferSetup(pDrvInst->spiBus, &setup);
    
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
        case WIFI_STATE_START_READ:
        case WIFI_STATE_SEND_READ:
        case WIFI_STATE_WAIT_READ:
        case WIFI_STATE_GET_READ:
        case WIFI_STATE_HAVE_READ:
        case WIFI_STATE_QUEUE_READ:     
        case WIFI_STATE_START_WRITE:
        case WIFI_STATE_SEND_WRITE:
        case WIFI_STATE_WAIT_WRITE:
            return SYS_STATUS_READY;
//            return SYS_STATUS_BUSY;
    }
    return SYS_STATUS_ERROR;
}

enum spi_status {
    SPI_STATUS_ERROR,
    SPI_STATUS_COMPLETE,
    SPI_STATUS_PENDING
};


enum spi_status get_spi_state(struct wifi_data *data, const char *place) {
    DRV_SPI_TRANSFER_EVENT spi_res = DRV_SPI_TransferStatusGet(data->transfer);
    switch (spi_res) {
        case DRV_SPI_TRANSFER_EVENT_ERROR:
            DBG("%s DRV_SPI_TRANSFER_EVENT_ERROR\r\n", place);
            return SPI_STATUS_ERROR;
        case DRV_SPI_TRANSFER_EVENT_COMPLETE:
            return SPI_STATUS_COMPLETE;
        case DRV_SPI_TRANSFER_EVENT_PENDING:
            return SPI_STATUS_PENDING;
            break;
        case DRV_SPI_TRANSFER_EVENT_HANDLE_EXPIRED:
            DBG("%s DRV_SPI_TRANSFER_EVENT_HANDLE_EXPIRED\r\n", place);
            return SPI_STATUS_ERROR;
        case DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID:
            DBG("%s DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID\r\n", place);
            return SPI_STATUS_ERROR;
    }
    DBG("%s UNKNOWN %d\r\n", place, spi_res);
    return SPI_STATUS_ERROR;
}

void wifi_tasks(SYS_MODULE_OBJ object) {
    struct wifi_data *data = (struct wifi_data *)object;
    uint32_t now;
    uint8_t *gather;
    TCPIP_MAC_DATA_SEGMENT *dseg;
    int len = 0;
        
    
    enum spi_status spi_res;
    
//    static uint32_t dbgts = 0;    
//    if (xTaskGetTickCount() - dbgts > 1000) {
//        dbgts = xTaskGetTickCount();
//        DBG("[%d]\r\n", data->mainState);
//    }
//    
    
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
            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, sizeof(TCPIP_MAC_ADDR) + 1, &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_WAIT_MAC;
            break;
            
        case WIFI_STATE_WAIT_MAC:
            spi_res = get_spi_state(data, "WIFI_STATE_WAIT_MAC");
            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_SEND_MAC;
                break;
            }
            if (spi_res == SPI_STATUS_COMPLETE) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_SEND_SSID;
            }
            break;
            
        case WIFI_STATE_SEND_SSID:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->spibuf[0] = REG_SSID;
            len = strlen(wifi_settings.ssid);
            memcpy(&data->spibuf[1], wifi_settings.ssid, len);
            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, len + 1, &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_WAIT_SSID;
            break;
            
        case WIFI_STATE_WAIT_SSID:
            spi_res = get_spi_state(data, "WIFI_STATE_WAIT_SSID");
            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_SEND_SSID;
                break;
            }
            if (spi_res == SPI_STATUS_COMPLETE) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_SEND_PSK;
            }
            break;
            
        case WIFI_STATE_SEND_PSK:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->spibuf[0] = REG_PSK;
            len = strlen(wifi_settings.psk);
            memcpy(&data->spibuf[1], wifi_settings.psk, len);
            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, len + 1, &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_WAIT_PSK;
            break;
            
        case WIFI_STATE_WAIT_PSK:
            spi_res = get_spi_state(data, "WIFI_STATE_WAIT_PSK");
            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_SEND_PSK;
                break;
            }
            if (spi_res == SPI_STATUS_COMPLETE) {
                data->mainState = WIFI_STATE_RUN;
                data->stackParameters.processFlags = TCPIP_MAC_PROCESS_FLAG_NONE;
                data->stackParameters.macType = TCPIP_MAC_TYPE_ETH;
                data->stackParameters.linkMtu = TCPIP_MAC_LINK_MTU_ETH;
                
                data->timers.queryStatus = xTaskGetTickCount();
            }
            break;
            
            
            
            
            
            
        case WIFI_STATE_RUN:
            now = xTaskGetTickCount();
            
            if ((now - data->timers.queryStatus) >= 1000) {
                data->timers.queryStatus = now;
                data->afterStatus = WIFI_STATE_RUN;
                data->mainState = WIFI_STATE_SEND_STATUS;
                break;
            }

            if (ESP_INT_Get() == 0) {
                data->stats.rtTime = data->stats.rxTime = xTaskGetTickCount();
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_START_READ;
                break;
            }


            if (!TCPIP_Helper_ProtSglListIsEmpty(&data->txPendingPackets)) {
                data->stats.txTime = xTaskGetTickCount();
                data->mainState = WIFI_STATE_START_WRITE;
                break;
            }



            break;
            
            
            
            
            
            
            
            
        case WIFI_STATE_SEND_STATUS:
            data->spibuf[0] = REG_STATUS;            
            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, 1, &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_WAIT_STATUS;
            break;
            
        case WIFI_STATE_WAIT_STATUS:
            spi_res = get_spi_state(data, "WIFI_STATE_WAIT_STATUS");
            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_SEND_STATUS;
                break;
            }
            if (spi_res == SPI_STATUS_COMPLETE) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_GET_STATUS;
            }
            break;
            
        case WIFI_STATE_GET_STATUS:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_ReadTransferAdd(data->spiBus, data->phyStatus.val, sizeof(wifi_status_t), &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_HAVE_STATUS;
            break;

        case WIFI_STATE_HAVE_STATUS:
            spi_res = get_spi_state(data, "WIFI_STATE_HAVE_STATUS");
            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_GET_STATUS;
                break;
            }
            if (spi_res == SPI_STATUS_COMPLETE) {
                data->phyStatus.nextsize = TCPIP_Helper_ntohs(data->phyStatus.nextsize);
                data->phyStatus.rssi = TCPIP_Helper_ntohl(data->phyStatus.rssi);
                data->mainState = data->afterStatus;
            }
            break;


            
            
            
 
            
            
            
        case WIFI_STATE_START_READ:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->mainState = WIFI_STATE_SEND_STATUS;
            data->afterStatus = WIFI_STATE_SEND_READ;
            break;


            
            
        case WIFI_STATE_SEND_READ:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_SHORT_DELAY) break;
            data->spibuf[0] = REG_RPACKET;
            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, 1, &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_WAIT_READ;
            break;

        case WIFI_STATE_WAIT_READ:
            spi_res = get_spi_state(data, "WIFI_STATE_WAIT_READ");
            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_SEND_READ;
                break;
            }

            if (spi_res == SPI_STATUS_COMPLETE) {
                data->timers.stateDelay = xTaskGetTickCount();
                data->mainState = WIFI_STATE_GET_READ;
            }
            break;            
            
        case WIFI_STATE_GET_READ:
            if ((xTaskGetTickCount() - data->timers.stateDelay) < MAC_LONG_DELAY) break;
            if ((data->phyStatus.nextsize + 2) > WIFI_RX_BUFFER_SIZE) {
                DBG("Bad size: %d\r\n", data->phyStatus.nextsize);
                data->mainState = WIFI_STATE_RUN;
                break;
            } 
            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_ReadTransferAdd(data->spiBus, data->spibuf, data->phyStatus.nextsize + 2, &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_HAVE_READ;
            break;
            
        case WIFI_STATE_HAVE_READ:
            spi_res = get_spi_state(data, "WIFI_STATE_HAVE_READ");
            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_GET_READ;
                break;
            }
            if (spi_res == SPI_STATUS_COMPLETE) {
                int l = (data->spibuf[0] << 8) | data->spibuf[1];
                if (l != data->phyStatus.nextsize) {
                    DBG("X");
                    data->mainState = WIFI_STATE_RUN;
                    break;
                }
                data->mainState = WIFI_STATE_QUEUE_READ;                
            }
            break;
            
        case WIFI_STATE_QUEUE_READ:
            if (TCPIP_Helper_ProtSglListIsEmpty(&data->rxFreePackets)) {
                DBG("F");
                break;
            }
            data->pkt = (TCPIP_MAC_PACKET *)TCPIP_Helper_ProtSglListHeadRemove(&data->rxFreePackets);
            
            len = data->phyStatus.nextsize;
            gather = &data->spibuf[2];
            dseg = data->pkt->pDSeg;
            
            while ((len > 0) && (dseg != NULL)) {
                int space = dseg->segSize;
                if (space > len) space = len;
                memcpy(dseg->segLoad, gather, space);
                dseg->segLen = space;
                len -= space;
                gather += space;
                dseg = dseg->next;
            }
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
            break;
            
            
            
            
            
            
            
            
            
            

        case WIFI_STATE_START_WRITE:            
            data->pkt = (TCPIP_MAC_PACKET *)TCPIP_Helper_ProtSglListHeadRemove(&data->txPendingPackets);
            data->spibuf[0] = REG_WPACKET;
            gather = data->spibuf + 1;
            dseg = data->pkt->pDSeg;
            data->transferSize = 0;
            while ((data->transferSize < SPI_MAX_SIZE) && (dseg != NULL)) {
                memcpy(gather, dseg->segLoad, dseg->segLen);
                gather += dseg->segLen;
                data->transferSize += dseg->segLen;
                dseg = dseg->next;
            }
            data->mainState = WIFI_STATE_SEND_WRITE;
            break;
            
        case WIFI_STATE_SEND_WRITE:

            data->transfer = DRV_SPI_TRANSFER_HANDLE_INVALID;
            DRV_SPI_WriteTransferAdd(data->spiBus, data->spibuf, data->transferSize+1, &data->transfer);
            if (data->transfer == DRV_SPI_TRANSFER_HANDLE_INVALID) break;
            data->mainState = WIFI_STATE_WAIT_WRITE;
            break;
            
        case WIFI_STATE_WAIT_WRITE:           
            spi_res = get_spi_state(data, "WIFI_STATE_WAIT_WRITE");

            if (spi_res == SPI_STATUS_ERROR) {
                data->mainState = WIFI_STATE_RUN;
                break;
            }

            if (spi_res == SPI_STATUS_COMPLETE) {
                data->currentEvents |= TCPIP_MAC_EV_TX_DONE;
                if (!data->phyStatus.link) {
                    data->pkt->ackRes = (int8_t)TCPIP_MAC_PKT_ACK_LINK_DOWN;
                } else {
                    data->pkt->ackRes = (int8_t)TCPIP_MAC_PKT_ACK_TX_OK;
                }

                data->pkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
                data->stackConfig.pktAckF(data->pkt, TCPIP_MAC_PKT_ACK_TX_OK, TCPIP_MODULE_MAC_EXTERNAL);
                data->mainState = WIFI_STATE_RUN;
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