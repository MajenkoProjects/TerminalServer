#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "port.h"
#include "session.h"
#include "util.h"

struct port *ports = NULL;

const char *port_types[] = {
    "Unused",
    "USB CDC/ACM",
    "EIA-232",
    "Network In",
    "Network Out",
    "Telnet Login",
    "Telnet Outbound",
    "TCP Inbound",
   
};

const char *access_names[] = {
    "Local",
    "Remote",
    "Dynamic"
};

const char *breakmode_names[] = {
    "Disabled",
    "Local",
    "Remote"
};


// Write a block of data to the port. Returns the actual number
// of bytes written to the port.
int port_write(struct port *port, uint8_t *data, size_t len) {
    size_t buf_free = xStreamBufferSpacesAvailable(port->write_buffer);
    if (buf_free == 0) return 0;
    
    if (buf_free > len) buf_free = len;
    return xStreamBufferSend(port->write_buffer, data, buf_free, 1);
}

// Read up-to len bytes from the port. Returns the actual number
// of bytes read
int port_read(struct port *port, uint8_t *data, size_t len) {    
    
    size_t avail = xStreamBufferBytesAvailable(port->read_buffer);
    if (avail == 0) return 0;
    if (avail > len) avail = len;
    return xStreamBufferReceive(port->read_buffer, data, avail, 1);
}

int port_read_byte(struct port *port) {
    if (xStreamBufferBytesAvailable(port->read_buffer) == 0) return -1;
    uint8_t b;
    xStreamBufferReceive(port->read_buffer, &b, 1, 1);
    return b;
}

int port_write_byte(struct port *port, uint8_t b) {
    if (!xStreamBufferIsFull(port->write_buffer)) {
        xStreamBufferSend(port->write_buffer, &b, 1, 1);
        return 1;
    }
    return 0;
}

struct port *add_port(enum port_type type, void *data) {
    
    struct port *scan;
    // First scan through the existing ports looking for
    // one that is flagged as unused. Use that one if found.
    
    for (scan = ports; scan; scan = scan->next) {
        if (scan->type == PORT_NONE) {
            scan->type = type;
            scan->mode = MODE_IDLE;
            scan->access = ACCESS_LOCAL;
            scan->local_switch = LOCAL_SWITCH_NONE;
            scan->forward_switch = LOCAL_SWITCH_NONE;
            scan->backward_switch = LOCAL_SWITCH_NONE;
            scan->port_data = data;
            scan->low_water = 16;
            scan->high_water = CIRCULAR_BUFFER_SIZE - 16;
            scan->waterlevel = 0;
            snprintf(scan->name, 9, "Port_%d", scan->no);
            scan->name[8] = 0;
            scan->fn_stop = NULL;
            scan->fn_start = NULL;
            scan->fn_can_tx = NULL;
            scan->fn_close = NULL;
            scan->fn_show_detail = NULL;
            scan->lines = 24;
            scan->columns = 80;
            scan->keybuf_pos = 0;
            set_terminal_type(scan, "ANSI");
            return scan;
        }
    }
    // No vacant port found - make a new one.
    
    struct port *newport = malloc(sizeof(struct port));
    if (newport == NULL) {
        return NULL;
    }
    memset(newport, 0, sizeof(struct port));
 
    newport->type = type;
    newport->mode = MODE_IDLE;
    newport->access = ACCESS_LOCAL;
    newport->local_switch = LOCAL_SWITCH_NONE;
    newport->forward_switch = LOCAL_SWITCH_NONE;
    newport->backward_switch = LOCAL_SWITCH_NONE;
    newport->port_data = data;
    newport->low_water = 16;
    newport->high_water = CIRCULAR_BUFFER_SIZE - 16;
    newport->waterlevel = 0;
    newport->fn_stop = NULL;
    newport->fn_start = NULL;
    newport->fn_can_tx = NULL;
    newport->fn_close = NULL;
    newport->fn_show_detail = NULL;
    newport->lines = 24;
    newport->columns = 80;
    newport->keybuf_pos = 0;
    newport->read_buffer = xStreamBufferCreate(CIRCULAR_BUFFER_SIZE, 1);
    newport->write_buffer = xStreamBufferCreate(CIRCULAR_BUFFER_SIZE, 1);
    set_terminal_type(newport, "ANSI");
    
    int maxno = 0;    
    for (scan = ports; scan; scan = scan->next) {
        if (scan->no > maxno) {
            maxno = scan->no;
        }        
    }
    maxno++;

    newport->no = maxno;
    snprintf(newport->name, 9, "Port_%d", newport->no);
    newport->name[8] = 0;

    if (ports == NULL) {
        ports = newport;
        return newport;
    }

    scan = ports;
    while (scan->next != NULL) {
        scan = scan->next;
    }
    scan->next = newport;
    
    return newport;
}

void delete_port(struct port *port) {
    port->type = PORT_NONE;
    port->mode = MODE_IDLE;
    if (port->port_data) free(port->port_data);
    port->port_data = NULL;
    port->cmdno = 0;
    port->name[0] = 0;
    port->username[0] = 0;
    port->waterlevel = 0;
    port->stopped = false;
    port->local_switch = LOCAL_SWITCH_NONE;
    port->breakmode = BREAK_DISABLED;
    port->access = ACCESS_LOCAL;
    port->columns = 80;
    port->lines = 24;
    port->ticks = 0;
    port->active_session = NULL;
    port->fn_stop = NULL;
    port->fn_start = NULL;
    port->fn_can_tx = NULL;
    port->fn_close = NULL;
    port->fn_show_detail = NULL;    
    port->fn_flush = NULL;
    port->fn_yield = NULL;
}

