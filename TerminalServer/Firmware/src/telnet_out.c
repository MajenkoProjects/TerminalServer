#include "telnet_out.h"
#include "cbuff.h"
#include "session.h"
#include "errno.h"

TaskHandle_t telnet_out_thread_handle;


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

static void telnet_out_thread(void *args) {

    int r;
    while (1) {
        for (struct port *scan = ports; scan; scan = scan->next) {
            if (scan->type != PORT_TELNET_OUT) continue;
            struct todata *data = (struct todata *)scan->port_data;
            switch (data->state) {
                case TO_START_DNS:
                    data->dns_result = TCPIP_DNS_Resolve(data->hostname, TCPIP_DNS_TYPE_A);
                    data->state = TO_RUN_DNS;          
                    break;

                case TO_RUN_DNS:
                    data->dns_result = TCPIP_DNS_IsResolved(data->hostname, &data->addr, TCPIP_DNS_TYPE_A);
                    if (data->dns_result == TCPIP_DNS_RES_OK) {
                        data->state = TO_FOUND_HOST;
                    } else if (data->dns_result == TCPIP_DNS_RES_PENDING) {
                        data->state = TO_RUN_DNS;
                    } else {
                        data->state = TO_BAD_HOST;
                    }   
                    break;
                    
                case TO_BAD_HOST:
                    port_rprintf(scan, "Host not found\r\n");
                    vTaskDelay(100);
                    session_slave_close(scan);
                    vTaskDelay(100);
                    delete_port(scan);
                    // Urk
                    break;
                   
                case TO_FOUND_HOST:
                    // Yay
                    port_rprintf(scan, "Connecting to %d.%d.%d.%d...",
                            data->addr.v4Add.v[0],
                            data->addr.v4Add.v[1],
                            data->addr.v4Add.v[2],
                            data->addr.v4Add.v[3]
                            );
                    data->state = TO_CONNECT;
                    
                    data->socket = TCPIP_TCP_ClientOpen(IP_ADDRESS_TYPE_IPV4, 23, &data->addr);
                    data->state = TO_CONNECTING;
                    
                    
                    break;
                case TO_CONNECTING:
                    if (TCPIP_TCP_IsConnected(data->socket)) {
                        data->state = TO_CONNECT;
                    }
                    break;
                case TO_NOCONN:
                    port_rprintf(scan, "Unable to connect\r\n");
                    prterror();
                    vTaskDelay(100);
                    session_slave_close(scan);
                    vTaskDelay(100);
                    delete_port(scan);
                    // Urk
                    break;
                    
                case TO_CONNECT:
                    port_rprintf(scan, "Connected\r\n");
                    data->state = TO_RUN;
                    break;
                case TO_RUN:
                    
                    if (TCPIP_TCP_GetIsReady(data->socket)) {
                        char b;
                        TCPIP_TCP_ArrayGet(data->socket, (uint8_t *)&b, 1);
                        port_printf(CONSOLE, "[%d]\r\n", b);
                    }
//                    port_printf(CONSOLE, ".");
                    //if (cb_free(&(scan->read_buffer)) > 0) {
                        
//                        r = recv(data->fd, (char *)&b, 1, 0);
                        
//                        if (r == 1) {
//                            port_printf(CONSOLE, "[%d]\r\n", b);
                      //      cb_write(&(scan->read_buffer), b);
//                        } else {
//                            prterror();
//                        }
                    //}
//                    int num = cb_available(&scan->write_buffer);
//                    if (num > 0) {
//                        if (num > CIRCULAR_BUFFER_SIZE) num = CIRCULAR_BUFFER_SIZE;
//                        char tmp[CIRCULAR_BUFFER_SIZE];
//                        for (int i = 0; i < num; i++) {
//                            tmp[i] = cb_read(&scan->write_buffer);
//                        }
//                        int count = 0;
//                        while (num > 0) {
//                            int sent = send(data->fd, &tmp[count], num, 0);
//                            if (sent != num) {
//                                if (errno != EWOULDBLOCK) {
////                                    closesocket(data->fd);
////                                    delete_port(scan);
////                                    close_port(scan);
//                                    break;
//                                }
//                            }
//                            if (sent > 0) {
//                                count += sent;
//                                num -= sent;
//                            }
//                        }
//                        vTaskDelay(10);
//                    }
                    
                    break;
                    
                    
            }
        }
    }
}

void telnet_out_initialize() {
    (void) xTaskCreate(
           (TaskFunction_t) telnet_out_thread,
           "Telnet_Out",
           1024,   
           NULL,
           1U ,
           &telnet_out_thread_handle);    
}

COMMAND(telnet) {
    if (argc != 1) {
        return ERR_INCOMPLETE;
    }
    port_printf(port, "Telnetting to %s\r\n", argv[0]);

    struct todata *data = malloc(sizeof(struct todata));
    memset(data, 0, sizeof(struct todata));
    strncpy(data->hostname, argv[0], 63);
    data->state = TO_START_DNS;
    
    struct port *slave = add_port(PORT_TELNET_OUT, data);
    slave->access = ACCESS_REMOTE;
    struct session *session = add_session(port, slave, SESSION_DIRECT);
    port->active_session = session;
    port->mode = MODE_SESSION;
    return ERR_OK;
}