#include "tcp_in.h"

struct tcp_in_socket sockets[] = {
    { .port_no = 3001, .target_port_no = 1, .state = TCP_IN_CLOSED },
    { .port_no = 3002, .target_port_no = 2, .state = TCP_IN_CLOSED },
    { .port_no = 3003, .target_port_no = 3, .state = TCP_IN_CLOSED },
    { .port_no = 3004, .target_port_no = 4, .state = TCP_IN_CLOSED },
    { .port_no = 3005, .target_port_no = 5, .state = TCP_IN_CLOSED },
    { .port_no = 3006, .target_port_no = 6, .state = TCP_IN_CLOSED },
};

#define NUM_SOCKETS (sizeof(sockets) / sizeof(struct tcp_in_socket))

static bool running = false;

static void tcp_in_close_port(struct port *port) {
    struct tcp_in_data *data = (struct tcp_in_data *)port->port_data;
    TCPIP_TCP_Close(data->socket->socket);
    data->socket->state = TCP_IN_DISCONNECTED;
}

void tcp_in_transfer_data(struct port *port) {
    
    
    struct tcp_in_data *data = (struct tcp_in_data *)port->port_data;
    struct tcp_in_socket *socket = (struct tcp_in_socket *)data->socket;
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

    
    xStreamBufferSend(port->read_buffer, incoming_buffer, available_bytes, 1);

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

void tcp_in_task() {
    if (!running) return;
    
    for (int sockno = 0; sockno < NUM_SOCKETS; sockno++) {
        struct tcp_in_socket *socket = &sockets[sockno];
        struct port *port = socket->port;
        struct port *target = socket->target;
        struct tcp_in_data *data = NULL;
        if (port) {
            data = (struct tcp_in_data *)port->port_data;
        }

        switch (socket->state) {
            case TCP_IN_CLOSED:
                if (socket->target == NULL) {
                    socket->target = get_port_by_number(socket->target_port_no);
                    target = socket->target;
                }

                if (target) {
                    if ((target->access == ACCESS_REMOTE) || (target->access == ACCESS_DYNAMIC)) {
                        socket->socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, socket->port_no, 0);
                        socket->state = TCP_IN_LISTEN;
                    }
                }
                break;

            case TCP_IN_LISTEN:
                if (TCPIP_TCP_IsConnected(socket->socket)) {
                 //   port_printf(CONSOLE, "Connection for port %d", socket->target_port_no);
                    data = calloc(1, sizeof(struct tcp_in_data));
                    data->socket = socket;
                    socket->port = add_port(PORT_TCP_IN, data);
                    socket->port->fn_close = &tcp_in_close_port;
                    socket->port->fn_flush = &tcp_in_transfer_data;
                    socket->port->fn_yield = &tcp_in_transfer_data;
                    port = socket->port;
                    socket->session = add_session(port, target, SESSION_DIRECT);
                    port->active_session = socket->session;
                    socket->port->mode = MODE_SESSION;
                    socket->state = TCP_IN_CONNECTED;
                }
                break;

            case TCP_IN_CONNECTED:
                if (TCPIP_TCP_WasDisconnected(socket->socket)) {
                    close_port(port);
                    break;
                }

                tcp_in_transfer_data(port);

                break;

            case TCP_IN_DISCONNECTED:
             //   port_printf(CONSOLE, "Disconnection on port %d\r\n", socket->target_port_no);
                delete_port(socket->port);
                socket->state = TCP_IN_CLOSED;
                break;
        }
    }    
}

void tcp_in_init() {
    running = true;
}