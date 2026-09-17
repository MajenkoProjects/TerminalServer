#include <string.h>
#include <ctype.h>

#include "app.h"
#include "port.h"
#include "command.h"
#include "uart.h"
#include "errors.h"
#include "telnet_in.h"
#include "settings.h"
#include "session.h"
#include "util.h"
#include "network.h"
#include "ttype.h"
#include "telnet_out.h"

COMMAND(help) {
    port_printf(port, "%s", "Sorry, I haven't written the help tree parser yet.\r\n");
    port_printf(port, "%s", "Try again at some point in the dim and distant future.\r\n");
    return ERR_OK;
}
 
 
COMMAND(logout) {
    port_printf(port, "Exiting the Majenko Technologies Terminal Server\r\n\n");
    close_port(port);
    return ERR_OK;
} 

COMMAND(show_server) {
    print_system_settings(port);
    port_printf(port, "\n");
    print_network_settings(port);
    port_printf(port, "\n");
    return ERR_OK;
}

COMMAND(set_priv) {
    port->mode = MODE_PASSWORD;
    return ERR_OK;
}

COMMAND(set_nopriv) {
    port->priv = false;
    return ERR_OK;
}

static const struct command show_port_sub[] = {
    {"CHARACTERISTICS", &show_port_characteristics, 0, NULL },
    {"STATUS", &show_port_status, 0, NULL },
    {0, 0, 0, 0}
};

static const struct command show_sub[] = {
    {"PORT", &show_port_characteristics, CMD_TARGET, show_port_sub},
    {"SERVER", &show_server, 0, NULL},
    {"SESSIONS", &show_sessions, CMD_TARGET, NULL},

    {0, 0, 0, 0}
};

static const struct command list_sub[] = {
    {"PORTS", &list_ports, 0, NULL},
    {0, 0, 0, 0}
};

static const struct command set_port_flow_sub[] = {
    {"NONE", &port_set_flow_none, 0, NULL},
    {"RTSCTS", &port_set_flow_rts, 0, NULL},
    {"DTRDSR", &port_set_flow_dtr, 0, NULL},
    {"XONXOFF", &port_set_flow_xon, 0, NULL},
    {0, 0, 0, 0}
};

static const struct command set_port_break_sub[] = {
    {"DISABLED", &port_set_break_disabled, 0, NULL},
    {"LOCAL", &port_set_break_local, 0, NULL},
    {"REMOTE", &port_set_break_remote, 0, NULL},
    {0, 0, 0, 0}
};

//static const struct command set_port_access_sub[] = {
//    {"DYNAMIC", &port_set_access_dynamic, NULL},
//    {"LOCAL", &port_set_access_local, 0, NULL},
//    {"REMOTE", &port_set_access_remote, 0, NULL},
//    {0, 0, 0, 0}
//};

static const struct command set_port_terminal_sub[] = {
    {"TYPE", &port_set_terminal_type, 0, NULL},
    {0, 0, 0, 0}
};

static const struct command set_port_local_sub[] = {
    {"SWITCH", &port_set_local_switch, 0, NULL},
    {0, 0, 0, 0}
};

static const struct command set_port_sub[] = {
    //{"ACCESS", NULL, 0, set_port_access_sub},
    {"BACKWARD", &port_set_backward_switch, 0, NULL},
    {"BREAK", NULL, 0, set_port_break_sub},
    {"FLOW", NULL, 0, set_port_flow_sub},
    {"FORWARD", &port_set_forward_switch, 0, NULL},
    {"LOCAL", &port_set_local_switch, 0, set_port_local_sub},
    //{"NAME", &port_set_name, 0, NULL},
    {"SPEED", &port_set_speed, 0, NULL},
    {"TERMINAL", NULL, 0, set_port_terminal_sub},
    {0, 0, 0, 0}
};

static const struct command set_sub[] = {
    {"NOPRIVILEGED", &set_nopriv, 0, NULL},
    {"PORT", NULL, CMD_TARGET, set_port_sub},
    {"PRIVILEGED", &set_priv, 0, NULL},
    {0, 0, 0, 0}
};


