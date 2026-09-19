#include <ctype.h>

#include "port_commands.h"
#include "settings.h"
#include "port.h"
#include "uart.h"
#include "usb.h"
#include "util.h"
#include "session.h"

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