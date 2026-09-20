#include <string.h>

#include "telnet_out.h"
#include "cbuff.h"
#include "session.h"
#include "errno.h"
#include "telopt.h"
#include "uart.h"
#include "util.h"
#include "settings.h"

TaskHandle_t telnet_out_thread_handle;


const char *telopt_names[] = {
    "BINARY",
    "ECHO",
    "RECONN",
    "SUPPRESS_GA",
    "AMSN",
    "STATUS",
    "TM",
    "RCTE",
    "OLW",
    "OPS",
    "OCRD",
    "OHTS",
    "OHTD",
    "OFFD",
    "OVTS",
    "OVTD",
    "OLFD",
    "EXTEND_ASCII",
    "LOGOUT",
    "BYTE_MACRO",
    "DET",
    "SUPDUP",
    "SUPDUP_OUT",
    "SEND_LOC",
    "TTYPE",
    "EOR",
    "TACACS",
    "OUTPUT_MARK",
    "LOCNO",
    "3270_REGIME",
    "X3_PAD",
    "NAWS",
    "SPEED",
    "FLOW",
    "LINEMODE",
    "X_DISP_LOC",
    "ENV_OPT",
    "AUTH_OPT",
    "ENC_OPT",
    "NEW_ENV_OPT",
    "TN3270E",
    "XAUTH"
};


void prterror() {
    switch (errno) {
        case ENOTCONN: port_printf(CONSOLE, "ENOTCONN\r\n"); break;
        case EBADF: port_printf(CONSOLE, "EBADF\r\n"); break;
        case EINVAL: port_printf(CONSOLE, "EINVAL\r\n"); break;
        case ECONNREFUSED: port_printf(CONSOLE, "ECONNREFUSED\r\n"); break;
        case EINPROGRESS: port_printf(CONSOLE, "EINPROGRESS\r\n"); break;
        case ECONNRESET: port_printf(CONSOLE, "ECONNRESET\r\n"); break;
        case EWOULDBLOCK: port_printf(CONSOLE, "EWOULDBLOCK\r\n"); break;
        default:
            port_printf(CONSOLE, "{%d}\r\n", errno);

    }    
}

#define SEND_IAC(X, Y)  while (TCPIP_TCP_Put(data->socket, TELOPT_IAC) != 1){}; \
                        while (TCPIP_TCP_Put(data->socket, (X)) != 1){}; \
                        while (TCPIP_TCP_Put(data->socket, (Y)) != 1){}; 

#define SEND_DO(X)      SEND_IAC(TELOPT_DO, X)
#define SEND_DONT(X)    SEND_IAC(TELOPT_DONT, X)
#define SEND_WILL(X)    SEND_IAC(TELOPT_WILL, X)
#define SEND_WONT(X)    SEND_IAC(TELOPT_WONT, X)

static char scratch[255];