static const struct command define_port_break_sub[] = {
    {"DISABLED", &port_define_break_disabled, CMD_PRIV, NULL},
    {"LOCAL", &port_define_break_local, CMD_PRIV, NULL},
    {"REMOTE", &port_define_break_remote, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_port_flow_sub[] = {
    {"NONE", &port_define_flow_none, CMD_PRIV, NULL},
    {"RTSCTS", &port_define_flow_rts, CMD_PRIV, NULL},
    {"DTRDSR", &port_define_flow_dtr, CMD_PRIV, NULL},
    {"XONXOFF", &port_define_flow_xon, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_port_access_sub[] = {
    {"DYNAMIC", &port_define_access_dynamic, CMD_PRIV, NULL},
    {"LOCAL", &port_define_access_local, CMD_PRIV, NULL},
    {"REMOTE", &port_define_access_remote, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_port_terminal_sub[] = {
    {"TYPE", &port_define_terminal_type, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_port_sub[] = {
    {"ACCESS", NULL, 0, define_port_access_sub},
    {"BACKWARD", &port_define_backward_switch, CMD_PRIV, NULL},
    {"BREAK", NULL, 0, define_port_break_sub},
    {"FLOW", NULL, 0, define_port_flow_sub},
    {"FORWARD", &port_define_forward_switch, CMD_PRIV, NULL},
    {"LOCAL", &port_define_local_switch, CMD_PRIV, NULL},
    {"NAME", &port_define_name, CMD_PRIV, NULL},
    {"SPEED", &port_define_speed, CMD_PRIV, NULL},
    {"TERMINAL", NULL, 0, define_port_terminal_sub},
    {0, 0, 0, 0}
};

static const struct command define_server_mac_sub[] = {
    {"ADDRESS", &ethernet_define_mac_address, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_server_subnet_sub[] = {
    {"MASK", &ethernet_define_subnet, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_server_secondary_sub[] = {
    {"NAMESERVER", &ethernet_define_secdns, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_server_dhcp_sub[] = {
    {"DISABLED", &ethernet_define_dhcp_disabled, CMD_PRIV, NULL},
    {"ENABLED", &ethernet_define_dhcp_enabled, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command define_server_sub[] = {
    {"DHCP", NULL, 0, define_server_dhcp_sub},
    {"DOMAIN", &system_define_domain, CMD_PRIV, NULL},
    {"IPADDRESS", &ethernet_define_ip, CMD_PRIV, NULL},
    {"SUBNET", &ethernet_define_subnet, CMD_PRIV, define_server_subnet_sub},
    {"GATEWAY", &ethernet_define_gateway, CMD_PRIV, NULL},
    {"MAC", &ethernet_define_mac_address, CMD_PRIV, define_server_mac_sub},
    {"NAME", &system_define_name, CMD_PRIV, NULL},
    {"NAMESERVER", &ethernet_define_pridns, CMD_PRIV, NULL},
    {"SECONDARY", NULL, 0, define_server_secondary_sub},
    {0, 0, 0, 0}
};

static const struct command define_sub[] = {
    {"PORT", NULL, CMD_TARGET, define_port_sub},
    {"SERVER", NULL, 0, define_server_sub},
    {0, 0, 0, 0}
};


static const struct command send_sub[] = {
    {"BREAK", &port_send_break, 0, NULL},
    {0, 0, 0, 0}
};

static const struct command connect_sub[] = {
    {"LOCAL", &connect_local, 0, NULL},
    {"TELNET", &telnet, 0, NULL},
    {0, 0, 0, 0}
};

static const struct command initialize_sub[] = {
    {"FACTORY", &system_factory_reset, CMD_PRIV, NULL},
    {0, 0, 0, 0}
};

static const struct command commands[] = {
    {"CONNECT", NULL, 0, connect_sub},
    {"DEFINE", NULL, 0, define_sub},
    {"DISCONNECT", &disconnect_session, CMD_SESSION, NULL},
    {"HELP", &help, 0, NULL},
    {"INITIALIZE", NULL, 0, initialize_sub},
    {"LIST", NULL, 0, list_sub},
    {"LOGOUT", &logout, 0, NULL},
    {"OPEN", &telnet, 0, NULL},
    {"RESUME", &resume_session, CMD_SESSION, NULL},
    {"SEND", NULL, 0, send_sub},
    {"SET", NULL, 0, set_sub},
    {"SHOW", NULL, 0, show_sub},
    {0, 0, 0, 0}
};

const char *shift(int *argc, char **argv) {
    if (*argc == 0) return NULL;
    const char *first = argv[0];
    for (int i = 0; i < *argc - 1; i++) {
        argv[i] = argv[i + 1];
    }
    *argc = *argc - 1;
    return first;
}

void print_strarr(struct port *port, int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        port_printf(port, "(%s) ", argv[i]);
    }
    port_printf(port, "\r\n");
}

error_t command_run(const struct command *tree, struct port *port, void *opt, int argc, char **argv) {

    while (1) {
        const char *cmd = shift(&argc, argv);

        // Find a matching entry in the commands table
        int matches = 0;
        const struct command *matched = NULL;
        const struct command *exact = NULL;
        for (const struct command *scan = tree; scan->command != 0; scan++) {
            if (strncasecmp(scan->command, cmd, strlen(cmd)) == 0) {
                if (strlen(scan->command) == strlen(cmd)) {
                    exact = scan;
                }
                matched = scan;
                matches++;
            }
        }

        if (exact) {
            matched = exact;
            matches = 1;
        }
        
        // If no match found, error with unknown command
        if (matches == 0) {
            return ERR_UNKNOWN;
        }
    
        // If more than one match found, error with ambiguous command
        if (matches > 1) {
            return ERR_AMBIGUOUS;
        }

        // If found command has optional target parameter look for it and
        // parse it.
        if (matched->flags & CMD_TARGET) {
            if ((argv[0][0] >= '0') && (argv[0][0] <= '9')) {
                const char *tid = shift(&argc, argv);
                int pno = strtol(tid, NULL, 10);
                opt = get_port_by_number(pno);
                if (!opt) {
                    return ERR_BADPORT;
                }
            }
        } else if (matched->flags & CMD_SESSION) {
            if ((argv[0][0] >= '0') && (argv[0][0] <= '9')) {
                const char *tid = shift(&argc, argv);
                int pno = strtol(tid, NULL, 10);
                opt = get_session_by_number(pno);
                if (!opt) {
                    return ERR_NOSESSION;
                }
            }
        }

        // If there are no more arguments then just call the function
        if (argc == 0) {
            if (matched->func) {
                if ((matched->flags & CMD_PRIV) && !port->priv) {
                    return ERR_PRIV;
                }
                return matched->func(port, opt, 0, NULL);
            } else {
                return ERR_INCOMPLETE;
            }
        }
    
        // If found command has sub-commands check the next entry in the
        // array to see if it's in there. If it is, traverse the tree. If it
        // isn't then execute the function in the command passing the next parameters
        // as its arguments.

        bool deeper = false;
        if (matched->sub_commands) {
            for (const struct command *scan = matched->sub_commands; scan->command != 0; scan++) {
                if (strncasecmp(scan->command, argv[0], strlen(argv[0])) == 0) {
                    deeper = true;
                }
            }
        }

        if (deeper) {       
            tree = matched->sub_commands;
        } else if (matched->func) {
            if ((matched->flags & CMD_PRIV) && !port->priv) {
                return ERR_PRIV;
            }
            return matched->func(port, opt, argc, argv);
        }
    }
}

void command_execute(struct port *port) {
    int argc = 0;
    char *argv[MAX_ARGS];
    char *ptr = NULL;
    char *bit;
    
    char cmdline[MAX_COMMAND];
    strcpy(cmdline, port->commands[port->cmdno]);
    
    bit = strtok_r(cmdline, " \t", &ptr);
    while (bit && (argc < MAX_ARGS)) {
        argv[argc++] = bit;
        bit = strtok_r(NULL, " \t", &ptr);
    }

    error_t ret = command_run(commands, port, NULL, argc, argv);
    
    if (ret != ERR_OK) {
        port_printf(port, "%s\r\n", error_strings[ret]);
    }
}



int command_process(struct port *port, char c, void (*func)(struct port *)) {
    
    uint16_t buf[9];
    int ret = 0;
    int len = fancy_read(port, c, buf, 8);

    for (int i = 0; i < len; i++) {
        switch (buf[i]) {
            case SPECIAL_KEY | KEY_RETURN:
                port_printf(port, "\r\n");
                
                if (strlen(port->commands[port->cmdno]) > 0) {
                    if (port->mode == MODE_LOCAL) {
                        if (port->cmdno == 0) {
                            for (int i = NUM_HISTORY-1; i > 0; i--) {
                                strcpy(port->commands[i], port->commands[i-1]);

                            }
                        }
                    }
                    func(port);
                }
                ret = 1;
                port->cmdno = 0;
                port->commands[0][0] = 0;
                port->cpos = 0;
                break;
            case SPECIAL_KEY | KEY_BACKSPACE:
                if (port->cpos > 0) {
                    port->cpos--;
                    
                    for (int i = port->cpos; i <= strlen(port->commands[port->cmdno]); i++) {
                        port->commands[port->cmdno][i] = port->commands[port->cmdno][i+1];
                    }

                    if (port->tinfo->delchar) {
                        port_printf(port, port->tinfo->cleft);
                        port_printf(port, port->tinfo->delchar);
                    } else {
                        port_printf(port, port->tinfo->cleft);
                        port_printf(port, "%s ", &port->commands[port->cmdno][port->cpos]);
                    }
                }
                break;
            case SPECIAL_KEY | KEY_UP:
                if (port->mode != MODE_LOCAL) break;
                if (port->cmdno < NUM_HISTORY-1) {
                    port->cmdno ++;
                }
                
                if (port->priv) {
                    if (port->tinfo->clreol) {
                        port_printf(port, "\rLocal>>%s%s", port->commands[port->cmdno], port->tinfo->clreol);
                    } else {
                        port_printf(port, "\r\nLocal>>%s", port->commands[port->cmdno]);
                    }                    
                } else {
                    if (port->tinfo->clreol) {
                        port_printf(port, "\rLocal>%s%s", port->commands[port->cmdno], port->tinfo->clreol);
                    } else {
                        port_printf(port, "\r\nLocal>%s", port->commands[port->cmdno]);
                    }
                }
                port->cpos = strlen(port->commands[port->cmdno]);
                break;
            case SPECIAL_KEY | KEY_DOWN:
                if (port->mode != MODE_LOCAL) break;
                if (port->cmdno > 0) {
                    port->cmdno --;
                }
                if (port->priv) {
                    if (port->tinfo->clreol) {
                        port_printf(port, "\rLocal>>%s%s", port->commands[port->cmdno], port->tinfo->clreol);
                    } else {
                        port_printf(port, "\r\nLocal>>%s", port->commands[port->cmdno]);
                    }
                } else {
                    if (port->tinfo->clreol) {
                        port_printf(port, "\rLocal>%s%s", port->commands[port->cmdno], port->tinfo->clreol);
                    } else {
                        port_printf(port, "\r\nLocal>%s", port->commands[port->cmdno]);
                    }
                }
                port->cpos = strlen(port->commands[port->cmdno]);
                break;
            case SPECIAL_KEY | KEY_LEFT:
                if (port->cpos > 0) {
                    port->cpos--;
                    port_printf(port, port->tinfo->cleft);
                }
                break;
            case SPECIAL_KEY | KEY_RIGHT:
                if (port->cpos < strlen(port->commands[port->cmdno])) {
                    port->cpos++;
                    port_printf(port, port->tinfo->cright);
                }
                break;
            default:
                if (!IS_SPECIAL(buf[i]) && (buf[i] >= ' ')) {
                    if (strlen(port->commands[port->cmdno]) < MAX_COMMAND-1) {
                        for (int i = strlen(port->commands[port->cmdno])+1; i > port->cpos; i--) {
                            port->commands[port->cmdno][i] = port->commands[port->cmdno][i-1];
                        }
                        port->commands[port->cmdno][port->cpos++] = buf[i];
//                        port->commands[port->cmdno].command[port->cpos] = 0;
                        if (port->mode != MODE_PASSWORD) {
                            if (port->tinfo->inschar) {
                                port_printf(port, "%s%c", port->tinfo->inschar, buf[i]);
                            }
                        }
                    }
//                } else if (IS_SPECIAL(buf[i])) {
//                    port_printf(port, "Special key %04x\r\n", buf[i]);
                }
                break;
        }
    }
    return ret;
}
