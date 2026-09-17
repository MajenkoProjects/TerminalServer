#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "app.h"
#include "port.h"
#define SETTINGS_MAGIC 0x04546942UL

#define SETTINGS_DHCP_ENABLED           0x00000001UL
#define SETTINGS_RTSCTS_ENABLED         0x00000002UL
#define SETTINGS_XONXOFF_ENABLED        0x00000004UL


// Settings are stored in blocks of 128 bytes (1 flash page), where each
// block starts with four bytes defining that parameter:
// 
//    Byte 1: the module ID number
//    Byte 2: the parameter ID number
//    Byte 3: the device index (or 0 if no index)
//    Byte 4: the data length
//
// If byte 1 is 0xFF then this is an empty (unused) block of memory. 
// The remaining 124 bytes are the contents of the parameter.

#define MODULE_SYSTEM               0x00
#define MODULE_UART                 0x01
#define MODULE_USB                  0x02
#define MODULE_SD                   0x03
#define MODULE_ETHERNET             0x04
#define MODULE_WIFI                 0x05
#define MODULE_PORT                 0x06

#define SETTINGS_TYPE_BYTE          0x01
#define SETTINGS_TYPE_SHORT         0x02
#define SETTINGS_TYPE_LONG          0x03
#define SETTINGS_TYPE_STRING        0x04

#define SETTINGS_SYSTEM_NAME        0x01
#define SETTINGS_SYSTEM_DOMAIN      0x02


#define SETTINGS_WIFI_IP            0x01
#define SETTINGS_WIFI_NETMASK       0x02
#define SETTINGS_WIFI_GATEWAY       0x03
#define SETTINGS_WIFI_PRIDNS        0x04
#define SETTINGS_WIFI_SECDNS        0x05
#define SETTINGS_WIFI_FLAGS         0x06
#define SETTINGS_WIFI_SSID          0x07
#define SETTINGS_WIFI_PSK           0x08

#define SETTINGS_UART_NAME          0x01
#define SETTINGS_UART_BAUD          0x02
#define SETTINGS_UART_FLAGS         0x03

#define SETTINGS_USB_NAME           0x01

struct setting {
    uint8_t module;
    uint8_t parameter;
    uint8_t index;
    uint8_t length;
    uint8_t data[124];
} __attribute__((packed));

struct system_settings {
    char nodename[33];
    char domain[64];
    char password[32];
};

extern void load_settings();
extern void setting_set(uint8_t module, uint8_t parameter, uint8_t index, uint8_t len, uint8_t *data);
extern void settings_erase();
extern void settings_dump(struct port *port);
extern void system_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data);
extern void print_system_settings(struct port *port);
extern void system_init_defaults();
extern struct system_settings system_settings;

extern COMMAND(system_define_name);
extern COMMAND(system_define_domain);
extern COMMAND(system_factory_reset);

#endif