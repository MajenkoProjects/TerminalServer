#ifndef _WIFI_BRIDGE_H
#define _WIFI_BRIDGE_H

typedef struct {
    uint16_t length;
    uint8_t *payload;
} l2_packet_t;


#define SPI_MAX_SIZE (1536 + 2)

typedef struct {
    union {
        uint8_t val[6];
        struct {
            uint8_t state;
            uint8_t link;
            int32_t rssi;
            uint8_t pending;
            uint16_t nextsize;
        } __attribute__((packed));
    } __attribute((packed));
} __attribute((packed)) wifi_status_t;

enum registers {
    REG_RESET = 0x80,
    REG_SSID,
    REG_PSK,
    REG_STATUS,
    REG_RBSTAT,
    REG_RPACKET,
    REG_WPACKET,
    REG_ADDMAC,
    REG_DELMAC,
    REG_NOP=0xFF
};

enum wifi_state {
    WIFI_IDLE = 0,
    WIFI_CONNECT,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_DISCONNECTED
};

#endif
