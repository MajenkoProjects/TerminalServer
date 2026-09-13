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

extern      ssize_t write(int fildes, const void *buf, size_t nbyte);
extern      int close(int fildes);

APP_DATA appData;

void input_username(struct port *port) {
    
    char *username = strtok(port->command, " \t");
    if (!username) {        
        return;
    }
    if (strlen(username) == 0) {
        return;        
    }
    memset(port->username, 0, 9);
    int l = strlen(port->command);
    if (l > 8) l = 8;
    memcpy(port->username, port->command, l);
    port->mode = MODE_LOCAL;
}

void APP_Initialize ( void ) {
    appData.state = APP_STATE_INIT;
}

void APP_Tasks ( void ) {    
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
            port_printf(CONSOLE, "\nSystem initialized. Press <RETURN> to activate console.\r\n\n");
            appData.state = APP_STATE_SERVICE_TASKS;
            break;
        case APP_STATE_SERVICE_TASKS:
            for (struct port *scan = ports; scan; scan = scan->next) {
                if (scan->type != PORT_NONE) {
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
                                    command_process(scan, c, &input_username);
                                }
                            }
                            break;
                        case MODE_LOCAL:
                            if (scan->access == ACCESS_LOCAL) {
                                if (port_available(scan)) {
                                    int c = port_read_byte(scan);
                                    command_process(scan, c, &command_execute);
                                }
                            }
                            break;
                        case MODE_SESSION:
                            if (scan->active_session) {
                                if (port_available(scan) && (cb_free(&scan->active_session->target->write_buffer))) {
                                    int c = port_read_byte(scan);
                                    port_write_byte(scan->active_session->target, c);
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

