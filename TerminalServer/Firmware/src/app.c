#include <string.h>
#include <fcntl.h>
#include <stdio.h>
//#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "app.h"
#include "usb.h"
#include "port.h"
#include "task.h"
#include "pin.h"
#include "uart.h"
#include "command.h"
#include "telnet_in.h"
#include "settings.h"
#include "session.h"
#include "network.h"
#include "telnet_out.h"
#include "util.h"


struct module {
    void (*fn_boot)();
    void (*fn_init)();
};

void system_greeter();
void final_boot_message();

// These function pointers define the boot sequence. First all the
// functions on the left are executed in order, then the stored settings
// are loaded from the EEPROM chip, then the functions on the right are
// executed in order.
static const struct module modules[] = {
    { &pin_init,                NULL },
    { &system_init_defaults,    NULL }, 
    { &uart_create_ports,       &uart_boot },
    { NULL,                     &system_greeter },
    { &usb_create_ports,        &USB_Initialize },
    { &ethernet_init_defaults,  &ethernet_boot },
    { NULL,                     &telnet_in_initialize },
    { NULL,                     &telnet_out_initialize },
    { NULL,                     &final_boot_message },
};

#define NUM_MODULES (sizeof(modules) / sizeof(struct module))

extern      ssize_t write(int fildes, const void *buf, size_t nbyte);
extern      int close(int fildes);

static enum app_state state = APP_STATE_BOOT;

void system_greeter() {
    port_printf(CONSOLE, "\x0c\n\nMajenko Technologies Terminal Server V" VERSION "\r\n");
    port_printf(CONSOLE, "(c) 2026 Majenko Technologies, All Rights Reserved\r\n");
    port_printf(CONSOLE, "\r\n\n\n");
}

void final_boot_message() {
    port_printf(CONSOLE, "\nSystem initialized. Press <RETURN> to activate console.\r\n\n");

}

void input_username(struct port *port) {
    
    char *username = strtok(port->commands[port->cmdno], " \t");
    if (!username) {        
        return;
    }
    if (strlen(username) == 0) {
        return;        
    }
    memset(port->username, 0, 9);
    int l = strlen(port->commands[port->cmdno]);
    if (l > 8) l = 8;
    memcpy(port->username, port->commands[port->cmdno], l);
    port->mode = MODE_LOCAL;
}

void input_password(struct port *port) {
    if (strcmp(port->commands[port->cmdno], system_settings.password) == 0) {
        port->priv = true;
    } else {
        port_printf(port, "%Error: Incorrect password.\r\n");
        port->priv = false;
    }
    port->mode = MODE_LOCAL;
}

void APP_Initialize ( void ) {
}

