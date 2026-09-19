#ifndef _TELNET_IN_H
#define _TELNET_IN_H

#include "tcpip/berkeley_api.h"
#include "port.h"

struct telnet_in_data {
    struct tcp_socket *socket;
    int iac_pos;
    uint8_t iac[5];
    bool iac_sb;
    uint8_t iac_sub[40];
    int iac_sub_pos;
    bool iac_sb_iac;
    bool queue_close;
};

enum socket_state {
    SOCK_LISTEN,
    SOCK_CONNECTED,
    SOCK_DISCONNECTED,
    SOCK_CLOSED
};

struct tcp_socket {
    TCP_SOCKET socket;
    enum socket_state state;
    struct port *port;
};

#define NUM_TELNET_SOCKETS 6
#define PORT_TELNET 23

extern void telnet_in_initialize();
extern void print_telnet_in_info(struct port *port, struct port *target);


#endif