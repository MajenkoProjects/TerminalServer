#ifndef _SERVICE_H
#define _SERVICE_H

enum protocol {
    PROTO_LOCAL = 0,
    PROTO_TELNET,
};

struct service {
    struct service *next;
    char name[16];
    char address[64];
    uint16_t port;
    enum protocol protocol;
};

#endif