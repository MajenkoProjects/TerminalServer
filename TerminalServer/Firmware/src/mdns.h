#ifndef _MDNS_H
#define	_MDNS_H

#include <stdint.h>
#include "library/tcpip/tcpip.h"

#define RR_A 1
#define RR_PTR 12
#define RR_TXT 16
#define RR_SRV 33

enum mdns_service_type {
    MDNS_UNKNOWN,
    MDNS_TELNET,
    MDNS_SERIAL,
};

struct mdns_host {
    struct mdns_host *next;
    const char *hostname;
    IPV4_ADDR address;
    uint32_t last_seen;
};

struct mdns_service {
    struct mdns_service *next;
    char *name;
    struct mdns_host *host;
    uint16_t port;
    uint32_t last_seen;
    enum mdns_service_type type;
};

struct mdns_header {
    uint16_t tid;
    struct {
        unsigned    response    :1;
        unsigned    opcode      :4;
        unsigned                :1;
        unsigned    truncated   :1;
        unsigned    recurse     :1;
        unsigned                :1;
        unsigned    reserved    :1;
        unsigned                :1;
        unsigned    auth        :1;
        unsigned                :4;
        
    } __attribute__((packed)) flags;
    uint16_t questions;
    uint16_t answer_rrs;
    uint16_t authority_rrs;
    uint16_t additional_rrs;
} __attribute__((packed));

struct mdns_query {
    uint16_t type;
    uint16_t query_class;
} __attribute__((packed));

struct mdns_answer {
    uint16_t type;
    uint16_t query_class;
    uint32_t ttl;
    uint16_t data_length;
} __attribute__((packed));
extern void mdns_init();
extern void mdns_advertise_service(const char *name, const char *type, const char *protocol, uint16_t port);
extern void mdns_tasks();
extern uint32_t mdns_get_service(const char *name, uint16_t *port);

extern COMMAND(show_discovered_services);

#endif	
