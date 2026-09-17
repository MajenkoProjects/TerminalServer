#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "app.h"
#include "port.h"
#include "leds.h"
#include "pin.h"
#include "uart.h"
#include "command.h"
#include "usb.h"
#include "settings.h"
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
    size_t buf_free = cb_free(&(port->write_buffer));
    if (len > buf_free) {
        len = buf_free;
    }
    
    for (int i = 0; i < len; i++) {
        cb_write(&port->write_buffer, data[i]);
    }
    return len;
}

// Read up-to len bytes from the port. Returns the actual number
// of bytes read
int port_read(struct port *port, uint8_t *data, size_t len) {    
    size_t avail = cb_available(&port->read_buffer);
    if (len > avail) {
        len = avail;
    }
    
    for (int i = 0; i < len; i++) {
        data[i] = cb_read(&port->read_buffer);
    }
    return len;
}

int port_read_byte(struct port *port) {
    return cb_read(&(port->read_buffer));
}

int port_write_byte(struct port *port, uint8_t b) {
    return cb_write(&(port->write_buffer), b);
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
    newport->read_buffer.mutex = xSemaphoreCreateMutex();
    newport->write_buffer.mutex = xSemaphoreCreateMutex();
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
    port->read_buffer.head = 0;
    port->read_buffer.tail = 0;
    port->write_buffer.head = 0;
    port->write_buffer.tail = 0;
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
    return cb_available(&port->read_buffer);
}

int port_printf(struct port *port, const char *fmt, ...) {
    va_list ap;
    int count = 0;
    
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

    // Send it to the output buffer
    for (int i = 0; i < n; i++) {
        while (cb_free(&port->write_buffer) < 3) {
            vTaskDelay(1);
        }
        cb_write(&port->write_buffer, str[i]);
        count++;
    }

    return count;
}

int port_rprintf(struct port *port, const char *fmt, ...) {
    va_list ap;
    int count = 0;
    
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

    // Send it to the output buffer
    for (int i = 0; i < n; i++) {
        while (cb_free(&port->read_buffer) < 3) {
            vTaskDelay(1);
        }
        cb_write(&port->read_buffer, str[i]);
        count++;
    }

    return count;
}

void port_flush(struct port *port) {
    while (cb_available(&port->read_buffer)) {
        vTaskDelay(1);
    }
    while (cb_available(&port->write_buffer)) {
        vTaskDelay(1);
    }
    if (port->type == PORT_SERIAL) {
        uart_flush(port->port_data);
    }
}



int debugf(const char *fmt, ...) {
    va_list ap;
    int count = 0;
    
    struct port *port = &ports[0];
    
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

    struct uart_data *data = port->port_data;
    data->fn_write((uint8_t *)str, n);
    
    return count;
}

COMMAND(port_set_speed) {
    OPT_TARGET
    if (target->type != PORT_SERIAL) {
        return ERR_BADPORT;
    }
    
    uart_set_baud(target, strtoul(argv[1], NULL, 10));
    return ERR_OK;
}

COMMAND(port_set_name) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_set_name(target, argv[1]);
            return ERR_OK;
        case PORT_CDC:
            usb_set_name(target, argv[1]);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}


COMMAND(port_define_speed) {
    OPT_TARGET
    if (target->type != PORT_SERIAL) {
        return ERR_BADPORT;
    }
    
    uart_define_baud(target, strtoul(argv[1], NULL, 10));
    return ERR_OK;
}

COMMAND(port_define_name) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_define_name(target, argv[1]);
            return ERR_OK;
//        case PORT_CDC:
//            usb_define_name(target, argv[1]);
//            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(show_port_characteristics) {
    OPT_TARGET
    if (!target) return ERR_BADPORT;
    port_printf(port, "\n");
    port_printf(port, "Port %-2d: Username: %-8s            Physical Port %d (%s)\r\n", target->no, target->username, target->no, port_types[target->type]);
    port_printf(port, "\n");
    
    if (target->fn_show_detail) {
        target->fn_show_detail(port, target);
    }

    port_printf(port, "\n");
    
    
    char tmp[20];
    format_local_switch(target->local_switch, tmp, 20);
    port_printf(port, "   Access:                 %8s    Local Switch:          %10s\r\n",
        access_names[target->access], format_local_switch(target->local_switch, tmp, 20)
    );
    port_printf(port, "   Backward:             %10s    Port Name:               %8s\r\n",
        format_local_switch(target->backward_switch, tmp, 20), target->name
    );
    port_printf(port, "   Low Watermark:               %3d    High Watermark:               %3d\r\n",
        target->low_water, target->high_water
    );
    port_printf(port, "   Break Ctrl:             %8s    Session Limit:                %3d\r\n",
        breakmode_names[port->breakmode], 4
    );
    port_printf(port, "   Lines:                     %5d    Columns:                    %5d\r\n", 
        target->lines, target->columns
    );
    port_printf(port, "   Forward:              %10s    Terminal Type: [%16s]\r\n", 
        format_local_switch(target->forward_switch, tmp, 20), target->ttype
    );

    port_printf(port, "\n");

    return ERR_OK;
}

