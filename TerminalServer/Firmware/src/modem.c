#include <string.h>
#include <ctype.h>

#include "modem.h"
#include "version.h"
#include "uart.h"
#include "telnet_out.h"
#include "session.h"

bool modem_response(struct port *port, enum modem_response code) {
    if (port->misc[63] & MODEM_VERBOSE) {
        switch (code) {
            case MODEM_OK:
                port_printf(port, "\r\nOK\r\n");
                break;
            case MODEM_CONNECT:
                port_printf(port, "\r\nCONNECT\r\n");
                break;
            case MODEM_RING:
                port_printf(port, "\r\nRING\r\n");
                break;
            case MODEM_NO_CARRIER:
                port_printf(port, "\r\nNO CARRIER\r\n");
                break;
            case MODEM_ERROR:
                port_printf(port, "\r\nERROR\r\n");
                break;
            case MODEM_CONNECT_1200:
                port_printf(port, "\r\nCONNECT 1200\r\n");
                break;
            case MODEM_NO_DIALTONE:
                port_printf(port, "\r\nNO DIALTONE\r\n");
                break;
            case MODEM_BUSY:
                port_printf(port, "\r\nBUSY\r\n");
                break;
            case MODEM_NO_ANSWER:
                port_printf(port, "\r\nNO ANSWER\r\n");
                break;
            case MODEM_CONNECT_2400:
                port_printf(port, "\r\nCONNECT 2400\r\n");
                break;
            case MODEM_CONNECT_4800:
                port_printf(port, "\r\nCONNECT 4800\r\n");
                break;
            case MODEM_CONNECT_9600:
                port_printf(port, "\r\nCONNECT 9600\r\n");
                break;
            case MODEM_CONNECT_14400:
                port_printf(port, "\r\nCONNECT 14400\r\n");
                break;
            case MODEM_CONNECT_19200:
                port_printf(port, "\r\nCONNECT 19200\r\n");
                break;
            case MODEM_CONNECT_1200_75:
                port_printf(port, "\r\nCONNECT 1200/75\r\n");
                break;
            case MODEM_CONNECT_75_1200:
                port_printf(port, "\r\nCONNECT 75/1200\r\n");
                break;
            case MODEM_CONNECT_7200:
                port_printf(port, "\r\nCONNECT 7200\r\n");
                break;
            case MODEM_CONNECT_12000:
                port_printf(port, "\r\nCONNECT 12000\r\n");
                break;
            case MODEM_CONNECT_38400:
                port_printf(port, "\r\nCONNECT 38400\r\n");
                break;
        }
    } else {
        port_printf(port, "%d\r", code);
    }
    return true;
}

static void modem_connect(struct port *port) {
    struct todata *data = (struct todata *)port->port_data;
    if (data->parent->type == PORT_SERIAL) {
        struct uart_data *pd = (struct uart_data *)data->parent->port_data;
        switch (pd->baud) {
            case 1200:
                modem_response(data->parent, MODEM_CONNECT_1200);
                return;
            case 2400:
                modem_response(data->parent, MODEM_CONNECT_2400);
                return;
            case 4800:
                modem_response(data->parent, MODEM_CONNECT_4800);
                return;
            case 9600:
                modem_response(data->parent, MODEM_CONNECT_9600);
                return;
            case 14400:
                modem_response(data->parent, MODEM_CONNECT_14400);
                return;
            case 19200:
                modem_response(data->parent, MODEM_CONNECT_19200);
                return;
            case 7200:
                modem_response(data->parent, MODEM_CONNECT_7200);
                return;
            case 12000:
                modem_response(data->parent, MODEM_CONNECT_12000);
                return;
            case 38400:
                modem_response(data->parent, MODEM_CONNECT_38400);
                return;
        }
    }
    modem_response(data->parent, MODEM_CONNECT);
}

