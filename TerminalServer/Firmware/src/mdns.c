#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "definitions.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mdns.h"
#include "tcpip/tcpip.h"
#include "settings.h"
#include "port.h"
#include "network.h"
#include "util.h"
//#include "arp_private.h"

UDP_SOCKET mdns_socket;

static void mdns_rebind() {
    IP_MULTI_ADDRESS ip;
    ip.v4Add.v[0] = 224;
    ip.v4Add.v[1] = 0;
    ip.v4Add.v[2] = 0;
    ip.v4Add.v[3] = 251;
    
    IP_MULTI_ADDRESS ip1;
        
    const TCPIP_NET_HANDLE *handle = TCPIP_STACK_NetHandleGet("eth0");
    ip1.v4Add.Val = TCPIP_STACK_NetAddress(handle);
    TCPIP_UDP_SocketNetSet(mdns_socket, TCPIP_STACK_NetHandleGet("eth0"));
    (void)TCPIP_UDP_RemoteBind(mdns_socket, IP_ADDRESS_TYPE_IPV4, 5353, NULL);
    (void)TCPIP_UDP_Bind(mdns_socket, IP_ADDRESS_TYPE_IPV4, 5353, NULL); 
    (void)TCPIP_UDP_DestinationIPAddressSet(mdns_socket, IP_ADDRESS_TYPE_IPV4, &ip);
    (void)TCPIP_UDP_SourceIPAddressSet(mdns_socket ,IP_ADDRESS_TYPE_IPV4, &ip1);

}

void mdns_init() {


    

    mdns_socket = TCPIP_UDP_ServerOpen(IP_ADDRESS_TYPE_IPV4, 5353, NULL);
    if (!mdns_socket) {
        port_printf(CONSOLE, "Unable to open MDNS socket\r\n");
    }
    uint32_t v = 1;
    (void)TCPIP_UDP_OptionsSet(mdns_socket, UDP_OPTION_STRICT_NET, &v);
    
    
    mdns_rebind();

//    TCPIP_MAC_ADDR mcast_addr = { {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB} };
//    add_mac(0, &mcast_addr);



}


struct mdns_rr_ptr {
    char *service;
    char *protocol;
    char *target;
};

struct mdns_rr_txt {
    char *instance;
    char *service;
    char *protocol;
    //char *info;
};

struct mdns_rr_srv {
    char *instance;
    char *service;
    char *protocol;
    uint16_t port;
    char *target;
};

struct mdns_rr_a {
    char *name;
    uint32_t ip;
};

struct mdns_rr {
    struct mdns_rr *next;
    uint16_t type;
    void *data;
};

struct word {
    struct word *next;
    char *word;
    uint16_t offset;
};

#define RR_A 1
#define RR_PTR 12
#define RR_TXT 16
#define RR_SRV 33

struct mdns_rr *rr_data = NULL;
struct word *rr_words = NULL;

void discard_rr_data() {
    struct mdns_rr_srv *srv;
    struct mdns_rr_txt *txt;
    struct mdns_rr_ptr *ptr;
    struct mdns_rr_a *a;
    
    while (rr_data != NULL) {
        struct mdns_rr *next = rr_data->next;
        
        switch (rr_data->type) {
            case RR_A:
                a = (struct mdns_rr_a *)rr_data->data;
                free(a->name);
                free(a);
                free(rr_data);
                break;
            case RR_PTR:
                ptr = (struct mdns_rr_ptr *)rr_data->data;
                free(ptr->service);
                free(ptr->protocol);
                free(ptr->target);
                free(ptr);
                free(rr_data);
                break;
            case RR_TXT:
                txt = (struct mdns_rr_txt *)rr_data->data;
                //free(txt->info);
                free(txt->instance);
                free(txt->service);
                free(txt->protocol);
                free(txt);
                free(rr_data);
                break;
            case RR_SRV:
                srv = (struct mdns_rr_srv *)rr_data->data;
                free(srv->instance);
                free(srv->protocol);
                free(srv->service);
                free(srv->target);
                free(srv);
                free(rr_data);
                break;
                
        }
        
        
        rr_data = next;
    }
}

