#ifndef _MDNS_H
#define	_MDNS_H

#include <stdint.h>

#define RR_A 1
#define RR_PTR 12
#define RR_TXT 16
#define RR_SRV 33

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
    uint16_t authority_rrs;
    uint16_t answer_rrs;
    uint16_t additional_rrs;
} __attribute__((packed));

struct mdns_query {
    uint16_t type;
    uint16_t query_class;
} __attribute__((packed));

extern void mdns_init();
extern void mdns_advertise_service(const char *name, const char *type, const char *protocol, uint16_t port);
extern void mdns_tasks();
#endif	