static void modem_disconnect(struct port *port) {
    struct todata *data = (struct todata *)port->port_data;
    modem_response(data->parent, MODEM_NO_CARRIER);
}

static void modem_fail(struct port *port) {
    struct todata *data = (struct todata *)port->port_data;
    modem_response(data->parent, MODEM_BUSY);
}

static void modem_try(struct port *port) {
//    struct todata *data = (struct todata *)port->port_data;
}

static void modem_cancelled(struct port *port) {
    struct todata *data = (struct todata *)port->port_data;
    modem_response(data->parent, MODEM_NO_ANSWER);
}

static void modem_notfound(struct port *port) {
    struct todata *data = (struct todata *)port->port_data;
    modem_response(data->parent, MODEM_NO_ANSWER);
}

static void modem_dial_telnet(struct port *port, char *address) {

    struct todata *data = malloc(sizeof(struct todata));
    memset(data, 0, sizeof(struct todata));
    
    char *colon = strchr(address, ':');
    if (colon > 0) {
        strncpy(data->hostname, address, colon - address);
        data->port = strtoul(colon+1, NULL, 0);
    } else {
        strncpy(data->hostname, address, 63);
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
    data->on_connect = &modem_connect;
    data->on_disconnect = &modem_disconnect;
    data->on_fail = &modem_fail;
    data->on_try = &modem_try;
    data->on_cancelled = &modem_cancelled;
    data->on_notfound = &modem_notfound;
    return;
}

static void modem_dial_port(struct port *port, char *name) {
    struct port *t = get_port_by_name(name);
    if (!t) {
        uint32_t n = strtoul(name, NULL, 10);
        if (n == 0) {
            modem_response(port, MODEM_NO_ANSWER);
            return;
        }
        t = get_port_by_number(n);
        if (!t) {
            modem_response(port, MODEM_NO_ANSWER);
            return;
        }
    }
    
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->target == t) {
            modem_response(port, MODEM_BUSY);
            return;
        }
    }

    if (t->access != ACCESS_REMOTE) {
        modem_response(port, MODEM_BUSY);
        return;
    }
    
    struct session *s = add_session(port, t, SESSION_DIRECT);
    port_set_active_session(port, s);
    port_set_mode(port, MODE_SESSION);
    
//    struct todata *data = (struct todata *)t->port_data;
    if (t->type == PORT_SERIAL) {
        struct uart_data *pd = (struct uart_data *)t->port_data;
        switch (pd->baud) {
            case 1200:
                modem_response(port, MODEM_CONNECT_1200);
                return;
            case 2400:
                modem_response(port, MODEM_CONNECT_2400);
                return;
            case 4800:
                modem_response(port, MODEM_CONNECT_4800);
                return;
            case 9600:
                modem_response(port, MODEM_CONNECT_9600);
                return;
            case 14400:
                modem_response(port, MODEM_CONNECT_14400);
                return;
            case 19200:
                modem_response(port, MODEM_CONNECT_19200);
                return;
            case 7200:
                modem_response(port, MODEM_CONNECT_7200);
                return;
            case 12000:
                modem_response(port, MODEM_CONNECT_12000);
                return;
            case 38400:
                modem_response(port, MODEM_CONNECT_38400);
                return;
        }
    }
    modem_response(port, MODEM_CONNECT);
    return;
}