void discard_words() {
    while (rr_words != NULL) {
        struct word *next = rr_words->next;
        free(rr_words->word);
        free(rr_words);
        rr_words = next;
    }
}

void mdns_begin_reply() {
    if (rr_data != NULL) {
        discard_rr_data();
    }
    
    rr_data = NULL;
}

void mdns_queue_rr(uint16_t type, void *data) {
    struct mdns_rr *n = malloc(sizeof(struct mdns_rr));
    n->type = type;
    n->data = data;
    n->next = rr_data;
    rr_data = n;
}

void mdns_queue_rr_a(char *name, uint32_t ip) {
    struct mdns_rr_a *a = malloc(sizeof(struct mdns_rr_a));
    a->ip = ip;
    a->name = strdup(name);
    mdns_queue_rr(RR_A, a);
}

void mdns_queue_rr_ptr(const char *service, const char *protocol, const char *target) {
    struct mdns_rr_ptr *ptr = malloc(sizeof(struct mdns_rr_ptr));
    ptr->service = strdup(service);
    ptr->protocol = strdup(protocol);
    ptr->target = strdup(target);
    mdns_queue_rr(RR_PTR, ptr);
}

void mdns_queue_rr_txt(char *instance, char *service, char *protocol, char *info) {
    struct mdns_rr_txt *txt = malloc(sizeof(struct mdns_rr_txt));
    txt->instance = strdup(instance);
    txt->service = strdup(service);
    txt->protocol = strdup(protocol);
//    txt->info = strdup(info);
    mdns_queue_rr(RR_TXT, txt);
}

void mdns_queue_rr_srv(char *instance, char *service, char *protocol, char *target, uint16_t port) {
    struct mdns_rr_srv *srv = malloc(sizeof(struct mdns_rr_srv));
    srv->instance = strdup(instance);
    srv->service = strdup(service);
    srv->protocol = strdup(protocol);
    srv->target = strdup(target);
    srv->port = port;
    mdns_queue_rr(RR_SRV, srv);
}


void mdns_add_word(const char *word, uint16_t offset) {
    struct word *w = malloc(sizeof(struct word));
    if (!w) {
        return;
    }
    w->offset = offset;
    w->word = strdup(word);
    w->next = rr_words;
    rr_words = w;
}

uint16_t mdns_find_word(const char *word) {
    for (struct word *scan = rr_words; scan; scan = scan->next) {
        if (strcasecmp(scan->word, word) == 0) return scan->offset;
    }
    return 0;
}

bool mdns_put_word(const char *str) {
  /*  
    int pos = mdns_find_word(str);
    if (pos > 0) {
        TCPIP_UDP_Put(mdns_socket, 0xc0 | (pos >> 8));
        TCPIP_UDP_Put(mdns_socket, pos);
        return true;
    }

    mdns_add_word(str, TCPIP_UDP_TxOffsetGet(mdns_socket));
    */
    int l = strlen(str);
    TCPIP_UDP_Put(mdns_socket, l);
    TCPIP_UDP_ArrayPut(mdns_socket, (uint8_t *)str, l);
    return false;
}

void mdns_put_data(uint8_t *data, int len) {
    TCPIP_UDP_Put(mdns_socket, len >> 8);
    TCPIP_UDP_Put(mdns_socket, len);
    TCPIP_UDP_ArrayPut(mdns_socket, data, len);
}

