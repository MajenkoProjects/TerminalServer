#include <ctype.h>

#include "driver/enc28j60/drv_enc28j60.h"
#include "network.h"
#include "definitions.h"
#include "settings.h"
#include "util.h"
#include "wifi.h"


struct ethernet_settings ethernet_settings;
struct wifi_settings wifi_settings;

#include "network_config.h"

void ethernet_boot() {
    port_printf(CONSOLE, "Ethernet MAC Address: %s\r\n", ethernet_settings.macaddr);
    CONSOLE->fn_flush(CONSOLE);
    port_printf(CONSOLE, "WiFi MAC Address:     %s\r\n", wifi_settings.macaddr);
    CONSOLE->fn_flush(CONSOLE);
    port_printf(CONSOLE, "Initializing network.\r\n");
    CONSOLE->fn_flush(CONSOLE);
    sysObj.netPres = NET_PRES_Initialize(0, (SYS_MODULE_INIT*)&netPresInitData);
    sysObj.tcpip = TCPIP_STACK_Init();

    
    return;
    const TCPIP_NET_HANDLE *handle = TCPIP_STACK_NetHandleGet("eth0");
 
    int tries = ethernet_settings.timeout * 2;
    while ((!TCPIP_STACK_NetIsLinked(handle)) && (tries > 0)) {
            vTaskDelay(500);
            port_printf(CONSOLE, ".");
            CONSOLE->fn_flush(CONSOLE);
            tries--;
    } 
    port_printf(CONSOLE, "\r\n");
    CONSOLE->fn_flush(CONSOLE);

    if (!TCPIP_STACK_NetIsLinked(handle)) {
        port_printf(CONSOLE, "No network link detected. Giving up.\r\n");
        CONSOLE->fn_flush(CONSOLE);
        return;
    }

    // Wait for DHCP address
    uint32_t ts = xTaskGetTickCount();
    if (ethernet_settings.flags & TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON) {
        port_printf(CONSOLE, "Waiting for DHCP address...");
        CONSOLE->fn_flush(CONSOLE);
        while (!TCPIP_STACK_NetIsReady(handle)) {
            vTaskDelay(500);
            if (xTaskGetTickCount() - ts >= 500) {
                ts = xTaskGetTickCount();
                port_printf(CONSOLE, ".");            
                CONSOLE->fn_flush(CONSOLE);
            }
        }
        port_printf(CONSOLE, "\r\n");
        CONSOLE->fn_flush(CONSOLE);
    }
    
    char ip[16];
    ip2str(TCPIP_STACK_NetAddress(handle), ip);
    
    char gw[16];
    ip2str(TCPIP_STACK_NetAddressGateway(handle), gw);
    port_printf(CONSOLE, "IP Address: %-15s Gateway: %s\r\n", ip, gw);
    CONSOLE->fn_flush(CONSOLE);

}