static bool modem_execute(struct port *port) {
    if (!port->commands[0][0]) return false;
    if (tolower(port->commands[0][0] != 'a')) return modem_response(port, MODEM_ERROR);
    if (tolower(port->commands[0][1] != 't')) return modem_response(port, MODEM_ERROR);
    
    char *ptr = &port->commands[0][2];
    
    int command_set = 0;
    int command = 0;
    int parameter = 0;
    
    while (*ptr) {
        if (( *ptr == ' ') || (*ptr == '\t')) {
            ptr++;
            continue;
        }
        if (*ptr == '&') {
            ptr++;
            command_set = 1;
            continue;
        }

        switch (command) {
            case 0:
                switch(command_set) {
                    case 0:
                        switch (tolower(*ptr)) {
                            case 'a': // Answer not implemented
                                ptr++;
                                // Skip next number if it's there
                                if (isdigit(*ptr)) {
                                    ptr++;
                                }
                                return modem_response(port, 4); 
                            case 'd':
                                ptr++;
                                if (tolower(*ptr) == 't') {
                                    ptr++;
                                    while (*ptr && (*ptr <= ' ')) ptr++;
                                    if (!*ptr) {
                                        modem_response(port, MODEM_ERROR);
                                        return true;
                                    }
                                    modem_dial_telnet(port, ptr);
                                } else if (tolower(*ptr) == 'p') {
                                    while (*ptr && (*ptr <= ' ')) ptr++;
                                    if (!*ptr) {
                                        modem_response(port, MODEM_ERROR);
                                        return true;
                                    }
                                    modem_dial_port(port, ptr);
                                }
                                return true;
                                break;
                            case 'e':
                                ptr++;
                                port->misc[63] &= ~MODEM_ECHO;
                                if (*ptr == '1') {
                                    port->misc[63] |= MODEM_ECHO;
                                }
                                if (isdigit(*ptr)) {
                                    ptr++;
                                }
                                break;
                            case 'o':
                                ptr++;
                                if (port->active_session) {
                                    port_set_mode(port, MODE_SESSION);
                                    return true;
                                }
                                modem_response(port, MODEM_ERROR);
                                return true;
                                break;
                            case 'v':
                                ptr++;
                                port->misc[63] &= ~MODEM_VERBOSE;
                                if (*ptr == '1') {
                                    port->misc[63] |= MODEM_VERBOSE;
                                }
                                if (isdigit(*ptr)) {
                                    ptr++;
                                }
                                break;
                            case 'h':
                                ptr++;
                                if (isdigit(*ptr)) {
                                    ptr++;
                                }
                                if (!port->active_session) {
                                    modem_response(port, MODEM_ERROR);
                                    return true;
                                }
                                if (port->active_session->target->fn_close) {
                                    port->active_session->target->fn_close(port->active_session->target);
                                }
                                delete_session(port->active_session);
                                port_set_active_session(port, NULL);
                                modem_response(port, MODEM_NO_CARRIER);
                                return true;
                            case 'i':
                                ptr++;
                                parameter = 0;
                                if (isdigit(*ptr)) {
                                    parameter = *ptr - '0';
                                    ptr++;
                                }
                                switch (parameter) {
                                    case 0:
                                        port_printf(port, "\r\nMajenko Technologies Terminal Server V" VERSION);
                                        break;
                                    case 1:
                                        port_printf(port, "\r\n" VERSION);
                                        break;
                                    case 2:
                                        port_printf(port, "\r\n%s", port->name);
                                        break;
                                    case 3:
                                        break;
                                    case 4:
                                        break;
                                    case 5:
                                        break;
                                    case 6:
                                        break;
                                    case 7:
                                        break;
                                    case 8:
                                        break;
                                    case 9:
                                        break;
                                }
                                break;
                            default:
                                modem_response(port, MODEM_ERROR);
                                return false;
                                ptr++;
                        }                        
                        break;
                }
                break;
            case 'd': // Everything after an ATD[x] is an address to connect to.
                
        }
        
        
    }
    
    return modem_response(port, 0);
}

void modem_process(struct port *port, int c) {
    int l;
    if (port->misc[63] & MODEM_ECHO) port_write_byte(port, c);
    switch (c) {
        case '\r':
            modem_execute(port);
            memset(port->commands[0], 0, MAX_COMMAND);
            port->commands[0][0] = 0;
            break;
        default:
            l = strlen(port->commands[0]);
            if (l < (MAX_COMMAND-1)) {
                port->commands[0][l++] = c;
                port->commands[0][l] = 0;
            }
            break;            
    }
}
