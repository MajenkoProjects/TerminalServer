#include <string.h>

#include "app.h"
#include "port.h"

int command_execute(struct port *port) {
    int argc = 0;
    char *argv[MAX_ARGS];
    char *ptr = NULL;
    char *bit;
    
    bit = strtok_r(port->command, " \t", &ptr);
    while (bit && (argc < MAX_ARGS)) {
        argv[argc++] = bit;
        bit = strtok_r(NULL, " \t", &ptr);
    }
  
    for (int i = 0; i < argc; i++) {
        port_printf(port, "%d: %s\r\n", i, argv[i]);
    }
    return 0;
}

int command_process(struct port *port, char c) {
    switch (c) {
        case '\n':
            break;
        case '\r':
            port_write_byte(port, '\r');
            port_write_byte(port, '\n');
            if (port->command_len > 0) {
                command_execute(port);
            }
            port->command_len = 0;
            port->command[0] = 0;
            port_write_byte(port, '>');
            port_write_byte(port, ' ');
            break;
        case 8:
            if (port->command_len > 0) {
                port_write_byte(port, 8);
                port_write_byte(port, ' ');
                port_write_byte(port, 8);
                port->command_len--;
                port->command[port->command_len] = 0;
            }
            break;
        default:
            if (port->command_len < MAX_COMMAND-1) {
                port->command[port->command_len++] = c;
                port->command[port->command_len] = 0;
                port_write_byte(port, c);
            }
            break;
    }
    return 0;
}