void mdns_finish_rr() {
    int count = 0;
    for (struct mdns_rr *scan = rr_data; scan; scan = scan->next) {
        count++;
    }
    
    if (count == 0) {
        TCPIP_UDP_Discard(mdns_socket);
        discard_words();
        discard_rr_data();
        return;
    }
    
    (void)TCPIP_UDP_PutIsReady(mdns_socket);
    // TID
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // Flags
    TCPIP_UDP_Put(mdns_socket, 0x84);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // Qs
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    // RRs
    TCPIP_UDP_Put(mdns_socket, count >> 8);
    TCPIP_UDP_Put(mdns_socket, count);

    // Auth
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);
    
    // Add
    TCPIP_UDP_Put(mdns_socket, 0x00);
    TCPIP_UDP_Put(mdns_socket, 0x00);

    
    for (struct mdns_rr *scan = rr_data; scan; scan = scan->next) {
        struct mdns_rr_srv *srv;
        struct mdns_rr_txt *txt;
        struct mdns_rr_ptr *ptr;
        struct mdns_rr_a *a;        
        int dsize_pos;
        int data_start;
        int data_end;
        int data_len;
        
        switch (scan->type) {
            case RR_A:
                a = (struct mdns_rr_a *)scan->data;
                if (!mdns_put_word(a->name))
                    if (!mdns_put_word("local"))
                        TCPIP_UDP_Put(mdns_socket, 0x00);

                TCPIP_UDP_Put(mdns_socket, RR_A >> 8);
                TCPIP_UDP_Put(mdns_socket, RR_A);
                TCPIP_UDP_Put(mdns_socket, 0x80);
                TCPIP_UDP_Put(mdns_socket, 0x01);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x78);                
                mdns_put_data((uint8_t *)&a->ip, 4);
                break;
                
            case RR_PTR:
                ptr = (struct mdns_rr_ptr *)scan->data;
                if (!mdns_put_word(ptr->service))
                    if (!mdns_put_word(ptr->protocol))
                        if (!mdns_put_word("local"))
                            TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, RR_PTR >> 8);
                TCPIP_UDP_Put(mdns_socket, RR_PTR);
                TCPIP_UDP_Put(mdns_socket, 0x80);
                TCPIP_UDP_Put(mdns_socket, 0x01);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x78);
                dsize_pos = TCPIP_UDP_TxOffsetGet(mdns_socket);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                data_start = TCPIP_UDP_TxOffsetGet(mdns_socket);
                if (!mdns_put_word(ptr->target))
                    if (!mdns_put_word(ptr->service))
                        if (!mdns_put_word(ptr->protocol))
                            if (!mdns_put_word("local"))
                                TCPIP_UDP_Put(mdns_socket, 0x00);
                data_end = TCPIP_UDP_TxOffsetGet(mdns_socket);
                data_len = data_end - data_start;
                TCPIP_UDP_TxOffsetSet(mdns_socket, dsize_pos, false);
                TCPIP_UDP_Put(mdns_socket, data_len >> 8);
                TCPIP_UDP_Put(mdns_socket, data_len);
                TCPIP_UDP_TxOffsetSet(mdns_socket, data_end, false);
                break;
                    
            case RR_TXT:
                txt = (struct mdns_rr_txt *)scan->data;
                if (!mdns_put_word(txt->instance))
                    if (!mdns_put_word(txt->service))
                        if (!mdns_put_word(txt->protocol))
                            if (!mdns_put_word("local"))
                                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, RR_TXT >> 8);
                TCPIP_UDP_Put(mdns_socket, RR_TXT);
                TCPIP_UDP_Put(mdns_socket, 0x80);
                TCPIP_UDP_Put(mdns_socket, 0x01);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x78);        

                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x01);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                break;
            case RR_SRV:
                srv = (struct mdns_rr_srv *)scan->data;
                if (!mdns_put_word(srv->instance))
                    if (!mdns_put_word(srv->service))
                        if (!mdns_put_word(srv->protocol))
                            if (!mdns_put_word("local"))
                                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, RR_SRV >> 8);
                TCPIP_UDP_Put(mdns_socket, RR_SRV);
                TCPIP_UDP_Put(mdns_socket, 0x80);
                TCPIP_UDP_Put(mdns_socket, 0x01);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x78);        
                dsize_pos = TCPIP_UDP_TxOffsetGet(mdns_socket);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);

                data_start = TCPIP_UDP_TxOffsetGet(mdns_socket);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, 0x00);
                TCPIP_UDP_Put(mdns_socket, srv->port >> 8);
                TCPIP_UDP_Put(mdns_socket, srv->port);
                if (!mdns_put_word(srv->target))
                    if (!mdns_put_word("local"))
                        TCPIP_UDP_Put(mdns_socket, 0x00);
                data_end = TCPIP_UDP_TxOffsetGet(mdns_socket);
                data_len = data_end - data_start;
                TCPIP_UDP_TxOffsetSet(mdns_socket, dsize_pos, false);
                TCPIP_UDP_Put(mdns_socket, data_len >> 8);
                TCPIP_UDP_Put(mdns_socket, data_len);
                TCPIP_UDP_TxOffsetSet(mdns_socket, data_end, false);
                break;
        }
    }
        
    mdns_rebind();
    TCPIP_UDP_Flush(mdns_socket);
    discard_words();
    discard_rr_data();
}