COMMAND(show_port_status) {
    OPT_TARGET
    switch (port->type) {
        case PORT_SERIAL:
            uart_show_status(port, target);
            break;
        default:
            port_printf(port, "No status for this port type\r\n");
            break;
    }
    return ERR_OK;
}

COMMAND(list_ports) {
    for (struct port *scan = ports; scan; scan = scan->next) {
        if (scan->type != PORT_NONE) {
            port_printf(port, "%-2d | %-16s | %s\r\n", scan->no, port_types[scan->type], scan->name);
        }
    }
    return ERR_OK;
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

COMMAND(port_set_flow_none) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_set_flow(target, UART_FLOW_NONE);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(port_set_flow_rts) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_set_flow(target, UART_FLOW_RTSCTS);
            target->fn_start(target);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(port_set_flow_dtr) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_set_flow(target, UART_FLOW_DTRDSR);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(port_set_flow_xon) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_set_flow(target, UART_FLOW_XONXOFF);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}





COMMAND(port_define_flow_none) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_define_flow(target, UART_FLOW_NONE);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(port_define_flow_rts) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_define_flow(target, UART_FLOW_RTSCTS);
            target->fn_start(target);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(port_define_flow_dtr) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_define_flow(target, UART_FLOW_DTRDSR);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(port_define_flow_xon) {
    OPT_TARGET
    switch (target->type) {
        case PORT_SERIAL:
            uart_define_flow(target, UART_FLOW_XONXOFF);
            return ERR_OK;
        default:
            return ERR_BADPORT;
    }
}

COMMAND(port_send_break) {
    if (!port->active_session) return ERR_NOSESSION;
    port->active_session->target->send_break = true;
    return ERR_OK;
}

COMMAND(port_set_break_disabled) {
    OPT_TARGET
    target->breakmode = BREAK_DISABLED;
    return ERR_OK;
}

COMMAND(port_set_break_local) {
    OPT_TARGET
    target->breakmode = BREAK_LOCAL;
    return ERR_OK;
}

COMMAND(port_set_break_remote) {
    OPT_TARGET
    target->breakmode = BREAK_REMOTE;
    return ERR_OK;
}

COMMAND(port_define_break_disabled) {
    OPT_TARGET
    if (target->type != PORT_SERIAL) return ERR_BADPORT;
    uint8_t pm = BREAK_DISABLED;
    setting_set(MODULE_PORT, SETTING_PORT_BREAKMODE, port->no, 1, &pm);
    return ERR_OK;
}

COMMAND(port_define_break_local) {
    OPT_TARGET
    if (target->type != PORT_SERIAL) return ERR_BADPORT;
    uint8_t pm = BREAK_LOCAL;
    setting_set(MODULE_PORT, SETTING_PORT_BREAKMODE, port->no, 1, &pm);
    return ERR_OK;
}

