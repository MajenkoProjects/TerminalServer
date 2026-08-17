#ifndef _PORT_H    
#define _PORT_H

#include "definitions.h"
#include "command.h"
#include <semphr.h>


#define PORT_BUFFER_SIZE    64
#define PORT_MAX_NAME       8
#define MAX_PORTS           20

enum port_mode {
    MODE_INTERACTIVE = 0,
    MODE_PIPE
};

enum port_type {
    PORT_NONE = 0,
    PORT_CDC,
    PORT_SERIAL,
    PORT_NET_IN,
    PORT_NET_OUT
};

struct circular_buffer {
    uint8_t data[PORT_BUFFER_SIZE];
    int head;
    int tail;
    SemaphoreHandle_t mutex;
};

struct port {
    enum port_type type;
    enum port_mode mode;
    int remote_port;
    struct circular_buffer read_buffer;
    struct circular_buffer write_buffer;       
    void *port_data;
    char command[MAX_COMMAND];
    int command_len;
    int no;
};

extern struct port ports[MAX_PORTS];


extern struct port *add_port(enum port_type type, void *data);
extern int cb_write(struct circular_buffer *buf, uint8_t b);
extern int cb_read(struct circular_buffer *buf);
extern int cb_free(struct circular_buffer *buf);
extern int cb_available(struct circular_buffer *buf);
extern int port_read_byte(struct port *port);
extern int port_write_byte(struct port *port, uint8_t b);
extern int port_available(struct port *port);
extern int cb_available(struct circular_buffer *buf);
extern int port_printf(struct port *port, const char *fmt, ...);
#endif 