void ethernet_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data) {
    switch (parameter) {
        case SETTINGS_ETHERNET_MAC:
            strncpy(ethernet_settings.macaddr, (char *)data, 17);
            break;
        case SETTINGS_ETHERNET_IP:
            strncpy(ethernet_settings.ip, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_NETMASK:
            strncpy(ethernet_settings.netmask, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_GATEWAY:
            strncpy(ethernet_settings.gateway, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_PRIDNS:
            strncpy(ethernet_settings.pridns, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_SECDNS:
            strncpy(ethernet_settings.secdns, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_FLAGS:
            ethernet_settings.flags = (*(uint16_t *)data) | TCPIP_NETWORK_CONFIG_MULTICAST_ON;
            break;
            
        case SETTINGS_WIFI_MAC:
            strncpy(wifi_settings.macaddr, (char *)data, 17);
            break;
        case SETTINGS_WIFI_IP:
            strncpy(wifi_settings.ip, (char *)data, length);
            break;
        case SETTINGS_WIFI_NETMASK:
            strncpy(wifi_settings.netmask, (char *)data, length);
            break;
        case SETTINGS_WIFI_GATEWAY:
            strncpy(wifi_settings.gateway, (char *)data, length);
            break;
        case SETTINGS_WIFI_PRIDNS:
            strncpy(wifi_settings.pridns, (char *)data, length);
            break;
        case SETTINGS_WIFI_SECDNS:
            strncpy(wifi_settings.secdns, (char *)data, length);
            break;
        case SETTINGS_WIFI_FLAGS:
            wifi_settings.flags = (*(uint16_t *)data) | TCPIP_NETWORK_CONFIG_MULTICAST_ON;
            break;
        case SETTINGS_WIFI_SSID:
            memset(wifi_settings.ssid, 0, 33);
            memcpy(wifi_settings.ssid, data, length < 33 ? length : 32);
            break;
        case SETTINGS_WIFI_PSK:
            memset(wifi_settings.psk, 0, 64);
            memcpy(wifi_settings.psk, data, length < 64 ? length : 63);
            break;
    }
}

void ethernet_init_defaults() {
    snprintf(ethernet_settings.macaddr, 19, "DE:AD:BE:EF:%02X:%02X", (DEVCFG3bits.USERID >> 8) & 0xFF, DEVCFG3bits.USERID & 0xFF);
    strcpy(ethernet_settings.ip, "192.168.1.100");
    strcpy(ethernet_settings.netmask, "255.255.255.0");
    strcpy(ethernet_settings.gateway, "192.168.1.1");
    strcpy(ethernet_settings.pridns, "8.8.8.8");
    strcpy(ethernet_settings.secdns, "8.8.4.4");
    ethernet_settings.flags = (TCPIP_NETWORK_CONFIG_MULTICAST_ON | TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC);
    ethernet_settings.timeout = 10;

    snprintf(wifi_settings.macaddr, 19, "BA:BE:FA:CE:%02X:%02X", (DEVCFG3bits.USERID >> 8) & 0xFF, DEVCFG3bits.USERID & 0xFF);
    strcpy(wifi_settings.ip, "192.168.2.100");
    strcpy(wifi_settings.netmask, "255.255.255.0");
    strcpy(wifi_settings.gateway, "192.168.2.1");
    strcpy(wifi_settings.pridns, "8.8.8.8");
    strcpy(wifi_settings.secdns, "8.8.4.4");
    wifi_settings.flags = (TCPIP_NETWORK_CONFIG_MULTICAST_ON | TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC);
    wifi_settings.timeout = 10;
    memset(wifi_settings.ssid, 0, 33);
    memset(wifi_settings.psk, 0, 64);

}

void print_network_settings(struct port *port) {
    const TCPIP_NET_HANDLE *eth0 = TCPIP_STACK_NetHandleGet("eth0");


    port_printf(port, "Ethernet settings:\r\n");
    port_printf(port, "   MAC Address: %s\r\n", ethernet_settings.macaddr);
 
    port_printf(port, "   Link: %s\r\n", 
            TCPIP_STACK_NetIsLinked(eth0) ? "Connected" : "Disconnected"
            );
    
    port_printf(port, "Configured:\r\n");
    port_printf(port, "   IP Address:  %-15s   Netmask:       %-15s\r\n", ethernet_settings.ip, ethernet_settings.netmask);
    port_printf(port, "   Gateway:     %-15s   DNS:           %s/%s\r\n", ethernet_settings.gateway, ethernet_settings.pridns, ethernet_settings.secdns);
    if (ethernet_settings.flags & TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON) {
        port_printf(port, "   DHCP:        Enabled\r\n");
        port_printf(port, "DHCP:\r\n");

        
        char ip[16];
        ip2str(TCPIP_STACK_NetAddress(eth0), ip);
        
        char gw[16];
        ip2str(TCPIP_STACK_NetAddressGateway(eth0), gw);

        char dns1[16];
        ip2str(TCPIP_STACK_NetAddressDnsPrimary(eth0), dns1);

        char dns2[16];
        ip2str(TCPIP_STACK_NetAddressDnsSecond(eth0), dns2);
        
        port_printf(port, "   IP Address:  %-15s   Gateway:       %s\r\n", ip, gw);
        port_printf(port, "   DNS Primary: %-15s   DNS Secondary: %s\r\n", dns1, dns2);
    } else {
        port_printf(port, "   DHCP:        Disabled\r\n");
    }

    
#ifdef WIFI_ENABLED

    const TCPIP_NET_HANDLE *wlan0 = TCPIP_STACK_NetHandleGet("wlan0");
    port_printf(port, "\nWiFi settings:\r\n");
    port_printf(port, "   MAC Address: %s\r\n", wifi_settings.macaddr);
    port_printf(port, "   Link: %s\r\n", 
            TCPIP_STACK_NetIsLinked(wlan0) ? "Associated" : "Idle"
            );
    port_printf(port, "Configured:\r\n");
    port_printf(port, "   IP Address:  %-15s   Netmask:       %-15s\r\n", wifi_settings.ip, wifi_settings.netmask);
    port_printf(port, "   Gateway:     %-15s   DNS:           %s/%s\r\n", wifi_settings.gateway, wifi_settings.pridns, wifi_settings.secdns);
    if (wifi_settings.flags & TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON) {
        port_printf(port, "   DHCP:        Enabled\r\n");
        port_printf(port, "DHCP:\r\n");

        
        char ip[16];
        ip2str(TCPIP_STACK_NetAddress(wlan0), ip);
        
        char gw[16];
        ip2str(TCPIP_STACK_NetAddressGateway(wlan0), gw);

        char dns1[16];
        ip2str(TCPIP_STACK_NetAddressDnsPrimary(wlan0), dns1);

        char dns2[16];
        ip2str(TCPIP_STACK_NetAddressDnsSecond(wlan0), dns2);
        
        port_printf(port, "   IP Address:  %-15s   Gateway:       %s\r\n", ip, gw);
        port_printf(port, "   DNS Primary: %-15s   DNS Secondary: %s\r\n", dns1, dns2);
    } else {
        port_printf(port, "   DHCP:        Disabled\r\n");
    }
    port_printf(port, "   SSID: %s\r\n", wifi_settings.ssid[0] == 0 ? "None" : wifi_settings.ssid);
    port_printf(port, "   PSK: %s\r\n", wifi_settings.psk[0] == 0 ? "Unset" : "Set");
#endif
}

COMMAND(ethernet_define_mac_address) {
    if (argc == 0) {
        return ERR_INCOMPLETE;
    }
    
    char *mac = argv[0];
    if (!validate_mac(mac)) return ERR_INVALID;
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_MAC, 0, 17, (uint8_t *)mac);
    return ERR_OK;
    
}
COMMAND(ethernet_define_ip) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_IP, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(ethernet_define_subnet) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_netmask(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_NETMASK, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(ethernet_define_gateway) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_GATEWAY, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(ethernet_define_pridns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_PRIDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(ethernet_define_secdns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_SECDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(ethernet_define_dhcp_enabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}
COMMAND(ethernet_define_dhcp_disabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}

COMMAND(wifi_define_mac_address) {
    if (argc == 0) {
        return ERR_INCOMPLETE;
    }
    
    char *mac = argv[0];
    if (!validate_mac(mac)) return ERR_INVALID;
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_MAC, 0, 17, (uint8_t *)mac);
    return ERR_OK;
    
}
COMMAND(wifi_define_ip) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_IP, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(wifi_define_subnet) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_netmask(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_NETMASK, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(wifi_define_gateway) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_GATEWAY, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(wifi_define_pridns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_PRIDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(wifi_define_secdns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SECDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(wifi_define_dhcp_enabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}
COMMAND(wifi_define_dhcp_disabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}
COMMAND(wifi_define_ssid) {
    if (argc != 1) return ERR_INCOMPLETE;
    int length = strlen(argv[0]);
    if (length > 32) length = 32;

    if (strcasecmp(argv[0], "none") == 0) {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SSID, 0, 0, NULL);
    } else {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SSID, 0, length, (uint8_t *)argv[0]);
    }
    return ERR_OK;
}
COMMAND(wifi_define_psk) {
    if (argc != 1) return ERR_INCOMPLETE;
    int length = strlen(argv[0]);
    if (length > 63) length = 63;
    if (strcasecmp(argv[0], "none") == 0) {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SSID, 0, 0, NULL);
    } else {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_PSK, 0, length, (uint8_t *)argv[0]);
    }
    return ERR_OK;    
}
