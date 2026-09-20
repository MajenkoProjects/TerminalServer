#ifndef _TCP_IN_H
#define	_TCP_IN_H
#include <stdint.h>
#include <stdbool.h>

#include "config/default/library/tcpip/tcpip.h"
#include "config/default/library/tcpip/tcp.h"
#include "port.h"
#include "session.h"

enum tcp_in_state {
    TCP_IN_CLOSED = 0,
    TCP_IN_LISTEN,
    TCP_IN_CONNECTED,
    TCP_IN_DISCONNECTED
};

struct tcp_in_socket {
    TCP_SOCKET socket;
    uint16_t port_no;
    struct port *port;
    struct port *target;
    enum tcp_in_state state;
    int target_port_no;
    struct session *session;
    
};

struct tcp_in_data {
    struct tcp_in_socket *socket;
};

extern void tcp_in_init();
extern void tcp_in_task();

#endif	

