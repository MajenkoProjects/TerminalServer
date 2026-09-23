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
#include "port_commands.h"



#include "command_tree.h"

 
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
    port->cstate = CMD_ASKPRIVPASS;
    return ERR_OK;
}

COMMAND(set_nopriv) {
    port->priv = false;
    return ERR_OK;
}


const char *shift(int *argc, char **argv) {
    if (*argc == 0) return NULL;
    const char *first = argv[0];
    for (int i = 0; i < *argc - 1; i++) {
        argv[i] = argv[i + 1];
    }
    *argc = *argc - 1;
    return first;
}

enum cmds {
    CMD_FOUND = 0,
    CMD_HAS_SUB,
    CMD_NOT_FOUND,
};

enum cmds find_command_in_list(const char *command, const struct command *list, const struct command **ptr) {
    for (int i = 0; list[i].command != 0; i++) {
        if (strcasecmp(command, list[i].command) == 0) {
            *ptr = &list[i];
            if (list[i].sub_commands != NULL) {
                return CMD_HAS_SUB;
            }
            return CMD_FOUND;
        }
    }
    return CMD_NOT_FOUND;
}

COMMAND(help) {
    
    const struct command *ptr = commands;
    const struct command *lastptr = NULL;
    
    while (argc > 0) {
        const char *cmd = shift(&argc, argv);

        const struct command *fptr;
        enum cmds res = find_command_in_list(cmd, ptr, &fptr);
        
        switch (res) {
            case CMD_FOUND:
                if (fptr->help == NULL) {
                    port_printf(port, "Sorry, help has not been written for that yet.\r\n");
                    return ERR_OK;
                }
                port_printf(port, "\r\n%s\r\n", fptr->help);
                return ERR_OK;
            case CMD_NOT_FOUND:
                port_printf(port, "No help found for %s\r\n", cmd);
                return ERR_OK;
            case CMD_HAS_SUB:
                lastptr = ptr;
                ptr = fptr->sub_commands;
                break;
        }
    }
    
    port_printf(port, "\r\nAvailable sub-commands:\r\n");
    int offset = 0;
    if (lastptr && (lastptr->flags & CMD_TARGET)) {
        port_printf(port, "\r\n    %-15s", "(port)");
        offset++;
    }

    if (lastptr && (lastptr->flags & CMD_SESSION)) {
        port_printf(port, "\r\n    %-15s", "(session)");
        offset++;
    }
    
    
    for (int i = 0; ptr[i].command != 0; i++) {
        if (((i + offset) % 3) == 0) {
            port_printf(port, "\r\n");
        }
        port_printf(port, "    %-15s", ptr[i].command);
    }
    port_printf(port, "\r\n\n");
    
    return ERR_OK;
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

static void chpass1(struct port *port) {
}

static void chpass2(struct port *port) {
}










static const struct command_state cstates[] = {
    { &command_execute, "Local>", "Local>>", true },
    { &input_username, "Username>", "Username>", true },
    { &input_password, "Password>", "Password>", false },
    { &chpass1, "Password>", "Password>", false },
    { &chpass2, "Repeat>", "Repeat>", false },  
};

const char *prompt(struct port *port) {
    if (port->priv) {
        return cstates[port->cstate].privprompt;
    } else {
        return cstates[port->cstate].prompt;
    }
}
int command_process(struct port *port, char c) {
    
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
                    cstates[port->cstate].fn_execute(port);
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
                
                if (port->tinfo->clreol) {
                    port_printf(port, "\r%s%s%s", prompt(port), port->commands[port->cmdno], port->tinfo->clreol);
                } else {
                    port_printf(port, "\r\n%s%s", prompt(port), port->commands[port->cmdno]);
                }                    

                port->cpos = strlen(port->commands[port->cmdno]);
                break;
            case SPECIAL_KEY | KEY_DOWN:
                if (port->mode != MODE_LOCAL) break;
                if (port->cmdno > 0) {
                    port->cmdno --;
                }
                if (port->tinfo->clreol) {
                    port_printf(port, "\r%s%s%s", prompt(port), port->commands[port->cmdno], port->tinfo->clreol);
                } else {
                    port_printf(port, "\r\n%s%s", prompt(port), port->commands[port->cmdno]);
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
                        if (cstates[port->cstate].echo) {
                            if (port->tinfo->inschar) {
                                port_printf(port, "%s%c", port->tinfo->inschar, buf[i]);
                            }
                        }
                    }
                }
                break;
        }
    }
    return ret;
}
