#include <errno.h>
#include "app.h"
#include "telnet_in.h"
#include "port.h"
#include "session.h"

#include "telopt.h"

//#define NUM_SOCKETS 6

//static TCP_SOCKET sockets[NUM_SOCKETS];

static struct tcp_socket sockets[NUM_TELNET_SOCKETS]; 


#define SEND_IAC(X, Y)  while (TCPIP_TCP_Put(socket->socket, TELOPT_IAC) != 1){}; \
                        while (TCPIP_TCP_Put(socket->socket, (X)) != 1){}; \
                        while (TCPIP_TCP_Put(socket->socket, (Y)) != 1){}; 

#define SEND_SB_SEND(X) while (TCPIP_TCP_Put(socket->socket, TELOPT_IAC) != 1){}; \
                        while (TCPIP_TCP_Put(socket->socket, TELOPT_SB) != 1){}; \
                        while (TCPIP_TCP_Put(socket->socket, (X)) != 1){}; \
                        while (TCPIP_TCP_Put(socket->socket, SB_SEND) != 1){}; \
                        while (TCPIP_TCP_Put(socket->socket, TELOPT_IAC) != 1){}; \
                        while (TCPIP_TCP_Put(socket->socket, TELOPT_SE) != 1){};

#define SEND_DO(X)      SEND_IAC(TELOPT_DO, X)
#define SEND_DONT(X)    SEND_IAC(TELOPT_DONT, X)
#define SEND_WILL(X)    SEND_IAC(TELOPT_WILL, X)
#define SEND_WONT(X)    SEND_IAC(TELOPT_WONT, X)

const char IAC_WILL_SUPPRESS_GA[3] = {TELOPT_IAC, TELOPT_WILL, TELOPT_SUPPRESS_GA};
const char IAC_DO_SUPPRESS_GA[3] = {TELOPT_IAC, TELOPT_DO, TELOPT_SUPPRESS_GA};
const char IAC_DONT_LINEMODE[3] = {TELOPT_IAC, TELOPT_DONT, TELOPT_LINEMODE};
const char IAC_DONT_ECHO[3] = {TELOPT_IAC, TELOPT_DONT, TELOPT_ECHO};
const char IAC_WONT_LINEMODE[3] = {TELOPT_IAC, TELOPT_WONT, TELOPT_LINEMODE};
const char IAC_WILL_ECHO[3] = {TELOPT_IAC, TELOPT_WILL, TELOPT_ECHO};
const char IAC_DO_NAWS[3] = {TELOPT_IAC, TELOPT_DO, TELOPT_NAWS};
const char IAC_DO_TTYPE[3] = {TELOPT_IAC, TELOPT_DO, TELOPT_TTYPE};
const char IAC_SB_TTYPE_SEND[6] = {TELOPT_IAC, TELOPT_SB, TELOPT_TTYPE, SB_SEND, TELOPT_IAC, TELOPT_SE};


static void telnet_in_listen_socket(struct tcp_socket *socket) {
    socket->socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, PORT_TELNET, 0);
    socket->state = SOCK_LISTEN;
    socket->port = NULL;
}

static void telnet_in_close_socket(struct tcp_socket *socket) {
    if (socket->port) {
        //close_port(socket->port);
        socket->port = NULL;
        TCPIP_TCP_Close(socket->socket);
        socket->state = SOCK_CLOSED;
    }
}

void telnet_in_close_port(struct port *port) {
    struct telnet_in_data *data = (struct telnet_in_data *)port->port_data;
    telnet_in_close_socket(data->socket);
    delete_port(port);
}




