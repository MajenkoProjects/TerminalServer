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
#include "leds.h"
#include "pin.h"
#include "uart.h"
#include "command.h"
#include "telnet_in.h"
#include "settings.h"
#include "session.h"
#include "network.h"
#include "telnet_out.h"
#include "util.h"

extern      ssize_t write(int fildes, const void *buf, size_t nbyte);
extern      int close(int fildes);

APP_DATA appData;

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
    appData.state = APP_STATE_INIT;
    pin_mode(&pins[U1TXLED], PIN_OUTPUT);
}

//uint32_t ts = 0;
//int v = 0;
void APP_Tasks ( void ) {    
/*
    if ((xTaskGetTickCount() - ts) > 500) {
        v = 1 - v;
        pin_set(&pins[U1TXLED], v);
        ts = xTaskGetTickCount();
    }
*/



    switch ( appData.state ) {
        case APP_STATE_INIT:        
            system_init_defaults();
            uart_create_ports();
            usb_create_ports();
            ethernet_init_defaults();
            appData.state = APP_STATE_LOAD_SETTINGS;
            break;
        case APP_STATE_LOAD_SETTINGS:
            load_settings(); 
            appData.state = APP_STATE_INIT_UARTS;
            break;
        case APP_STATE_INIT_UARTS:
            uart_boot();
            port_printf(CONSOLE, "\x0c\n\nMajenko Technologies Terminal Server V" VERSION "\r\n");
            port_printf(CONSOLE, "(c) 2026 Majenko Technologies, All Rights Reserved\r\n");
            port_printf(CONSOLE, "\r\n\n\n");
            appData.state = APP_STATE_INIT_USB;
            break;
        case APP_STATE_INIT_USB:
            USB_Initialize();
            appData.state = APP_STATE_INIT_ETHERNET;
            break;
        case APP_STATE_INIT_ETHERNET:
            ethernet_boot();
            telnet_in_initialize();
            telnet_out_initialize();
            port_printf(CONSOLE, "\nSystem initialized. Press <RETURN> to activate console.\r\n\n");
            appData.state = APP_STATE_SERVICE_TASKS;
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
                        case MODE_IDLE:
                            if (scan->access == ACCESS_LOCAL) {
                                if (scan->type == PORT_SERIAL) {
                                    if (port_available(scan)) {
                                        int c = port_read_byte(scan);
                                        if (c == 13) {
                                            scan->mode = MODE_GREET;
                                        }
                                    }
                                }
                            }
                            break;
                        case MODE_PREGREET: 
                            if (scan->ticks == 0) {
                                scan->ticks = xTaskGetTickCount();
                            } else if (xTaskGetTickCount() - scan->ticks > 1000) {
                                scan->ticks = 0;
                                scan->mode = MODE_GREET;
                            }
                            break;
                        case MODE_GREET:
                            if (scan->access == ACCESS_LOCAL) {
                                greet(scan);
                            }
                            break;
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
                        case MODE_PASSWORD:
                            if (scan->access == ACCESS_LOCAL) {
                                if (port_available(scan)) {
                                    int c = port_read_byte(scan);
                                    command_process(scan, c, &input_password);
                                }
                            }
                            break;
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
                        case MODE_SESSION:
                            if (scan->active_session && (scan->active_session->type == SESSION_DIRECT)) {
                                if (port_available(scan) && (cb_free(&scan->active_session->target->write_buffer))) {
                                    int c = port_read_byte(scan);
                                    uint16_t tmp[11];
                                    int r = fancy_read(scan, c, tmp, 10);
                                    for (int i = 0; i < r; i++) {
                                        if (tmp[i] == scan->local_switch) {
                                            port_printf(scan, "+++ OUT OF CHEESE ERROR +++\r\n");
                                            scan->mode = MODE_LOCAL;
                                        } else if (tmp[i] == scan->forward_switch) {
                                            struct session *first = NULL;
                                            struct session *prev = NULL;
                                            struct session *curr = NULL;
                                            struct session *next = NULL;
                                            struct session *last = NULL;
                                            for (struct session *sess = sessions; sess; sess = sess->next) {
                                                if (sess->type == SESSION_DELETED) continue;
                                                if (sess->parent == scan) {
                                                    if (first == NULL) first = sess;
                                                    last = sess;
                                                    if (sess == scan->active_session) {
                                                        curr = sess;
                                                        continue;
                                                    }
                                                    if ((curr == NULL)) {
                                                        prev = sess;
                                                        continue;
                                                    }
                                                    if ((next == NULL) && (curr != NULL)) {
                                                        next = sess;
                                                        continue;
                                                    }
                                                }
                                            }
                                            if (next == NULL) next = first;
                                            scan->active_session = next;
                                        } else if (tmp[i] == scan->backward_switch) {
                                            struct session *first = NULL;
                                            struct session *prev = NULL;
                                            struct session *curr = NULL;
                                            struct session *next = NULL;
                                            struct session *last = NULL;
                                            for (struct session *sess = sessions; sess; sess = sess->next) {
                                                if (sess->type == SESSION_DELETED) continue;
                                                if (sess->parent == scan) {
                                                    if (first == NULL) first = sess;
                                                    last = sess;
                                                    if (sess == scan->active_session) {
                                                        curr = sess;
                                                        continue;
                                                    }
                                                    if ((curr == NULL)) {
                                                        prev = sess;
                                                        continue;
                                                    }
                                                    if ((next == NULL) && (curr != NULL)) {
                                                        next = sess;
                                                        continue;
                                                    }
                                                }
                                            }
                                            if (prev == NULL) prev = last;
                                            scan->active_session = prev;
                                            
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