void APP_Tasks ( void ) {    
    static uint32_t reset_ts = 0;
    static bool reset_state = true;
    pin_get(&pins[FACTORY_RESET]);
    if (pin_get(&pins[FACTORY_RESET]) != reset_state) {
        reset_state = pin_get(&pins[FACTORY_RESET]);
        
        if (reset_state == false) {
            reset_ts = xTaskGetTickCount();
            port_printf(CONSOLE, "Keep holding RESET for 30 seconds to factory reset.\r\n");
        }
        vTaskDelay(50);
    } 
    
    if ((reset_state == false) && ((xTaskGetTickCount() - reset_ts) > 30000)) {
        port_printf(CONSOLE, "Erasing NVRAM and rebooting. Please wait.\r\n");
        settings_erase();
        SYS_RESET_SoftwareReset();
    }
    
    switch ( state ) {
        case APP_STATE_BOOT:   
            for (int i = 0; i < NUM_MODULES; i++) {
                if (modules[i].fn_boot) modules[i].fn_boot();
            }
            state = APP_STATE_LOAD_SETTINGS;
            break;
            
        case APP_STATE_LOAD_SETTINGS:
            load_settings(); 
            state = APP_STATE_INIT;
            break;
            
        case APP_STATE_INIT:
            for (int i = 0; i < NUM_MODULES; i++) {
                if (modules[i].fn_init) modules[i].fn_init();
            }
            state = APP_STATE_SERVICE_TASKS;
            break;
            
        case APP_STATE_SERVICE_TASKS: 
            for (struct port *scan = ports; scan; scan = scan->next) {
                if (scan->type != PORT_NONE) {
                    bool have_prompted = false;
                    if (scan->mode != scan->previous_mode) {
                        scan->previous_mode = scan->mode;
                        if (scan->mode == MODE_LOCAL) {
                            if (scan->priv) {
                                port_printf(scan, "Local>>");
                            } else {
                                port_printf(scan, "Local>");
                            }
                            have_prompted = true;
                        }
                        if (scan->mode == MODE_USERNAME) {
                            port_printf(scan, "Username>");
                            have_prompted = true;
                        }
                        if (scan->mode == MODE_PASSWORD) {
                            port_printf(scan, "Password>");
                            have_prompted = true;
                        }
                    }
                    
                    switch (scan->mode) {
                        
                        // A port is sitting doing nothing. If it's a serial port
                        // and in local access mode then respond to a RETURN
                        // keypress to initiate a login prompt.
                        case MODE_IDLE:
                            if (scan->access == ACCESS_LOCAL) {
                                if (scan->type == PORT_SERIAL) {
                                    if (port_available(scan)) {
                                        int c = port_read_byte(scan);
                                        if (c == 13) {
                                            port_set_mode(scan, MODE_GREET);
                                        }
                                    }
                                }
                            }
                            break;

                        // A short delay before presenting the greeting and
                        // login prompt. Really only for telnet-in.
                        case MODE_PREGREET: 
                            if (scan->ticks == 0) {
                                scan->ticks = xTaskGetTickCount();
                            } else if (xTaskGetTickCount() - scan->ticks > 500) {
                                scan->ticks = 0;
                                port_set_mode(scan, MODE_GREET);
                            }
                            break;

                        // The main greeting and login prompt display.
                        case MODE_GREET:
                            if (scan->access == ACCESS_LOCAL) {
                                greet(scan);
                            }
                            break;

                        // Reading in the username and recording it in the port
                        case MODE_USERNAME:
                            if (scan->access == ACCESS_LOCAL) {
                                if (port_available(scan)) {
                                    int c = port_read_byte(scan);
                                    if (command_process(scan, c, &input_username) == 1) {
                                        if ((!have_prompted) && (scan->mode == MODE_USERNAME)) {
                                            port_printf(scan, "Username> ");
                                            have_prompted = true;
                                        }
                                    }
                                }
                            }
                            break;

                        // Reading in the password and verifying it against the
                        // stored password for PRIV operation
                        case MODE_PASSWORD:
                            if (scan->access == ACCESS_LOCAL) {
                                if (port_available(scan)) {
                                    int c = port_read_byte(scan);
                                    command_process(scan, c, &input_password);
                                }
                            }
                            break;

                        // Main Local> prompt processing mode. Deal with all commands
                        // entered.
                        case MODE_LOCAL:
                            if (scan->access == ACCESS_LOCAL) {
                                if (port_available(scan)) {
                                    int c = port_read_byte(scan);
                                    if (command_process(scan, c, &command_execute) == 1) {
                                        if ((!have_prompted) && (scan->mode == MODE_LOCAL)) {
                                            if (scan->priv) {
                                                port_printf(scan, "Local>>");
                                            } else {
                                                port_printf(scan, "Local>");
                                            }
                                            have_prompted = true;
                                        }
                                    }
                                }
                            }
                            break;
                            
                        // Session mode - pass data from the parent to the target
                        // and back from the target to the parent. Deal with
                        // local switch keypresses.
                        case MODE_SESSION:
                            if (scan->active_session && (scan->active_session->type == SESSION_DIRECT)) {
                                if (port_available(scan) && (cb_free(&scan->active_session->target->write_buffer))) {
                                    int c = port_read_byte(scan);
                                    uint16_t tmp[11];
                                    int r = fancy_read(scan, c, tmp, 10);
                                    for (int i = 0; i < r; i++) {
                                        if (tmp[i] == scan->local_switch) {
                                            port_printf(scan, "+++ OUT OF CHEESE ERROR +++\r\n");
                                            port_set_mode(scan, MODE_LOCAL);
                                        } else if (tmp[i] == scan->forward_switch) {
                                            struct session *first = NULL;
                                            struct session *curr = NULL;
                                            struct session *next = NULL;
                                            for (struct session *sess = sessions; sess; sess = sess->next) {
                                                if (sess->type == SESSION_DELETED) continue;
                                                if (sess->parent == scan) {
                                                    if (first == NULL) first = sess;
                                                    if (sess == scan->active_session) {
                                                        curr = sess;
                                                        continue;
                                                    }
        
                                                    if ((next == NULL) && (curr != NULL)) {
                                                        next = sess;
                                                        continue;
                                                    }
                                                }
                                            }
                                            if (next == NULL) next = first;
                                            port_set_active_session(scan, next);
                                        } else if (tmp[i] == scan->backward_switch) {
                                            struct session *prev = NULL;
                                            struct session *curr = NULL;
                                            struct session *last = NULL;
                                            for (struct session *sess = sessions; sess; sess = sess->next) {
                                                if (sess->type == SESSION_DELETED) continue;
                                                if (sess->parent == scan) {
                                                    last = sess;
                                                    if (sess == scan->active_session) {
                                                        curr = sess;
                                                        continue;
                                                    }
                                                    if ((curr == NULL)) {
                                                        prev = sess;
                                                        continue;
                                                    }
                                                }
                                            }
                                            if (prev == NULL) prev = last;
                                            port_set_active_session(scan, prev);
                                            
                                        } else {
                                            if (IS_SPECIAL(tmp[i])) {
                                                const char *key = scan->tinfo->keys[tmp[i] & 0xFF];
                                                for (int j = 0; j < strlen(key); j++) {
                                                    port_write_byte(scan->active_session->target, key[j]);                                                    
                                                }
                                            } else {
                                                port_write_byte(scan->active_session->target, tmp[i]);
                                            }
                                        }
                                    }
                                }
                                if (port_available(scan->active_session->target) && cb_free(&scan->write_buffer)) {
                                    int c = port_read_byte(scan->active_session->target);
                                    port_write_byte(scan, c);               
                                }
                            }
                            break;
 
                        default:
                            break;
                    }
                }
            }
            break;
       
        default:
            break;
    }

}

