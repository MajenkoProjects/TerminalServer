#ifndef _TELNET_OUT_H
#define _TELNET_OUT_H

#include <stdint.h>
#include <stddef.h>

#include "tcpip/berkeley_api.h"
#include "port.h"
#include "command.h"

enum tostate {
    TO_START_DNS,
    TO_RUN_DNS,
    TO_BAD_HOST,
    TO_FOUND_HOST,
    TO_NOCONN,
    TO_CONNECTING,
    TO_CONNECT,
    TO_RUN,
};

struct todata {
    TCP_SOCKET socket;
    int fd;
    struct sockaddr_in sin;
    IP_MULTI_ADDRESS addr;
    TCPIP_DNS_RESULT dns_result;
    char hostname[64];
    enum tostate state;
};

extern void telnet_out_initialize();

extern COMMAND(telnet);

#endif