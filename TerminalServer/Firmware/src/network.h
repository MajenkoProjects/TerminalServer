#ifndef _NETWORK_H
#define _NETWORK_H

#include "tcpip/tcpip.h"
#include "command.h"
#include "port.h"

#define SETTINGS_ETHERNET_IP        0x01
#define SETTINGS_ETHERNET_NETMASK   0x02
#define SETTINGS_ETHERNET_GATEWAY   0x03
#define SETTINGS_ETHERNET_PRIDNS    0x04
#define SETTINGS_ETHERNET_SECDNS    0x05
#define SETTINGS_ETHERNET_FLAGS     0x06
#define SETTINGS_ETHERNET_MAC       0x07

#define SETTINGS_WIFI_IP            0x11
#define SETTINGS_WIFI_NETMASK       0x12
#define SETTINGS_WIFI_GATEWAY       0x13
#define SETTINGS_WIFI_PRIDNS        0x14
#define SETTINGS_WIFI_SECDNS        0x15
#define SETTINGS_WIFI_FLAGS         0x16
#define SETTINGS_WIFI_MAC           0x17
#define SETTINGS_WIFI_SSID          0x18
#define SETTINGS_WIFI_PSK           0x19

struct ethernet_settings {
    char macaddr[18];
    char ip[16];
    char netmask[16];
    char gateway[16];
    char pridns[16];
    char secdns[16];
    uint16_t flags;
    uint8_t timeout;
};

struct wifi_settings {
    char macaddr[18];
    char ip[16];
    char netmask[16];
    char gateway[16];
    char pridns[16];
    char secdns[16];
    uint16_t flags;
    uint8_t timeout;
    char ssid[33];
    char psk[64];
};

extern struct ethernet_settings ethernet_settings;
extern struct wifi_settings wifi_settings;
extern const TCPIP_STACK_MODULE_CONFIG TCPIP_STACK_MODULE_CONFIG_TBL[];
extern const size_t TCPIP_STACK_MODULE_CONFIG_TBL_SIZE;

extern void ethernet_init_defaults();
extern void ethernet_boot();
extern void ethernet_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data);
extern void print_network_settings(struct port *port);
extern void add_mac(int index, TCPIP_MAC_ADDR *mac);



extern COMMAND(ethernet_set_ip);
extern COMMAND(ethernet_set_gateway);
extern COMMAND(ethernet_define_mac_address);
extern COMMAND(ethernet_define_ip);
extern COMMAND(ethernet_define_subnet);
extern COMMAND(ethernet_define_gateway);
extern COMMAND(ethernet_define_pridns);
extern COMMAND(ethernet_define_secdns);
extern COMMAND(ethernet_define_dhcp_enabled);
extern COMMAND(ethernet_define_dhcp_disabled);

extern COMMAND(wifi_set_ip);
extern COMMAND(wifi_set_gateway);
extern COMMAND(wifi_define_mac_address);
extern COMMAND(wifi_define_ip);
extern COMMAND(wifi_define_subnet);
extern COMMAND(wifi_define_gateway);
extern COMMAND(wifi_define_pridns);
extern COMMAND(wifi_define_secdns);
extern COMMAND(wifi_define_dhcp_enabled);
extern COMMAND(wifi_define_dhcp_disabled);
extern COMMAND(wifi_define_ssid);
extern COMMAND(wifi_define_psk);

#endif