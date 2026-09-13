#ifndef _TELNET_IN_H
#define _TELNET_IN_H

#include "tcpip/berkeley_api.h"
#include "port.h"

struct telnet_in_data {
    int fd;
    struct sockaddr_in sin;
    int iac_pos;
    uint8_t iac[5];
    bool iac_sb;
    uint8_t iac_sub[40];
    int iac_sub_pos;
    bool iac_sb_iac;
};

extern void telnet_in_initialize();
extern void print_telnet_in_info(struct port *port, struct port *target);

#endif