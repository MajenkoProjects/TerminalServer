#ifndef _TELNET_OUT_H
#define _TELNET_OUT_H

#include <stdint.h>
#include <stddef.h>

#include "tcpip/tcpip.h"
#include "tcpip/tcp.h"
#include "port.h"
#include "command.h"

enum tostate {
    TO_DNS_PRECHECK,
    TO_START_DNS,
    TO_RUN_DNS,
    TO_BAD_HOST,
    TO_START_DNS2,
    TO_RUN_DNS2,
    TO_FOUND_HOST,
    TO_NOCONN,
    TO_BREAK,
    TO_CONNECTING,
    TO_CONNECT,
    TO_RUN,
    TO_CLOSE,
    
};

struct todata {
    TCP_SOCKET socket;
    IP_MULTI_ADDRESS addr;
    TCPIP_DNS_RESULT dns_result;
    char hostname[255];
    uint16_t port;
    enum tostate state;
    int iac_pos;
    uint8_t iac[5];
    bool iac_sb;
    uint8_t iac_sub[40];
    int iac_sub_pos;
    bool iac_sb_iac;
    bool queue_close;
    uint32_t ts;
    struct port *parent;
    struct session *session;
};

extern void telnet_out_initialize();
extern void telnet_out_task();

extern COMMAND(telnet);

#endif