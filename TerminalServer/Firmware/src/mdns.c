#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "mdns.h"
#include "tcpip/tcpip.h"
#include "settings.h"
#include "port.h"
#include "network.h"


UDP_SOCKET mdns_socket;

static uint32_t mdns_last_bc = 0;

void mdns_init() {

    IP_MULTI_ADDRESS ip;
    ip.v4Add.v[0] = 224;
    ip.v4Add.v[1] = 0;
    ip.v4Add.v[2] = 0;
    ip.v4Add.v[3] = 251;
    
    IP_MULTI_ADDRESS ip1;
        
    const TCPIP_NET_HANDLE *handle = TCPIP_STACK_NetHandleGet("eth0");
    ip1.v4Add.Val = TCPIP_STACK_NetAddress(handle);
    

    mdns_socket = TCPIP_UDP_ServerOpen(IP_ADDRESS_TYPE_IPV4, 5353, NULL);
    if (!mdns_socket) {
        port_printf(CONSOLE, "Unable to open MDNS socket\r\n");
    } else {
        port_printf(CONSOLE, "MDNS Socket opened\r\n");
    }
    uint32_t v = 1;
    (void)TCPIP_UDP_OptionsSet(mdns_socket, UDP_OPTION_STRICT_NET, &v);
    
    (void)TCPIP_UDP_RemoteBind(mdns_socket, IP_ADDRESS_TYPE_IPV4, 5353, NULL);
    (void)TCPIP_UDP_Bind(mdns_socket, IP_ADDRESS_TYPE_IPV4, 5353, NULL); 
    (void)TCPIP_UDP_DestinationIPAddressSet(mdns_socket, IP_ADDRESS_TYPE_IPV4, &ip);
    (void)TCPIP_UDP_SourceIPAddressSet(mdns_socket ,IP_ADDRESS_TYPE_IPV4, &ip1);


    TCPIP_MAC_ADDR mcast_addr = { {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB} };
    add_mac(0, &mcast_addr);



}





void mdns_advertise_service(const char *name, const char *type, const char *protocol, uint16_t port) {

    if (!TCPIP_UDP_TxPutIsReady(mdns_socket, 256)) return;

    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x84);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // Qs
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // RRs
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x01);

    // Auth
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    
    // Add
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    TCPIP_UDP_Put(mdns_socket, strlen(name));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)name, strlen(name));
    TCPIP_UDP_Put(mdns_socket, strlen(type));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)type, strlen(type));
    TCPIP_UDP_Put(mdns_socket, strlen(protocol));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)protocol, strlen(protocol));
    TCPIP_UDP_Put(mdns_socket, 5);
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)"local", 5);
    TCPIP_UDP_Put(mdns_socket, 0);

    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x21);

    TCPIP_UDP_Put(mdns_socket, 0x80);
    TCPIP_UDP_Put(mdns_socket, 0x01);

    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x0e);
    TCPIP_UDP_Put(mdns_socket, 0x10);

    
    uint16_t l = strlen(system_settings.nodename) + 14;


    TCPIP_UDP_Put(mdns_socket, (l >> 8) & 0xFF);
    TCPIP_UDP_Put(mdns_socket, l & 0xFF);
    
    // Priority
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // Weight
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    
    // Port
    TCPIP_UDP_Put(mdns_socket, (port >> 8) & 0xFF);
    TCPIP_UDP_Put(mdns_socket, port & 0xFF);

    TCPIP_UDP_Put(mdns_socket, strlen(system_settings.nodename));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)system_settings.nodename, strlen(system_settings.nodename));
    TCPIP_UDP_Put(mdns_socket, 0x05);
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)"local", 5);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    TCPIP_UDP_Flush(mdns_socket);
}


void mdns_advertise_pointer(const char *name, const char *type, const char *protocol, uint16_t port) {

    if (!TCPIP_UDP_TxPutIsReady(mdns_socket, 256)) return;

    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x84);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // Qs
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // RRs
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x01);

    // Auth
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    
    // Add
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    TCPIP_UDP_Put(mdns_socket, strlen(type));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)type, strlen(type));
    TCPIP_UDP_Put(mdns_socket, strlen(protocol));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)protocol, strlen(protocol));
    TCPIP_UDP_Put(mdns_socket, 5);
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)"local", 5);
    TCPIP_UDP_Put(mdns_socket, 0);

    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 12);

    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x01);

    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x0e);
    TCPIP_UDP_Put(mdns_socket, 0x10);

    
    uint16_t l = strlen(name) + 21;


    TCPIP_UDP_Put(mdns_socket, (l >> 8) & 0xFF);
    TCPIP_UDP_Put(mdns_socket, l & 0xFF);
    
    TCPIP_UDP_Put(mdns_socket, strlen(name));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)name, strlen(name));
    TCPIP_UDP_Put(mdns_socket, strlen(type));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)type, strlen(type));
    TCPIP_UDP_Put(mdns_socket, strlen(protocol));
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)protocol, strlen(protocol));
    TCPIP_UDP_Put(mdns_socket, 5);
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)"local", 5);
    TCPIP_UDP_Put(mdns_socket, 0);

    TCPIP_UDP_Flush(mdns_socket);
}

void mdns_tasks() {

    if ((xTaskGetTickCount() - mdns_last_bc) > 10000) {
        port_printf(CONSOLE, "Sending BC req\r\n"); CONSOLE->fn_flush(CONSOLE);
        mdns_last_bc = xTaskGetTickCount();
        for (struct port *scan = ports; scan; scan = scan->next) {
            if ((scan->access == ACCESS_REMOTE) || (scan->access == ACCESS_DYNAMIC)) {
                port_printf(CONSOLE, "  -> %s\r\n", scan->name); CONSOLE->fn_flush(CONSOLE);
                mdns_advertise_service(scan->name, "_serial", "_tcp", 3000 + scan->no);
                mdns_advertise_pointer(scan->name, "_serial", "_tcp", 3000 + scan->no);
            }
        }
        port_printf(CONSOLE, "Sent BC req\r\n"); CONSOLE->fn_flush(CONSOLE);
    }
}