int mdns_expand_name(uint8_t *buf, int offset, char *out, int maxlen) {
    char tmp[64] = {0};
    
    while (buf[offset] != 0) {
        if ((buf[offset] & 0xC0) == 0xC0) {
            uint16_t recoff = ((buf[offset] << 8) | buf[offset+1]) & 0x3FFF;
            memset(tmp, 0, 64);
            mdns_expand_name(buf, recoff, tmp, 63);
            strcat(out, tmp);
//            strcat(out, ".");
            offset += 2;
            return offset;
        }

        memset(tmp, 0, 64);
        memcpy(tmp, &buf[offset+1], buf[offset]);
        strcat(out, tmp);
        strcat(out, ".");
        offset += buf[offset]+1;
    }
    return offset+1;
}

void mdns_process_incoming() {
    UDP_SOCKET_INFO info;

    const TCPIP_NET_HANDLE *eth0 = TCPIP_STACK_NetHandleGet("eth0");
//    const TCPIP_NET_HANDLE *wlan0 = TCPIP_STACK_NetHandleGet("wlan0");

        TCPIP_UDP_SocketInfoGet(mdns_socket, &info);
        if (info.sourceIPaddress.v4Add.Val == TCPIP_STACK_NetAddress(eth0)) {
            TCPIP_UDP_Discard(mdns_socket);
            return;
        }

//        if (info.sourceIPaddress.v4Add.Val == TCPIP_STACK_NetAddress(wlan0)) {
//            TCPIP_UDP_Discard(mdns_socket);
//            return;
//        }

        
        struct mdns_header header;
        
        int len = TCPIP_UDP_GetIsReady(mdns_socket);
                
        if (len < sizeof(struct mdns_header)) {
            TCPIP_UDP_Discard(mdns_socket);
            return;
        }

        uint8_t *buf = alloca(len);
        TCPIP_UDP_ArrayGet(mdns_socket, buf, len);
        mdns_rebind();

        struct mdns_header *h = (struct mdns_header *)buf;        
        
        int offset = sizeof(struct mdns_header);
        
        header.additional_rrs = TCPIP_Helper_ntohs(h->additional_rrs);
        header.answer_rrs = TCPIP_Helper_ntohs(h->answer_rrs);
        header.authority_rrs = TCPIP_Helper_ntohs(h->authority_rrs);
        header.questions = TCPIP_Helper_ntohs(h->questions);
        header.tid = TCPIP_Helper_ntohs(h->tid);
        header.flags.auth = h->flags.auth;
        header.flags.opcode = h->flags.opcode;
        header.flags.recurse = h->flags.recurse;
        header.flags.response = h->flags.response;
        header.flags.truncated = h->flags.truncated;
                
        mdns_begin_reply();
        
        for (int i = 0; i < header.questions; i++) {
            char tmp[256] = {0};
            char fqdn[64] = {0};

            offset = mdns_expand_name(buf, offset, tmp, 256);
            struct mdns_query query;
            struct mdns_query *q;
            q = (struct mdns_query *)&buf[offset];
            
            offset += sizeof(struct mdns_query);
            query.query_class = TCPIP_Helper_ntohs(q->query_class);
            query.type = TCPIP_Helper_ntohs(q->type);
            

            switch (query.type) {

                case RR_A:
                    strcpy(fqdn, system_settings.nodename);
                    strcat(fqdn, ".local.");
                    if (strcasecmp(fqdn, tmp) == 0) {
                        mdns_queue_rr_a(system_settings.nodename, TCPIP_STACK_NetAddress(TCPIP_STACK_NetHandleGet("eth0")));
                    }
                    break;

                case RR_PTR:
                    if (strcasecmp(tmp, "_serial._tcp.local.") == 0) {
                        for (struct port *port = ports; port; port = port->next) {
                            if ((port->type == PORT_SERIAL) || (port->type == PORT_CDC)) {
                                if ((port->access == ACCESS_REMOTE) || (port->access == ACCESS_DYNAMIC)) {
                                    mdns_queue_rr_ptr("_serial", "_tcp", port->name);
                                }
                            }
                        }
                        break;
                    }
                    if (strcasecmp(tmp, "_telnet._tcp.local.") == 0) {
                        mdns_queue_rr_ptr("_telnet", "_tcp", system_settings.nodename);
                        break;
                    }
                    break;                    
                case RR_TXT:
                    for (struct port *port = ports; port; port = port->next) {
                        if ((port->type == PORT_SERIAL) || (port->type == PORT_CDC)) {
                            if ((port->access == ACCESS_REMOTE) || (port->access == ACCESS_DYNAMIC)) {                                              
                                strcpy(fqdn, port->name);
                                strcat(fqdn, "._serial._tcp.local.");
                                if (strcasecmp(fqdn, tmp) == 0) {
                                    mdns_queue_rr_txt(port->name, "_serial", "_tcp", "");
                                }
                            }
                        }
                    }
                    strcpy(fqdn, system_settings.nodename);
                    strcat(fqdn, "._telnet._tcp.local.");
                    if (strcasecmp(fqdn, tmp) == 0) {
                        mdns_queue_rr_txt(system_settings.nodename, "_telnet", "_tcp", "");                        
                    }
                    break;
                    
                case RR_SRV:
                    for (struct port *port = ports; port; port = port->next) {
                        if ((port->type == PORT_SERIAL) || (port->type == PORT_CDC)) {
                            if ((port->access == ACCESS_REMOTE) || (port->access == ACCESS_DYNAMIC)) {                  
                                strcpy(fqdn, port->name);
                                strcat(fqdn, "._serial._tcp.local.");
                                if (strcasecmp(fqdn, tmp) == 0) {
                                    mdns_queue_rr_srv(port->name, "_serial", "_tcp", system_settings.nodename, 3000 + port->no);
                                    
                                }
                            }
                        }
                    }
                    strcpy(fqdn, system_settings.nodename);
                    strcat(fqdn, "._telnet._tcp.local.");
                    if (strcasecmp(fqdn, tmp) == 0) {
                        mdns_queue_rr_srv(system_settings.nodename, "_telnet", "_tcp", system_settings.nodename, 23);
                    }
            }            
        }

        mdns_finish_rr();
        
        TCPIP_UDP_Discard(mdns_socket);
    
}

void mdns_tasks() {
    
    if (TCPIP_UDP_GetIsReady(mdns_socket) > 0) {
        mdns_process_incoming();
    }
    
    
    
}