void telnet_in_transfer_data(struct port *port) {
    struct telnet_in_data *data = (struct telnet_in_data *)port->port_data;
    struct tcp_socket *socket = data->socket;
    int free_bytes;
    int available_bytes;
    uint8_t incoming_buffer[CIRCULAR_BUFFER_SIZE];

    // First, incoming data from the server. Read it and process
    // it byte by byte.

    // Get amount of space in the circular buffer
    free_bytes = xStreamBufferSpacesAvailable(port->read_buffer);
    // Get the number of pending bytes
    available_bytes = TCPIP_TCP_GetIsReady(socket->socket);

    // Limit the number of bytes available to the size of the
    // circular buffer space
    if (available_bytes > free_bytes) available_bytes = free_bytes;

    // Grab the data from the socket and put it in our processing buffer
    TCPIP_TCP_ArrayGet(socket->socket, incoming_buffer, available_bytes);

    // Process each byte in turn
    for (int byteno = 0; byteno < available_bytes; byteno++) {
        uint8_t this_byte = incoming_buffer[byteno];

        if (data->iac_sb) {
            switch (this_byte) {
                case TELOPT_IAC:
                    if (data->iac_sb_iac) {
                        data->iac_sub[data->iac_sub_pos++] = this_byte;
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
                            for (int i = 2; i < data->iac_sub_pos; i++) {
                                int x = i - 2;
                                if (x < 16) {
                                    port->ttype[x] = data->iac_sub[i];
                                    port->ttype[x+1] = 0;
                                }
                                set_terminal_type(port, port->ttype);
                            }
                            break;
                        default:
                            break;
                    }
                    data->iac_sb = false;
                    data->iac_pos = 0;
                    break;
                default:
                    data->iac_sub[data->iac_sub_pos++] = this_byte;
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
                    if (this_byte == TELOPT_IAC) {
                        data->iac[data->iac_pos++] = TELOPT_IAC;
                    } else {
                        xStreamBufferSend(port->read_buffer, &this_byte, 1, 1);
                    }
                    break;

                case 1:
                    switch (this_byte) {
                        case TELOPT_IAC:
                            xStreamBufferSend(port->read_buffer, &this_byte, 1, 1);
                            data->iac_pos = 0;                                       
                            break;
                        case TELOPT_SB:
                            data->iac_sb = true;
                            data->iac_sub_pos = 0;
                            break;
                        case TELOPT_BREAK:
                            data->iac_pos = 0;
                            switch (port->breakmode) {
                                case BREAK_LOCAL:
                                    port_printf(port, "+++ OUT OF CHEESE +++\r\n\n");
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
                        default:
                            data->iac[data->iac_pos++] = this_byte;
                            break;
                    }
                    break;

                case 2:
                    switch (this_byte) {
                        default:
                            data->iac[data->iac_pos++] = this_byte;
                            switch (data->iac[1]) {
                                case TELOPT_WILL:
                                    switch (data->iac[2]) {
                                        case TELOPT_LINEMODE:
                                            SEND_DONT(TELOPT_LINEMODE);
                                            SEND_DONT(TELOPT_ECHO);
                                            SEND_WILL(TELOPT_ECHO);
                                            break;
                                        case TELOPT_TTYPE:
                                            SEND_DO(TELOPT_TTYPE);
                                            SEND_SB_SEND(TELOPT_TTYPE);
                                            break;
                                        case TELOPT_NAWS:
                                            SEND_DO(TELOPT_NAWS);
                                            break;
                                        default:
                                            break;

                                    }
                                    break;
                                case TELOPT_WONT:
                                    break;
                                case TELOPT_DO:
                                    switch (data->iac[2]) {
                                        case TELOPT_SUPPRESS_GA:
                                            SEND_WILL(TELOPT_SUPPRESS_GA);
                                            SEND_DO(TELOPT_SUPPRESS_GA);
                                            break;
                                        default:
                                            break;
                                    }
                                    break;
                                case TELOPT_DONT:
                                    break;
                                default:
                                    break;
                            }
                            data->iac_pos = 0;
                    }
            }
        }
    }

    // Now we do similar with outgoing data from the circular
    // buffer to the server. Easier this time, nothing to
    // process - just read and pipe through to the other end.


    // Number of bytes we have to send
    available_bytes = xStreamBufferBytesAvailable(port->write_buffer);
    // Amount of space to send into
    free_bytes = TCPIP_TCP_PutIsReady(socket->socket);

    // Truncate byte count to what space there is
    if (available_bytes > free_bytes) available_bytes = free_bytes;

    xStreamBufferReceive(port->write_buffer, incoming_buffer, available_bytes, 1);
    TCPIP_TCP_ArrayPut(socket->socket, incoming_buffer, available_bytes);


}

//static void telnet_in_thread(void *args) {

void telnet_in_task() {
        for (int sockno = 0; sockno < NUM_TELNET_SOCKETS; sockno++) {
            struct tcp_socket *socket = &sockets[sockno];
            struct port *port = socket->port;
         //   struct telnet_in_data *data = NULL;
            

            
         //   if (port) {
         //       data = (struct telnet_in_data *)socket->port->port_data;
         //   }
            
            switch (socket->state) {
                case SOCK_LISTEN:
                    // If we have a new connection on a listening socket...
                    if (TCPIP_TCP_IsConnected(socket->socket)) {
                        socket->state = SOCK_CONNECTED;
                        struct telnet_in_data *data = calloc(1, sizeof(struct telnet_in_data));
                        data->socket = socket;
                        socket->port = add_port(PORT_TELNET_IN, data);
                        socket->port->fn_close = &telnet_in_close_port;
                        socket->port->fn_show_detail = &print_telnet_in_info;
                        snprintf(socket->port->name, 9, "Telnt%d", socket->port->no);
                        socket->port->name[8] = 0;
                        socket->port->breakmode = BREAK_LOCAL;
                        socket->port->fn_yield = &telnet_in_transfer_data;
                        socket->port->fn_flush = &telnet_in_transfer_data;
                        port_set_mode(socket->port, MODE_PREGREET);
                    }
                    break;
                case SOCK_CONNECTED:
                    if (TCPIP_TCP_WasDisconnected(socket->socket)) {
                        socket->state = SOCK_DISCONNECTED;
                    }
                    
                    telnet_in_transfer_data(port);
                    
                    break;
                case SOCK_DISCONNECTED:
                    telnet_in_close_socket(socket);
                    delete_port(port);
                    socket->state = SOCK_CLOSED;
                    break;
                            
                case SOCK_CLOSED:
                    telnet_in_listen_socket(socket);
                    break;
               
            }
            
        }

}

void telnet_in_initialize() {

    for (int i = 0; i < NUM_TELNET_SOCKETS; i++) {
        telnet_in_listen_socket(&sockets[i]);
    }
}

void print_telnet_in_info(struct port *port, struct port *target) {
    TCP_SOCKET_INFO info;

    struct telnet_in_data *data = (struct telnet_in_data *)port->port_data;
    
    TCPIP_TCP_SocketInfoGet(data->socket->socket, &info);
    
    char ip[16];
    snprintf(ip, 16, "%d.%d.%d.%d\r\n", 
            info.remoteIPaddress.v4Add.v[0],
            info.remoteIPaddress.v4Add.v[1],
            info.remoteIPaddress.v4Add.v[2],
            info.remoteIPaddress.v4Add.v[3]
    );            
    port_printf(port, "   Remote Address:  %15s\r\n", ip);
}