struct port *get_port_by_number(int num) {
    for (struct port *scan = ports; scan; scan = scan->next) {
        if (num == scan->no) {
            if (scan->type != PORT_NONE) {
                return scan;
            }
        }
    }
    return NULL;
}

struct port *get_port_by_name(const char *name) {
    for (struct port *scan = ports; scan; scan = scan->next) {
        if (strcasecmp(name, scan->name) == 0) {
            if (scan->type != PORT_NONE) {
                return scan;
            }
        }
    }
    return NULL;
}

int port_available(struct port *port) {
    return xStreamBufferBytesAvailable(port->read_buffer);
}

int port_printf(struct port *port, const char *fmt, ...) {
    va_list ap;
    
    // Calculate the amount of space needed
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) return -1;

    // Allocate that plus one byte on the stack
    char *str = alloca(n + 1);
    // Print it
    va_start(ap, fmt);
    vsnprintf(str, n+1, fmt, ap);
    va_end(ap);

    char *pos = str;
    int to_send = n;
    
    while (to_send > 0) {
        int sent = port_write(port, (uint8_t *)pos, to_send);
        port->fn_yield(port);
        to_send -= sent;
        pos += sent;
    }
    
    return n;
}

int port_rprintf(struct port *port, const char *fmt, ...) {
    va_list ap;
    
    // Calculate the amount of space needed
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) return -1;

    // Allocate that plus one byte on the stack
    char *str = alloca(n + 1);
    // Print it
    va_start(ap, fmt);
    vsnprintf(str, n+1, fmt, ap);
    va_end(ap);

    char *pos = str;
    int to_send = n;
    
    while (to_send > 0) {
        int sent = xStreamBufferSend(port->read_buffer, pos, to_send, 1);
        port->fn_yield(port);
        to_send -= sent;
        pos += sent;
    }
    return n;
}

void port_flush(struct port *port) {
    if (port->fn_flush) {
        port->fn_flush(port);
    }
}

void greet(struct port *port) {
    port_printf(port, "\r\n");
    port_printf(port, "\r\n");
    port_printf(port, "Majenko Technologies Terminal Server V" VERSION "\r\n");
    port_printf(port, "\r\n");
    port_printf(port, "Type HELP at the 'Local>' prompt for assistance.\r\n");
    port_printf(port, "\r\n");
    port->mode = MODE_USERNAME;
}

void port_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data) {
    struct port *port = get_port_by_number(index);
    char temp[17] = {0};
    if (port) {
        switch (parameter) {
            case SETTING_PORT_BREAKMODE:
                port->breakmode = *(uint8_t *)data;
                break;
            case SETTING_PORT_ACCESS:
                port->access = *(uint8_t *)data;
                break;
            case SETTING_PORT_TERMINAL_TYPE:
                if (length > 16) length = 16;
                memcpy(temp, data, length);
                set_terminal_type(port, temp);
                break;
            case SETTING_PORT_LOCAL_SWITCH:
                port->local_switch = *(int *)data;
                break;
            case SETTING_PORT_FORWARD_SWITCH:
                port->forward_switch = *(int *)data;
                break;
            case SETTING_PORT_BACKWARD_SWITCH:
                port->backward_switch = *(int *)data;
                break;
        }
    }
}

void close_port(struct port *port) {
  //  if (xSemaphoreTake(port->mutex, MUTEX_TICKS) == pdTRUE) {
        if (port->type != PORT_NONE) {
            port->mode = MODE_IDLE;
            port->priv = false;
            port->username[0] = 0;
            destroy_sessions(port);
            if (port->fn_close) {
                port->fn_close(port);
            }
        }
   //     xSemaphoreGive(port->mutex);
   // }
}

const char *port_type(struct port *port) {
    return port_types[port->type];
}

void set_terminal_type(struct port *port, const char *ttype) {
  //  if (xSemaphoreTake(port->mutex, MUTEX_TICKS) == pdTRUE) {
        char *ucname = alloca(strlen(ttype) + 1);
        for (int i = 0; i < strlen(ttype); i++) {
            ucname[i] = toupper(ttype[i]);
            ucname[i+1] = 0;
        }
    
        strncpy(port->ttype, ucname, 16);
        port->tinfo = &ttype_ansi; // Default

        // First look for an exact match
        for (int i = 0; ttype_map[i].name != 0; i++) {
            if (strcmp(ttype_map[i].name, ucname) == 0) {
                port->tinfo = ttype_map[i].ttype;
                return;
            }
        }

        // Now look for a prefix match for terminal types that allow it
        for (int i = 0; ttype_map[i].name != 0; i++) {
            if (ttype_map[i].prefix == true) {
                if (strncmp(ttype_map[i].name, ucname, strlen(ttype_map[i].name)) == 0) {
                    port->tinfo = ttype_map[i].ttype;
                    return;
                }
            }
        }
  //      xSemaphoreGive(port->mutex);
  //  }
}

void port_set_active_session(struct port *port, struct session *session) {
  //  if (xSemaphoreTake(port->mutex, MUTEX_TICKS) == pdTRUE) {
        port->active_session = session;
   //     xSemaphoreGive(port->mutex);
  //  }
}

void port_set_mode(struct port *port, enum port_mode mode) {
  //  if (xSemaphoreTake(port->mutex, MUTEX_TICKS) == pdTRUE) {
        port->mode = mode;
 //       xSemaphoreGive(port->mutex);
 //   }    
}