COMMAND(port_define_break_remote) {
    OPT_TARGET
    if (target->type != PORT_SERIAL) return ERR_BADPORT;
    uint8_t pm = BREAK_REMOTE;
    setting_set(MODULE_PORT, SETTING_PORT_BREAKMODE, port->no, 1, &pm);
    return ERR_OK;
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

COMMAND(port_set_access_local) {
    OPT_TARGET
    target->access = ACCESS_LOCAL;
    target->mode = MODE_IDLE;
    return ERR_OK;
}

COMMAND(port_set_access_remote) {
    OPT_TARGET
    target->access = ACCESS_REMOTE;
    target->mode = MODE_IDLE;
    return ERR_OK;
}

COMMAND(port_set_access_dynamic) {
    OPT_TARGET
    target->access = ACCESS_DYNAMIC;
    target->mode = MODE_IDLE;
    return ERR_OK;
}



COMMAND(port_define_access_local) {
    OPT_TARGET
    uint8_t b = ACCESS_LOCAL;
    setting_set(MODULE_PORT, SETTING_PORT_ACCESS, target->no, 1, &b);
    return ERR_OK;
}

COMMAND(port_define_access_remote) {
    OPT_TARGET
    uint8_t b = ACCESS_REMOTE;
    setting_set(MODULE_PORT, SETTING_PORT_ACCESS, target->no, 1, &b);
    return ERR_OK;
}

COMMAND(port_define_access_dynamic) {
    OPT_TARGET
    uint8_t b = ACCESS_DYNAMIC;
    setting_set(MODULE_PORT, SETTING_PORT_ACCESS, target->no, 1, &b);
    return ERR_OK;
}


void close_port(struct port *port) {
    destroy_sessions(port);
    port->mode = MODE_IDLE;
    port->priv = false;
    port->username[0] = 0;
    if (port->fn_close) {
        port->fn_close(port);
    }
}

const char *port_type(struct port *port) {
    return port_types[port->type];
}

void set_terminal_type(struct port *port, const char *ttype) {
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
}

COMMAND(port_set_terminal_type) {
    OPT_TARGET
    if (argc != 1) {
        return ERR_INCOMPLETE;
    }
    set_terminal_type(target, argv[0]);
    return ERR_OK;
}

COMMAND(port_define_terminal_type) {
    OPT_TARGET
    if (argc != 1) {
        return ERR_INCOMPLETE;
    }
    
    char *ucname = alloca(strlen(argv[0]) + 1);
    for (int i = 0; i < strlen(argv[0]); i++) {
        ucname[i] = toupper(argv[0][i]);
        ucname[i+1] = 0;
    }

    setting_set(MODULE_PORT, SETTING_PORT_TERMINAL_TYPE, target->no, strlen(ucname), (uint8_t *)ucname);
    return ERR_OK;
}

COMMAND(port_set_local_switch) {
    OPT_TARGET
    if (argc != 1) return ERR_INCOMPLETE;
    int key = parse_local_switch(argv[0]);
    if (key == LOCAL_SWITCH_ERROR) {
        return ERR_INVALID;
    }
    target->local_switch = key;
    return ERR_OK;
}

COMMAND(port_set_forward_switch) {
    OPT_TARGET
    if (argc != 1) return ERR_INCOMPLETE;
    int key = parse_local_switch(argv[0]);
    if (key == LOCAL_SWITCH_ERROR) {
        return ERR_INVALID;
    }
    target->forward_switch = key;
    return ERR_OK;
}

COMMAND(port_set_backward_switch) {
    OPT_TARGET
    if (argc != 1) return ERR_INCOMPLETE;
    int key = parse_local_switch(argv[0]);
    if (key == LOCAL_SWITCH_ERROR) {
        return ERR_INVALID;
    }
    target->backward_switch = key;
    return ERR_OK;
}



COMMAND(port_define_local_switch) {
    OPT_TARGET
    if (argc != 1) return ERR_INCOMPLETE;
    int key = parse_local_switch(argv[0]);
    if (key == LOCAL_SWITCH_ERROR) {
        return ERR_INVALID;
    }
    
    setting_set(MODULE_PORT, SETTING_PORT_LOCAL_SWITCH, target->no, 4, (uint8_t *)&key);
    return ERR_OK;
}

COMMAND(port_define_forward_switch) {
    OPT_TARGET
    if (argc != 1) return ERR_INCOMPLETE;
    int key = parse_local_switch(argv[0]);
    if (key == LOCAL_SWITCH_ERROR) {
        return ERR_INVALID;
    }
    setting_set(MODULE_PORT, SETTING_PORT_FORWARD_SWITCH, target->no, 4, (uint8_t *)&key);
    return ERR_OK;
}

COMMAND(port_define_backward_switch) {
    OPT_TARGET
    if (argc != 1) return ERR_INCOMPLETE;
    int key = parse_local_switch(argv[0]);
    if (key == LOCAL_SWITCH_ERROR) {
        return ERR_INVALID;
    }
    setting_set(MODULE_PORT, SETTING_PORT_BACKWARD_SWITCH, target->no, 4, (uint8_t *)&key);
    return ERR_OK;
}