void telnet_out_transfer_data(struct port *port) {
    struct todata *data = (struct todata *)port->port_data;

    int av = xStreamBufferBytesAvailable(port->write_buffer);

    if (av > 0) {
        int fr = TCPIP_TCP_PutIsReady(data->socket);
        if (av > fr) {
            av = fr;
        }

        uint8_t *tmp = alloca(av);
        xStreamBufferReceive(port->write_buffer, tmp, av, 1000);
        TCPIP_TCP_ArrayPut(data->socket, tmp, av);
    }

    if (TCPIP_TCP_GetIsReady(data->socket)&& (xStreamBufferSpacesAvailable(port->read_buffer) > 0)) {
        uint8_t b;
        if ((TCPIP_TCP_ArrayGet(data->socket, &b, 1) == 1) ) {
           // port_printf(CONSOLE, "[%02x] ", b);
            if (data->iac_sb) {
                switch (b) {
                    case TELOPT_IAC:
                        if (data->iac_sb_iac) {
                            data->iac_sub[data->iac_sub_pos++] = b;
                            data->iac_sub[data->iac_sub_pos] = 0;
                            if (data->iac_sub_pos >= 40) {
                                data->iac_sb = false;
                                data->iac_pos = 0;    
                            }                                        
                        } else {
                            data->iac_sb_iac = true;
                        }
                        break;
                    case TELOPT_SE:
                        switch (data->iac_sub[0]) {
                            case TELOPT_NAWS:
                                port->columns = (data->iac_sub[1] << 8) | data->iac_sub[2];
                                port->lines = (data->iac_sub[3] << 8) | data->iac_sub[4];
                                break;
                            case TELOPT_TTYPE:
                                if (data->iac_sub[1] == SB_SEND) {                           
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_IAC) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_SB) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_TTYPE) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, SB_IS) == 0) {};
                                    for (int i = 0; i < strlen(data->parent->ttype); i++) {
                                        while (TCPIP_TCP_Put(data->socket, data->parent->ttype[i]) == 0) {};
                                    }
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_IAC) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_SE) == 0) {};
                                }
                                break;
                            case TELOPT_SPEED:
                                if (data->iac_sub[1] == SB_SEND) {          
                                    int baud = 9600;
                                    if (data->parent->type == PORT_SERIAL) {
                                        struct uart_data *d = data->parent->port_data;
                                        baud = d->baud;
                                    }
                                    sprintf(scratch, "%d,%d", baud, baud);
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_IAC) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_SB) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_SPEED) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, SB_IS) == 0) {};
                                    for (int i = 0; i < strlen(scratch); i++) {
                                        while (TCPIP_TCP_Put(data->socket, scratch[i]) == 0) {};
                                    }
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_IAC) == 0) {};
                                    while (TCPIP_TCP_Put(data->socket, TELOPT_SE) == 0) {};
                                }
                                break;
                            default:
                                break;
                        }
                        data->iac_sb = false;
                        data->iac_pos = 0;
                        break;
                    default:
                        data->iac_sub[data->iac_sub_pos++] = b;
                        data->iac_sub[data->iac_sub_pos] = 0;
                        if (data->iac_sub_pos >= 40) {
                        data->iac_sb = false;
                            data->iac_pos = 0;    
                        }
                        break;
                }
            } else {
                switch (data->iac_pos) {
                    case 0: 
                        if (b == TELOPT_IAC) {
                            data->iac[data->iac_pos++] = TELOPT_IAC;
                        } else {
                            xStreamBufferSend(port->read_buffer, &b, 1, 1);
                        }
                        break;

                    case 1:
                        switch (b) {
                            case TELOPT_IAC:
                                xStreamBufferSend(port->read_buffer, &b, 1, 1);
                                data->iac_pos = 0;                                       
                                break;
                            case TELOPT_GA:
                                data->iac_pos = 0;
                                //port_rprintf(scan, "GA\r\n");
                                break;
                            case TELOPT_SB:
                                data->iac_sb = true;
                                data->iac_sub_pos = 0;
                                break;
                            case TELOPT_DM:
                                data->iac_pos = 0;

                                break;
                            case TELOPT_BREAK:
                                data->iac_pos = 0;
                                switch (port->breakmode) {
                                    case BREAK_LOCAL:
                                        port_rprintf(port, "+++ OUT OF CHEESE +++\r\n\n");
                                        port->mode = MODE_LOCAL;
                                        break;
                                    case BREAK_REMOTE:
                                        if (port->active_session) {
                                            port->active_session->target->send_break = true;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                                break;
                            case TELOPT_DO:
                            case TELOPT_DONT:
                            case TELOPT_WILL:
                            case TELOPT_WONT:
                                data->iac[data->iac_pos++] = b;
                                break;
                            default:
                                data->iac_pos = 0;
                                port_rprintf(port, "Unexpected IAC %d\r\n", b);
                                break;
                        }
                        break;

                    case 2:

                        switch (b) {
                            default:
                                data->iac[data->iac_pos++] = b;
                                switch (data->iac[1]) {
                                    case TELOPT_WILL:
                                        switch (data->iac[2]) {
                                            case TELOPT_SUPPRESS_GA:
                                            case TELOPT_ECHO:
                                            case TELOPT_TTYPE:
                                            case TELOPT_NAWS:
                                                SEND_DO(data->iac[2])
                                                break;


                                            case TELOPT_STATUS:
                                            case TELOPT_LINEMODE:
                                                SEND_DONT(data->iac[2])
                                                break;

                                            default:
                                                SEND_DONT(data->iac[2])
                                                break;

                                        }
                                        break;
                                    case TELOPT_WONT:
                                        SEND_DONT(data->iac[2])
                                        break;

                                    case TELOPT_DO:
                                        switch (data->iac[2]) {
                                            case TELOPT_TTYPE:
                                            case TELOPT_BINARY:
                                            case TELOPT_NAWS:
                                            case TELOPT_SPEED:
                                                SEND_WILL(data->iac[2])
                                                break;


                                            case TELOPT_ECHO:
                                            case TELOPT_X_DISP_LOC:
                                            case TELOPT_NEW_ENV_OPT:
                                            case TELOPT_ENV_OPT:
                                            case TELOPT_LINEMODE:
                                            case TELOPT_FLOW:
                                                SEND_WONT(data->iac[2])
                                                break;

                                            default:
                                                SEND_WONT(data->iac[2])                                                                
                                                break;
                                        }
                                        break;
                                    case TELOPT_DONT:
                                        switch (data->iac[2]) {
                                            default:
                                                SEND_WONT(data->iac[2])
                                                break;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                                data->iac_pos = 0;
                        }
                }
            }
        }


    }
}

//static void telnet_out_thread(void *args) {
void telnet_out_task() {
//    while (1) {
        for (struct port *scan = ports; scan; scan = scan->next) {
            if (scan->type != PORT_TELNET_OUT) continue;
            struct todata *data = (struct todata *)scan->port_data;
            TCP_SOCKET_INFO info;
            TCPIP_TCP_SocketInfoGet(data->socket, &info);
            uint8_t this_byte;
            
            //   port_printf(CONSOLE, "_%d_ ", info.rxPending);
          //  vTaskDelay(10);
            switch (data->state) {
                case TO_DNS_PRECHECK: // Check to see if either the fqdn or subdomain are cached.
                    
                    // First check to see if it's an IP address
                    if (validate_ip(data->hostname)) {
                        TCPIP_Helper_StringToIPAddress(data->hostname, &data->addr.v4Add);
                        data->state = TO_FOUND_HOST;
                    }
                    
                    data->dns_result = TCPIP_DNS_IsResolved(data->hostname, &data->addr, TCPIP_DNS_TYPE_A);
                    if (data->dns_result == TCPIP_DNS_RES_OK) {
                        data->state = TO_FOUND_HOST;
                        break;
                    }
                    snprintf(scratch, 254, "%s.%s", data->hostname, system_settings.domain);
                    data->dns_result = TCPIP_DNS_IsResolved(scratch, &data->addr, TCPIP_DNS_TYPE_A);
                    if (data->dns_result == TCPIP_DNS_RES_OK) {
                        data->state = TO_FOUND_HOST;
                        break;
                    }
                    data->state = TO_START_DNS;
                    break;
                    
                case TO_START_DNS: // Start looking up the name as an fqdn
                    data->dns_result = TCPIP_DNS_IsResolved(data->hostname, &data->addr, TCPIP_DNS_TYPE_A);
                    if (data->dns_result == TCPIP_DNS_RES_OK) {
                        data->state = TO_FOUND_HOST;
                    } else {
                        data->dns_result = TCPIP_DNS_Resolve(data->hostname, TCPIP_DNS_TYPE_A);
                        data->state = TO_RUN_DNS;          
                    }
                    break;

                case TO_RUN_DNS: // Check to see if the name is resolved
                    if (xStreamBufferBytesAvailable(scan->write_buffer) > 0) {
                        xStreamBufferReceive(scan->write_buffer, &this_byte, 1, 1);
                        if (this_byte == 3) {
                            data->state = TO_BREAK;
                            break;

                        }                    
                    }
                    data->dns_result = TCPIP_DNS_IsResolved(data->hostname, &data->addr, TCPIP_DNS_TYPE_A);
                    if (data->dns_result == TCPIP_DNS_RES_OK) {
                        data->state = TO_FOUND_HOST;
                    } else if (data->dns_result == TCPIP_DNS_RES_PENDING) {
                        data->state = TO_RUN_DNS;
                    } else {
                        data->state = TO_START_DNS2;
                    }   
                    break;


                case TO_START_DNS2: // Append the domain and search again
                    snprintf(scratch, 254, "%s.%s", data->hostname, system_settings.domain);
                    strcpy(data->hostname, scratch);
                    data->dns_result = TCPIP_DNS_IsResolved(data->hostname, &data->addr, TCPIP_DNS_TYPE_A);
                    if (data->dns_result == TCPIP_DNS_RES_OK) {
                        data->state = TO_FOUND_HOST;
                        break;
                    }
                    data->dns_result = TCPIP_DNS_Resolve(data->hostname, TCPIP_DNS_TYPE_A);
                    data->state = TO_RUN_DNS;
                    break;

                case TO_RUN_DNS2:
                    if (xStreamBufferBytesAvailable(scan->write_buffer) > 0) {
                        xStreamBufferReceive(scan->write_buffer, &this_byte, 1, 1);
                        if (this_byte == 3) {
                            data->state = TO_BREAK;
                            break;

                        }                    
                    }
                  
                    data->dns_result = TCPIP_DNS_IsResolved(data->hostname, &data->addr, TCPIP_DNS_TYPE_A);
                    if (data->dns_result == TCPIP_DNS_RES_OK) {
                        data->state = TO_FOUND_HOST;
                    } else if (data->dns_result == TCPIP_DNS_RES_PENDING) {
                        data->state = TO_RUN_DNS2;
                    } else {
                        data->state = TO_BAD_HOST;
                    }   
                    break;
                    
                case TO_BAD_HOST:
                    port_rprintf(scan, "Host not found\r\n");
                    data->state = TO_CLOSE;
                    break;
                    
                case TO_CLOSE: 
                    if (data->session) {
                        struct port *par = data->session->parent;
                        if (par) {
                            port_flush(scan);
                            par->active_session = NULL;
                            delete_session(data->session);                    
                            TCPIP_TCP_Close(data->socket);
                            delete_port(scan);
                            par->mode = MODE_LOCAL;
                        }
                    }
//                    close_port(scan);
                    break;
                    
                case TO_FOUND_HOST:
                    port_rprintf(scan, "Connecting to %d.%d.%d.%d:%d...",
                            data->addr.v4Add.v[0],
                            data->addr.v4Add.v[1],
                            data->addr.v4Add.v[2],
                            data->addr.v4Add.v[3],
                            data->port
                            );
                    data->state = TO_CONNECT;
                    data->socket = TCPIP_TCP_ClientOpen(IP_ADDRESS_TYPE_IPV4, data->port, &data->addr);
                    data->state = TO_CONNECTING;
                    data->ts = xTaskGetTickCount();
                    break;
                    
                case TO_CONNECTING:
                    if (TCPIP_TCP_ArrayGet(data->socket, (uint8_t *)scratch, 1) == 1) {
                        if (scratch[0] == 3) {
                            data->state = TO_BREAK;
                            break;
                        }
                    }
                    if (xTaskGetTickCount() - data->ts > 10000) {
                        data->state = TO_NOCONN;
                        break;
                    }
                    if (TCPIP_TCP_IsConnected(data->socket)) {
                        data->state = TO_CONNECT;
                    }
                    break;
                    
                case TO_NOCONN:
                    port_rprintf(scan, "Unable to connect\r\n");
                    data->state = TO_CLOSE;
                    break;

                case TO_BREAK:
                    port_rprintf(scan, "Cancelled\r\n");
                    data->state = TO_CLOSE;
                    break;
                    
                case TO_CONNECT:
                    port_rprintf(scan, "Connected\r\n");
                    data->state = TO_RUN;
                    break;
                    
                case TO_RUN:
                    if (TCPIP_TCP_WasDisconnected(data->socket)) {
                        data->state = TO_CLOSE;
                        break;
                    }
                    
                    telnet_out_transfer_data(scan);
                    


                    break;
                    
                    
            }
        }
//    }
}

void telnet_out_initialize() {
//    (void) xTaskCreate(
//           (TaskFunction_t) telnet_out_thread,
//           "Telnet_Out",
//           1024,   
//           NULL,
//           1U ,
//           &telnet_out_thread_handle);    
}

void telnet_out_close_port(struct port *port) {
    struct todata *data = (struct todata *)port->port_data;
    TCPIP_TCP_Close(data->socket);
    delete_port(port);
}

void telnet_out_show_detail(struct port *port, struct port *target) {
    
}

COMMAND(telnet) {
    if (argc == 0) {
        return ERR_INCOMPLETE;
    }
    

    char tmp[10];
    format_local_switch(port->local_switch, tmp, 10);
    port_printf(port, "Telnet/TCP protocol emulation 2.2  - Local Switch: <%s>.\r\n", tmp);

    struct todata *data = malloc(sizeof(struct todata));
    memset(data, 0, sizeof(struct todata));
    
    char *colon = strchr(argv[0], ':');
    if (colon > 0) {
        strncpy(data->hostname, argv[0], colon - argv[0]);
        data->port = strtoul(colon+1, NULL, 0);
    } else {
        strncpy(data->hostname, argv[0], 63);
        data->port = 23;
    }
    
    data->state = TO_DNS_PRECHECK;
    data->parent = port;
    
    struct port *slave = add_port(PORT_TELNET_OUT, data);
    slave->access = ACCESS_REMOTE;
    struct session *session = add_session(port, slave, SESSION_DIRECT);
    data->session = session;
    port->active_session = session;
    slave->active_session = session;
    port->mode = MODE_SESSION;
    slave->fn_close = &telnet_out_close_port;
    slave->fn_show_detail = &telnet_out_show_detail;
    slave->fn_flush = &telnet_out_transfer_data;
    slave->fn_yield = &telnet_out_transfer_data;
    return ERR_OK;
}

